module Interp where

import IO
import Char

------------------ Semantic basics ---------------------

data (Monad m) => StateT s m a = ST (s -> m (a,s))
deST (ST x) = x

liftSt :: Monad m => m a -> StateT s m a
liftSt phi = ST $ \ s0 -> phi >>= \ v -> return (v,s0)

instance Monad m => Monad (StateT s m) where
   return v	 = ST (\ s0 -> return (v,s0))
   (ST x) >>= f	 = ST (\ s0 -> (x s0) >>= \ (y,s1) -> deST (f y) s1)

liftSt phi = ST (\ s0 -> phi >>= (return . \ v -> (v,s0)))

data Err a = OK a | Error String
deOK (OK v)	 = v
deOK (Error msg) = error msg
data (Monad m) => ErrT m a = ErrM (m (Err a))
deErrM (ErrM phi) = phi

instance Monad m => Monad (ErrT m) where
     return	= ErrM . return . OK
     (ErrM phi) >>= f  = ErrM (phi >>= \ res ->
				 case res of
				      OK v -> ((deErrM . f) v)
				      Error msg -> return (Error msg))

liftErr phi = ErrM (phi >>= (return . OK))

type Loc    = Int
type Sto    = (String -> Int, Loc -> Int)
type LineNo = Int

type M a = StateT Sto (StateT Int (ErrT IO)) a

err :: String -> M a
err msg = liftSt $ liftSt $ ErrM (return (Error msg))

getline :: M LineNo
getline = liftSt $ ST (\ l -> return (l,l))

inc_line :: M ()
inc_line = liftSt $ ST (\ l -> return ((),l+1))

getSto :: M Sto
getSto = ST (\ sto -> return (sto,sto))

u :: (Sto -> Sto) -> M ()
u delta = ST (\ sto -> return ((),delta sto))

myputStr :: String -> M()
myputStr s = liftSt $ liftSt $ liftErr $ putStr s

readNum :: M Int
readNum = liftSt $ liftSt $ liftErr $ readLn

regarg :: RegArg -> M Loc
regarg = undefined

tweek f x v = \ n -> if n==x then v else f n

setreg r v = \ (rf,mem) -> (tweek rf r v,mem)
setloc l v = \ (rf,mem) -> (rf,tweek mem l v)

getreg r = do (rf,mem) <- getSto
	      return (rf (show r))
getloc loc = do (rf,mem) <- getSto
		return (mem loc)

getregarg :: RegArg -> M Int
getregarg (RgArg r)	  = getreg r
getregarg (RegInd r)	  = getreg r >>= getloc
getregarg (RegIndOff r o) = do v <- getreg r
			       getloc (v+o)

arg (RA ra)	= getregarg ra
arg (Literal i) = return i

store ra v = case ra of
		  (RgArg r)	  -> u (setreg (show r) v)
		  (RegInd r)	  -> getreg r >>= \ loc -> u (setloc loc v)
		  (RegIndOff r o) -> getreg r >>= \ loc -> u (setloc (loc+o) v)

tuple :: Tuple -> M ()
tuple (Asn ra a) = do i <- arg a
		      case ra of
			   (RgArg r)	   -> u (setreg (show r) i)
			   (RegInd r)	   -> (do loc <- getreg r ;
						  u (setloc loc i))
			   (RegIndOff r o) -> (do loc <- getreg r ;
						   u (setloc (loc+o) i))
tuple (Add ra a1 a2) = do i1 <- arg a1
			  i2 <- arg a2
			  case ra of
			      (RgArg r)	      -> u (setreg (show r) (i1+i2))
			      (RegInd r)      -> (do loc <- getreg r ;
						     u (setloc loc (i1+i2)))
			      (RegIndOff r o) -> (do loc <- getreg r ;
						     u (setloc (loc+o) (i1+i2)))
tuple (Sub ra a1 a2) = do i1 <- arg a1
			  i2 <- arg a2
			  case ra of
			      (RgArg r)	      -> u (setreg (show r) (i1-i2))
			      (RegInd r)      -> (do loc <- getreg r ;
						     u (setloc loc (i1-i2)))
			      (RegIndOff r o) -> (do loc <- getreg r ;
						     u (setloc (loc+o) (i1-i2)))
