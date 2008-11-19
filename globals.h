
/*
*	Compiler Globals
*/

/* This is the CFG for the Toy language. We refer to
   the number of each production as its "nodeTag"

1.  program    -> { stmt_list }
2.  stmt_list  -> stmt ; stmt_list
3.  stmt_list  -> lambda      --- i.e., an empty production
4.  stmt       -> decl
5.  stmt       -> assign
6.  stmt       -> read_stmt
7.  stmt       -> write_stmt
8.  decl       -> DECLARE Name
9.  assign     -> var = expr
10. read_stmt  -> READ var
11. write_stmt -> WRITE expr
12. expr       -> term
13. expr       -> term + term
14. expr       -> term - term
15. term       -> NUMBER
16. term       -> var
17. term       -> ( expr )
18. var        -> NAME

*/

typedef enum { STMTS,DECL,ASSIGNSTMT,READSTMT,WRITESTMT,PLUSEXP,MINUSEXP,LITEXP,ID,STMT,
               ARGS,ARGS_TAIL,F_CALL,IFTHENELSE_STMT,WHILE_STMT,
               PROCDECL, F_BODY, VARS, VARS_TAIL, DECLS, PROCS, PROGRAM } optags;

typedef struct tree {
        optags nodeTag;
        int nodeVal;
        char *nodeName;
        struct tree *child1,*child2;
        struct tree *next,*next1,*next2;
}tree;
