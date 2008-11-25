/*
*	Symbol table management for toy language compiler.
*	This is a trivial example: the only information kept
*       is the name, the token type: READ, WRITE or NAME
*/
#include        <stdio.h>			/* for (f)printf(), std{out,int} */
#include        <string.h>			/* for strcmp, strdup & friends */
#include        <stdlib.h>			/* for exit */
#include        "globals.h"
#include        "toy.tab.h"			/* token type definitions from .y file */
#include        "symbol.h"

typedef struct
{
    char *name;
    int declared;		        /* 1 if already declared, 0 else */
    int type;				/* tells if its a routine or a variable */
    char *local_to;			/* who the variable belongs to if in a proc, or null if its global */
    int num_args;                       /* will be 0 in the case of a variable */
    int fp_offset;                      /* variable is found at offset i from the frame pointer (fp and base offsets are mutually exclusive) */
    int base_addr_offset;               /* variable is found at offset i from the constant base address*/
} ENTRY;

#define		MAX_SYMBOLS	100
#define         ROUTINE         0
#define         VARIABLE        1
#define         NOTFOUND        -1
#define         FP_BASE         3

static ENTRY symbol_table[MAX_SYMBOLS];	/* static: we treat the table as an ADT */
static int n_symbol_table = 0;	        /* number of entries in symbol table */

void print_symboltable(void)
{
    int i;
    printf("symboltable...\n");
    for (i = 0; i < n_symbol_table; i++) {
	printf("name:%15s type:%9s local_to:%15s declared:%d num_args:%2d fp_offset:%2d base_addr_offset:%2d\n", symbol_table[i].name,
	       symbol_table[i].type == ROUTINE ? "routine" : "variable", symbol_table[i].local_to, symbol_table[i].declared,
	       symbol_table[i].num_args, symbol_table[i].fp_offset, symbol_table[i].base_addr_offset);
    }
    return;
}

/* traverse AST and write initial contents to symbol table in post-order traversal */
void symbolscan(tree *node, int entry_type, char *local_to, int num_args) {
    if(node->child1 != NULL) { symbolscan(node->child1, entry_type, local_to, num_args); }
    if(node->next != NULL) { symbolscan(node->next, entry_type, local_to, num_args); }
    if(node->nodeName != NULL) { symbol_insert(node->nodeName, entry_type, local_to, num_args); }
    return;
}

int count_func_args(tree *p) {
    int cnt = 0;
    if(p == NULL) return cnt;
    while( p->next != NULL ) {
        cnt++;
        p = p->next->next;
    }
    return (cnt + 1);
}

void mk_symboltable(tree *p) {
    int i, j, base_cnt = 0;
    
    start_symboltable(p);
    /* global variables */
    for(i = 0; i < n_symbol_table; i++) {
	if(symbol_table[i].type == VARIABLE && strcmp(symbol_table[i].local_to, "*global*") == 0)
	    symbol_table[i].base_addr_offset = base_cnt++;
    }
    
    base_cnt = 0;
    /* local variables */
    for(i = 0; i < n_symbol_table; i++)
	if(symbol_table[i].type == ROUTINE) {
	    for(j = i + 1; symbol_table[j].type != ROUTINE; j++) {
		symbol_table[j].fp_offset = FP_BASE + base_cnt++;
	    }
	    base_cnt = 0;
	}
    
    return;
}

void start_symboltable(tree *p)
{
	switch (p->nodeTag) {
	case PROGRAM:
	    if (p->next != NULL) { symbolscan(p->next, VARIABLE, "*global*", 0); } /* decl list */
	    if (p->next1 != NULL) { start_symboltable(p->next1); } /* proc list */
	    break;

	case PROCDECL:
	    if (p->child1 != NULL) { symbolscan(p->child1, ROUTINE, "*n/a*", count_func_args(p->next)); } /* procedure name */
	    if (p->next != NULL) { symbolscan(p->next, VARIABLE, p->child1->nodeName, 0); } /* arg variable list */
	    if (p->next1 != NULL) { symbolscan(p->next1, VARIABLE, p->child1->nodeName, 0); } /* proc body decl list */
	    break;

	default:
	    if (p->next1 != NULL) { start_symboltable(p->next1); }
	    if (p->child2 != NULL) { start_symboltable(p->child2); }
	    if (p->next != NULL) { start_symboltable(p->next); }
	    if (p->child1 != NULL) { start_symboltable(p->child1); }
	    break;
	}
}

void static_checks(tree *p) {
    int i, j, err = 0;
    
    /* if theres a procedure called main */
    if(symbol_find("main", ROUTINE) == -1) {
	fprintf(stderr, "static_checks(): Program is missing a required routine called 'main'.\n");
	err++;
    }
    
    /* if any routines have duplicate names */
    for(i=0; i<n_symbol_table; i++)
	if(symbol_table[i].type == ROUTINE)
	    for(j=0; j<n_symbol_table; j++)
		if(symbol_table[j].type == ROUTINE && i != j && (strcmp(symbol_table[i].name, symbol_table[j].name) == 0)) {
		    fprintf(stderr, "static_checks(): Program has multiple routines with the same name of '%s'.\n", symbol_table[j].name);
		    err++;
		}
    
    /* if any variables within the same scope have duplicate names */
    for(i=0; i<n_symbol_table; i++)
	if(symbol_table[i].type == VARIABLE && strcmp(symbol_table[i].local_to, "*n/a*") != 0)
	    for(j=0; j<n_symbol_table; j++)
		if(symbol_table[j].type == VARIABLE && strcmp(symbol_table[j].local_to, "*n/a*") != 0 && i != j)
		    if(strcmp(symbol_table[i].name, symbol_table[j].name) == 0 && strcmp(symbol_table[i].local_to, symbol_table[j].local_to) == 0) {
			printf("static_checks(): variables with the duplicate name of '%s' are both within the scope of '%s'.\n",
			       symbol_table[i].name, symbol_table[j].local_to);
			err++;
		    }

    /* check if prodecure call has correct number of args */
    err += proc_call_check(p);
    /* check if the variables that are being used are defined */ 
    err += var_define_check(p);

    printf("%d Errors from static checks have been found.\n", err);

    if(err > 0) {
	printf("Exited abnormally.\n");
	exit(1);
    }
    
    return;
}