tuple (Label i)	     = return ()
tuple (Read ra)	     = do myputStr "\t(stdin) input an integer:"
			  i <- readNum
			  store ra i
tuple (Write a)	     = do i <- arg a
			  myputStr $ "\t(stdout) write result: " ++ show i ++ "\n"

mng :: Int -> [Tuple] -> [Tuple] -> M ()
mng (n+1) (Jmp a : is) p     = do label <- arg a
			          mng n (findLabel label p) p
mng (n+1) (JmpNZ r a : is) p = do i <- getregarg r
                                  label <- arg a
                                  if i==0 
                                     then
                                       mng n is p
                                     else
			               mng n (findLabel label p) p
mng (n+1) (Exit : is) p	     = myputStr "Exiting"
mng (n+1) (i : is) p	     = tuple i >> mng n is p
mng _ [] _                   = myputStr "Exiting"
mng 0 is p = do breakpoint is
		myputStr "\nHow many steps [enter=1,to quit=0]? "
		i <- mytry readint
		case i of
		     (Left _)  -> mng 1 is p
		     (Right i) -> if i==0 then return () else mng i is p

breakpoint is = let next = case is of
				(c:cs) -> show c
				[]     -> "no next instr."
		in
		do fp <- getreg FP
		   sp <- getreg SP
		   r1 <- getreg (Reg 1)
		   r2 <- getreg (Reg 2)
		   r3 <- getreg (Reg 3)
		   myputStr $ "BREAK: " ++
			      "[FP="++(show fp)++","++
			      "SP="++(show sp)++","++
			      "R1="++(show r1)++","++
			      "R2="++(show r2)++","++
			      "R3="++(show r3)++","++
			      "next instr. = "++ next ++ "]\n"

mytry = liftSt . liftSt . liftErr . try

rw = (Read (RgArg SP)) : (Write (RA $ RgArg SP)) : rw

readint :: IO Int
readint = readLn

test = run (mng 10 rw rw)

run :: M a -> IO a
run phi = (deErrM (deST (deST phi sto0) lineNo0)) >>= (return . fst . fst . deOK)
    where sto0 = (\ r -> 0, \ l -> 0)
	  lineNo0 = 0

findLabel l [] = error $ "Jump to non-existent label " ++ (show l)
findLabel l (Label i : as) = if l==i then as else findLabel l as
findLabel l (_:as)	   = findLabel l as
-----------------------------------------------------------

instance Show Register where
    show (Reg i) = "R" ++ (show i)
    show SP	 = "SP"
    show BP	 = "BP"
    show FP	 = "FP"

instance Show RegArg where
    show (RgArg r)	 = show r
    show (RegInd r)	 = "M[" ++ show r ++ "]"
    show (RegIndOff r o) = "M[" ++ show r ++ "+" ++ show o ++ "]"

instance Show Tuple where
    show (Asn ra a)	= "(ASSIGN,"++show ra++","++show a++")"
    show (Add ra a1 a2) = "(ADDI,"++show ra++","++show a1++","++show a2++")"
    show (Sub ra a1 a2) = "(SUBI,"++show ra++","++show a1++","++show a2++")"
    show (Jmp a)	= "(JUMP,"++show a++")"
    show (JmpNZ r a)	= "(JNZ,"++show r++","++show a++")"
    show (Label i)	= "(LABEL,"++show i++")"
    show (Read ra)	= "(READI,"++show ra++")"
    show (Write a)	= "(WRITEI,"++show a++")"
    show Exit		= "EXIT"

instance Show Arg where
    show (RA ra)     = show ra
    show (Literal i) = show i

data Tuple    = Asn RegArg Arg
	      | Add RegArg Arg Arg
	      | Sub RegArg Arg Arg
	      | Jmp Arg
              | JmpNZ RegArg Arg
	      | Label Int
	      | Read RegArg
	      | Write Arg
	      | Exit
