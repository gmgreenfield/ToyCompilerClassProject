
/*
*	Compiler Globals
*/

/* This is the CFG for the Toy+ language. We refer to
   the number of each production as its "nodeTag"

1.  program    -> decl_list proc_list
2.  decl_list  -> decl ; decl_list
3.  decl_list  -> lambda
4.  proc_list  -> procdecl proc_list
5.  proc_list  -> lambda
6.  procdecl   -> proc NAME (var_list) { decl_list stmt_list }
7.  var_list   -> var var_list_tail
8.  var_list_tail -> , var_list
9.  var_list_tail -> lambda
10. arg_list   -> expr arg_list_tail
11. arg_list_tail -> , arg_list
12. arg_list_tail -> lambda
13.  stmt_list  -> stmt ; stmt_list
14.  stmt_list  -> lambda      --- i.e., an empty production
15.  stmt       -> assign
16.  stmt       -> read_stmt
17.  stmt       -> write_stmt
18.  decl       -> DECLARE Name
19.  assign     -> var = expr
20. read_stmt  -> READ var
21. write_stmt -> WRITE expr
22. expr       -> term
23. expr       -> term + term
24. expr       -> term - term
25. term       -> NUMBER
26. term       -> var
27. term       -> ( expr )
28. var        -> NAME

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
        char *reg_used;
}tree;
