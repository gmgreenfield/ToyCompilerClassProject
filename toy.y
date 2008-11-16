%{
/*	
*	Parser specification for Toy
*/
#include	<stdio.h>	/* for (f)printf() */
#include	<stdlib.h>	/* for exit() */
#include	<string.h>	/* for strlen(), strcpy() */
#include	"symbol.h"
#include	"globals.h"

char		*base;		
int		lineno	= 1;	/* number of current source line */
extern int	yylex();	/* lexical analyzer generated from lex.l */
extern char	*yytext;	/* last token, defined in lex.l  */

tree *nodealloc(int production) {
     tree *newnode     = (tree*) malloc(sizeof(tree));
     newnode->nodeTag  = production;
     newnode->nodeName = NULL;
     newnode->child1   = NULL;
     newnode->child2   = NULL;
     newnode->next     = NULL;
     newnode->next1    = NULL;
     newnode->next2    = NULL;
     return newnode;
};

void
yyerror(char *s)
{
fprintf(stderr,"Syntax error on line number %d: %s\n",lineno,s);
fprintf(stderr,"Last token was \"%s\"\n",yytext);
exit(1);
}

%}
   
%token	NAME
%token	NUMBER
%token	LPAREN
%token	RPAREN
%token	LBRACE
%token	RBRACE
%token	ASSIGN
%token	SEMICOLON
%token	PLUS
%token	MINUS
%token  COMMA
%token	DECLARE
%token	WRITE
%token	READ
%token  CALL
%token  PROC
%token  ELSE
%token  IF
%token  WHILE
%union {
       int integer;
       char* str;
       tree* node;
} ;

%type	<integer> NUMBER   
%type	<str> NAME
%type	<node> var stmts program stmt decl assign read write expr term args args_tail f_call 
%type   <node> procdecl vars vars_tail procs decls ifthenelse_stmt while_stmt

%%

program	: decls procs
	        {
		  tree *p = nodealloc(PROGRAM);
                  p->next = $1;
                  p->next1 = $2;
                  $$ = p;
		  return (int)$$;
                }
	;

ifthenelse_stmt : IF LPAREN expr RPAREN LBRACE stmts RBRACE ELSE LBRACE stmts RBRACE
{
	tree *p = nodealloc(IFTHENELSE_STMT);
	p->child1 = $3;
	p->next = $6;
	p->next1 = $10;
	$$ = p;
};

while_stmt : WHILE LPAREN expr RPAREN LBRACE stmts RBRACE
{
    tree *p = nodealloc(WHILE_STMT);
    p->child1 = $3;
    p->next = $6;
    $$ = p;
};

procs : procdecl procs
{
  tree *p = nodealloc(PROCS);
  p->child1 = $1;
  p->next = $2;
  $$ = p;
}
| /* empty */
{
  $$ = NULL;
};

decls : decl SEMICOLON decls
{
  tree *p = nodealloc(DECLS);
  p->child1 = $1;
  p->next = $3;
  $$ = p;
}
| /* empty */
{
  $$ = NULL;
};

procdecl : PROC var LPAREN vars RPAREN LBRACE decls stmts RBRACE
{
  tree *p = nodealloc(PROCDECL);
  p->child1 = $2;
  p->next = $4;
  p->next1 = $7;
  p->next2 = $8;
  $$ = p;
};

vars : var vars_tail
{
  tree *p = nodealloc(VARS);
  p->child1 = $1;
  p->next = $2;
  $$ = p;
};

vars_tail : COMMA vars
{
  tree *p = nodealloc(VARS_TAIL);
  p->next = $2;
  $$ = p;
}
| /* empty */
{
  $$ = NULL;
};

args    : expr args_tail
		{
                tree *p = nodealloc(ARGS);
                	p->child1 = $1;
                	p->next = $2;
			$$ = p;
		}
	;

args_tail : COMMA args
		{
                tree *p = nodealloc(ARGS_TAIL);
                	p->next = $2;
			$$ = p;
		}
	  | /* empty */
		{
                $$ = NULL;
		}
	  ;

stmts	: stmt SEMICOLON stmts
                {
                tree *p = nodealloc(STMTS);
		       p->child1 = $1;
		       p->next   = $3;
		       $$ = p;
                }
	| /* empty */
	        {
	        $$ = NULL;
                }
	;
	
stmt	: ifthenelse_stmt
        | while_stmt
        | f_call
	| assign
	| read
	| write
	;

f_call  : CALL var LPAREN args RPAREN
		{
		tree *p = nodealloc(F_CALL);
		p->child1 = $2;
		p->next = $4;
		$$ = p;
		}
		;

decl	: DECLARE NAME
		{
		tree *p = nodealloc(DECL);
		p->nodeName = (char*) malloc(strlen(yytext) * sizeof(char));
		strcpy(p->nodeName,yytext); 
		$$ = p;
		}
		;