data Register = Reg Int | SP | FP | BP deriving Eq
data RegArg   = RgArg Register | RegInd Register | RegIndOff Register Int
data Arg      = RA RegArg | Literal Int
type TupleProgram = [Tuple]

start = do
	 putStr "Filename? [type \"q\" to quit] "
	 file <- getLine
	 if (file=="q") then return ()
			   else do h <- try (openFile file ReadMode)
				   case h of
					(Left _)       -> start
					(Right handle) -> doit handle

doit handle = getit handle >>= \ ir ->
	      hClose handle >>
	      go ir
		    where go ir = let program = parseTuples (lexer ir)
				  in
				      --- putStr (show program)
				      run (mng 0 program program)

getit h = hIsEOF h >>= \ eof ->
	     if eof then
		      return ""
		    else
		      hGetLine h >>= \ l ->
		      getit h >>= \ rest ->
		      return (l ++ rest)

data Token = LPAR | RPAR | COMMA | ASSIGN
	   | ADDI | SUBI | JUMP | JUMPNZ | LIT Int
	   | READI | WRITEI | EXIT | REG Int | LABEL
	   | FPtok | SPtok | BPtok
	   | LBRACK | RBRACK | PLUS | M
	   deriving (Show,Eq)

lexNum cs = LIT (read num) : lexer rest
	where (num,rest) = span isDigit cs

lexAlpha cs = let (as,rest) = span isAlpha cs in
		 case as of
		      "ADDI"   -> ADDI : lexer rest
		      "SUBI"   -> SUBI : lexer rest
		      "READI"  -> READI : lexer rest
		      "WRITEI" -> WRITEI : lexer rest
		      "ASSIGN" -> ASSIGN : lexer rest
		      "LABEL"  -> LABEL : lexer rest
		      "M"      -> M : lexer rest
		      "FP"     -> FPtok : lexer rest
		      "SP"     -> SPtok : lexer rest
		      "BP"     -> BPtok : lexer rest
		      "JUMP"   -> JUMP : lexer rest
		      "JNZ"    -> JUMPNZ : lexer rest
		      "EXIT"   -> EXIT : lexer rest
		      "R"      -> let (LIT i :rs) = lexNum rest in
				    REG i : rs

lexer :: String -> [Token]
lexer [] = []
lexer ('(':cs) = LPAR : lexer cs
lexer (')':cs) = RPAR : lexer cs
lexer ('[':cs) = LBRACK : lexer cs
lexer (']':cs) = RBRACK : lexer cs
lexer ('+':cs) = PLUS : lexer cs
lexer (',':cs) = COMMA : lexer cs
lexer ('#':cs) = consumeLine cs
lexer (c:cs)
	| isSpace c = lexer cs
	| isAlpha c = lexAlpha (c:cs)
	| isDigit c = lexNum (c:cs)

consumeLine []	      = []
consumeLine ('#':cs) = lexer cs
consumeLine (c:cs)    = consumeLine cs

t2 = "(READI,SP) \n (WRITEI,SP)\n (READI,SP)\n (WRITEI,SP)\n (READI,SP)\n (WRITEI,SP)"

t0 = "(ASSIGN,M[SP+3],R11)"
t1 = "(ASSIGN,R11,R11)"

parseTuples []	      = []
parseTuples (LPAR:ts) = t : (parseTuples rest)
	where (t,rest) = parseTuple ts

-- RegArg   -> Register | M[Register] | M[Register + Num]
parseRegArg (REG i:ts) = (RgArg (Reg i), ts)
parseRegArg (SPtok:ts) = (RgArg SP, ts)
parseRegArg (FPtok:ts) = (RgArg FP , ts)
parseRegArg (BPtok:ts) = (RgArg BP, ts)
parseRegArg (M:LBRACK:r:RBRACK:ts) = (ra,ts)
      where ra = RegInd (register r)
parseRegArg (M:LBRACK:r:PLUS:(LIT o):RBRACK:ts) = (ra,ts)
      where ra = RegIndOff (register r) o