int proc_call_check(tree *p) {
    int i;
    static int err = 0;
    
    switch (p->nodeTag) {
	case F_CALL: /* check if procedure calls have correct number of args*/
	    for (i=0; i<n_symbol_table; i++)
		if (strcmp(symbol_table[i].name, p->child1->nodeName) == 0 && symbol_table[i].type == ROUTINE)
		    if (count_func_args(p->next) != symbol_table[i].num_args) {
			printf("static_check_trav(): The procedure '%s' was not called with the correct number of arguments.\n", p->child1->nodeName);
			err++;
		    }
	    break;
	      
	default:
	    if (p->next1 != NULL) { proc_call_check(p->next1); }
	    if (p->child2 != NULL) { proc_call_check(p->child2); }
	    if (p->next != NULL) { proc_call_check(p->next); }
	    if (p->child1 != NULL) { proc_call_check(p->child1); }
	    if (p->next2 != NULL) { proc_call_check(p->next2); }
	    break;
    }
    
    return err;
}

int var_define_check(tree *p) {
     static int err = 0;
     
     switch(p->nodeTag) {
         case PROGRAM:
	     if (p->next != NULL) { var_define_check(p->next); }
	     if (p->next1 != NULL) { var_define_check(p->next1); }
         break;

	 case VARS:
	 case VARS_TAIL:
	 case DECLS:
	 case DECL:
	 case LITEXP:       
	      break;

         case WHILE_STMT:
	     var_define_check(p->child1);
	     if (p->next != NULL) { var_define_check(p->next); }
         break;

         case IFTHENELSE_STMT:
	     var_define_check(p->child1);
	     if (p->next != NULL) { var_define_check(p->next); }
	     if (p->next1 != NULL) { var_define_check(p->next1); }
         break;

         case PROCS:
	      var_define_check(p->child1);
	      if (p->next != NULL) { var_define_check(p->next); }
	      break;

         case PROCDECL:
	      if (p->next2 != NULL) { var_define_check(p->next2); }
	      break;
	      
         case ARGS:
	      var_define_check(p->child1);
	      if (p->next != NULL) { var_define_check(p->next); }
              break;

         case ARGS_TAIL:
	      if (p->next != NULL) { var_define_check(p->next); }
              break;

         case F_CALL:
              if (p->next != NULL) { var_define_check(p->next); }
              break;

         case STMTS:
	      var_define_check(p->child1);
	      if (p->next != NULL) { var_define_check(p->next); }
              break;

         case ASSIGNSTMT:
              var_define_check(p->child1);         
              var_define_check(p->child2);
              break;

         case READSTMT:
              var_define_check(p->child1);                       
              break;

         case WRITESTMT:
              var_define_check(p->child1);                       
              break;

         case PLUSEXP:                  
              var_define_check(p->child1);       
              var_define_check(p->child2);                 
              break;

         case MINUSEXP:                   
              var_define_check(p->child1);         
              var_define_check(p->child2);                 
              break;

         case ID:
	     if(symbol_find(p->nodeName, VARIABLE) == NOTFOUND) { /* scan identifiers and check if they exist in symbol table */
		  printf("var_define_check(): The variable '%s' was not declared.\n", p->nodeName);
		  err++;
	      }
              break;
     }
     
     return err;
}

int symbol_find(char *name, int type)
/* Find index of symbol table entry, -1 if not found */
{
	int i;

	for (i = 0; (i < n_symbol_table); ++i)
		if (strcmp(symbol_table[i].name, name) == 0 && symbol_table[i].type == type)
		    return i;
	return NOTFOUND;
}

int symbol_type(int i) {
/* Return the type of symbol that has index i */
    return symbol_table[i].type;
}

int symbol_insert(char *name, int entry_type, char *local_to, int num_args)
/* Insert new symbol with a given type into symbol table, returns index new value */
/* Routine is 0, variable is 1 for the entry_type */
{
	if (n_symbol_table >= MAX_SYMBOLS) {
		fprintf(stderr, "Symbol table overflow (%d) entries\nExited abnormally.\n", n_symbol_table);
		exit(1);
	}
	symbol_table[n_symbol_table].name = strdup(name);
	symbol_table[n_symbol_table].declared = 1;
	symbol_table[n_symbol_table].type = entry_type;
	symbol_table[n_symbol_table].local_to = strdup(local_to);
	symbol_table[n_symbol_table].num_args = num_args;
	symbol_table[n_symbol_table].fp_offset = 0;
	symbol_table[n_symbol_table].base_addr_offset = 0;
	return n_symbol_table++;
}

void symbol_declare(int i)
/* Mark a symbol in the table as declared */
{
	symbol_table[i].declared = 1;
}

int symbol_declared(int i)
/* Return declared property of symbol */
{
	return symbol_table[i].declared;
}

char *symbol_name(int i)
/* Return declared property of symbol */
{
	return symbol_table[i].name;
}