assign	: var ASSIGN expr
		{
		tree *p = nodealloc(ASSIGNSTMT);
		p->child1 = $1;
		p->child2 = $3;
		$$ = p;
		}
		;

read	: READ var
		{
		tree *p = nodealloc(READSTMT);
		p->child1 = $2;
		$$ = p;
		}
	;

write	: WRITE expr
		{
		tree *p = nodealloc(WRITESTMT);
		p->child1 = $2;
		$$ = p;
		}
	;

expr	: term
        | term PLUS term
		{
		tree *p = nodealloc(PLUSEXP);
		p->child1 = $1;
		p->child2 = $3;
		$$ = p;
		}
	| term MINUS term
		{
		tree *p = nodealloc(MINUSEXP);
		p->child1 = $1;
		p->child2 = $3;
		$$ = p;
		}
	;

term	: NUMBER
		{
		tree *p = nodealloc(LITEXP);
		p->nodeVal = atoi(yytext);
		$$ = p;
		}
	| var
	| LPAREN expr RPAREN
	        {
	        $$ = $2;
	        }
	;
	
var	: NAME
		{
		tree *p = (tree*) nodealloc(ID);
		p->nodeName = (char*) malloc(strlen(yytext) * sizeof(char));
		strcpy(p->nodeName,yytext); 
		$$	= p;
		}
	;
%%

void prettystmts (tree *p) {
     switch(p->nodeTag) {
         case PROGRAM:
           if (p->next != NULL) { prettystmts(p->next); }
           if (p->next1 != NULL) { prettystmts(p->next1); }
         break;

         case WHILE_STMT:
           printf("while (");
           prettystmts(p->child1);
           printf(") {\n");
           if (p->next != NULL) { prettystmts(p->next); }
           printf("}");
         break;

         case IFTHENELSE_STMT:
           printf("if (");
           prettystmts(p->child1);
           printf(") {\n");
           if (p->next != NULL) { prettystmts(p->next); }
           printf("} else {\n");
           if (p->next1 != NULL) { prettystmts(p->next1); }
           printf("}");
         break;

         case PROCS:
	   prettystmts(p->child1);
	   //printf(" ");
	   if (p->next != NULL) { prettystmts(p->next); }
	 break;

         case PROCDECL:
	   printf("proc ");
	   prettystmts(p->child1);
	   printf(" (");
	   if (p->next != NULL) { prettystmts(p->next); }
	   printf(") ");
	   printf("{\n");
	   if (p->next1 != NULL) { prettystmts(p->next1); }
	   if (p->next2 != NULL) { prettystmts(p->next2); }
	   printf("}\n");
	 break;

         case VARS:
	      prettystmts(p->child1);
	      if (p->next != NULL) { prettystmts(p->next); }
	      break;

         case VARS_TAIL:
	      printf(", ");
	      if (p->next != NULL) { prettystmts(p->next); }
	      break;

         case DECLS:
	      prettystmts(p->child1);
	      printf(";\n");
	      if (p->next != NULL) { prettystmts(p->next); }
	      break;
	      
         case ARGS:
              prettystmts(p->child1);
              if (p->next != NULL) { prettystmts(p->next); }
              break;

         case ARGS_TAIL:
              printf(", ");
              if (p->next != NULL) { prettystmts(p->next); }
              break;

         case F_CALL:
              printf("call ");
              prettystmts(p->child1);
              printf(" (");
              if (p->next != NULL) { prettystmts(p->next); }
              printf(")");
              break;

         case STMTS:
	      prettystmts(p->child1);
	      printf(";\n");
	      if (p->next != NULL) { prettystmts(p->next); }
              break;

         case DECL:
              printf("declare %s",p->nodeName);
              break;

         case ASSIGNSTMT:
              prettystmts(p->child1);         
              printf(" = ");
              prettystmts(p->child2);
              break;

         case READSTMT:
              printf("read ");
              prettystmts(p->child1);                       
              break;

         case WRITESTMT:
              printf("write ");
              prettystmts(p->child1);                       
              break;

         case PLUSEXP:
              printf("(");                  
              prettystmts(p->child1);
              printf(" + ");         
              prettystmts(p->child2);
              printf(")");                  
              break;

         case MINUSEXP: 
              printf("(");                  
              prettystmts(p->child1);
              printf(" - ");         
              prettystmts(p->child2);
              printf(")");                  
              break;

         case LITEXP:
              printf("%d",p->nodeVal);         
              break;

         case ID:
              printf("%s",p->nodeName);
              break;
     }
}

void pretty (tree *ast) {
     printf("prettyprint...\n");
     if (ast != NULL) prettystmts(ast);
}

int
main(int argc,char *argv[])
{
     tree *root;
     if (argc!=2)
	{
	fprintf(stderr,"Usage: %s base_file_name\n",argv[0]);
	exit(1);
	}
     base = argv[1];
     root = (tree*) yyparse();
     pretty(root);
     mk_symboltable(root);
     print_symboltable();
     return 1;
}