parseRegArg ts = error $ "parsing error around: " ++ (show $ twenty ts)

parseArg (REG i:ts)		      = (RA $ RgArg (Reg i), ts)
parseArg (SPtok:ts)		      = (RA $ RgArg SP, ts)
parseArg (FPtok:ts)		      = (RA $ RgArg FP , ts)
parseArg (BPtok:ts)		      = (RA $ RgArg BP, ts)
parseArg (M:LBRACK:r:RBRACK:ts)	      = (RA ra,ts)
      where ra = RegInd (register r)
parseArg (M:LBRACK:r:PLUS:(LIT o):RBRACK:ts) = (RA ra,ts)
      where ra = RegIndOff (register r) o
parseArg (LIT i:ts)		      = (Literal i,ts)
parseArg ts = error $ "parsing error around: " ++ (show $ twenty ts)

register r = case r of
		  (REG i) -> (Reg i)
		  SPtok	  -> SP
		  FPtok	  -> FP
		  BPtok	  -> BP
		  _	  -> error $ "not a register: " ++ (show r)

parseTuple (ASSIGN:ts) = let ts0       = comma ts
			     (ra,ts1)  = parseRegArg ts0
			     ts2       = comma ts1
			     (arg,ts3) = parseArg ts2
			 in
			     (Asn ra arg, rpar ts3)

parseTuple (ADDI:ts)   = let ts0       = comma ts
			     (ra,ts1)  = parseRegArg ts0
			     ts2       = comma ts1
			     (arg1,ts3) = parseArg ts2
			     ts4       = comma ts3
			     (arg2,ts5) = parseArg ts4
			 in
			     (Add ra arg1 arg2, rpar ts5)
parseTuple (SUBI:ts)   = let ts0       = comma ts
			     (ra,ts1)  = parseRegArg ts0
			     ts2       = comma ts1
			     (arg1,ts3) = parseArg ts2
			     ts4       = comma ts3
			     (arg2,ts5) = parseArg ts4
			 in
			     (Sub ra arg1 arg2, rpar ts5)
parseTuple (JUMP:ts)   = let ts0       = comma ts
			     (arg,ts1)	= parseArg ts0
			 in
			     (Jmp arg, rpar ts1)
parseTuple (JUMPNZ:ts) = let ts0       = comma ts
			     (reg,ts1) = parseRegArg ts0
                             ts2       = comma ts1
			     (arg,ts3) = parseArg ts2
			 in
			     (JmpNZ reg arg, rpar ts3)
parseTuple (LABEL:ts)  = let ts0       = comma ts
			     (Literal i,ts1) = parseArg ts0
			 in
			     (Label i, rpar ts1)
parseTuple (READI:ts)  = let ts0       = comma ts
			     (arg,ts1)	= parseRegArg ts0
			 in
			     (Read arg, rpar ts1)
parseTuple (WRITEI:ts) = let ts0       = comma ts
			     (arg,ts1) = parseArg ts0
			 in
			     (Write arg, rpar ts1)
parseTuple (EXIT:ts)   = (Exit, rpar ts)

constant c (t:ts) = if c==t then ts
			    else error $ "parsing error around: " ++ (show $ twenty ts)

comma = constant COMMA
rpar  = constant RPAR

first n [] = []
first 0 xs = []
first n (x:xs) = x : (first (n-1) xs)

twenty = first 20


{-
Tuple	 -> Asn | Add | Sub | Jmp | Label | Read | Write | Exit
Asn	 -> (ASSIGN,RegArg,Arg)
Add	 -> (ADDI,RegArg,Arg,Arg)
Sub	 -> (SUBI,RegArg,Arg,Arg)
Jmp	 -> (JUMP,Arg)
Label	 -> (LABEL,Num) /* N.b., labels are Nums */
Read	 -> (READI,RegArg)
Write	 -> (WRITEI,Arg)
Exit	 -> (EXIT)
Arg	 -> RegArg | Num
Register -> R Num | SP | FP | BP
RegArg	 -> Register | M[Register] | M[Register + Num]
Num	 -> Digit+
-}



