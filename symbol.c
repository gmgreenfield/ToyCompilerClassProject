/*
*	Symbol table management for toy language compiler.
*	This is a trivial example: the only information kept
*       is the name, the token type: READ, WRITE or NAME
*/
#include	<stdio.h>			/* for (f)printf(), std{out,int} */
#include	<string.h>			/* for strcmp, strdup & friends */
#include	<stdlib.h>			/* for exit */
#include        "globals.h"
#include	"toy.tab.h"			/* token type definitions from .y file */
#include	"symbol.h"

typedef struct
{
    char *name;
    int declared;		        /* 1 if already declared, 0 else */
    int type;				/* tells if its a routine or a variable */
    char *local_to;			/* who the variable belongs to if in a proc, or null if its global */
} ENTRY;

#define		MAX_SYMBOLS	100
#define         ROUTINE         0
#define         VARIABLE        1     
static ENTRY symbol_table[MAX_SYMBOLS];	/* static: we treat the table as an ADT */
static int n_symbol_table = 0;	/* number of entries in symbol table */

void print_symboltable(void)
{
    int i;
    for (i = 0; i < n_symbol_table; i++) {
	printf("name:%s type:%d\n", symbol_table[i].name, symbol_table[i].type);
    }
}

void symbolscan(tree *node) {
    if(node->child1 != NULL) { symbolscan(node->child1); }
    if(node->next != NULL) { symbolscan(node->next); }
    if(node->nodeName != NULL) { printf("scan:%s\n", node->nodeName); }
    return;
}

void mk_symboltable(tree *p)
{
	switch (p->nodeTag) {
	case PROGRAM:
		if (p->next != NULL) { symbolscan(p->next); }
		if (p->next1 != NULL) { mk_symboltable(p->next1); }
		break;

	case WHILE_STMT:
		if (p->next != NULL) { mk_symboltable(p->next); }
		break;

	case IFTHENELSE_STMT:
		if (p->next != NULL) { mk_symboltable(p->next); }
		if (p->next1 != NULL) { mk_symboltable(p->next1); }
		break;

	case PROCS:
		mk_symboltable(p->child1);
		if (p->next != NULL) { mk_symboltable(p->next); }
		break;

	case PROCDECL: /* scan for symbols to add to table */
		if (p->next != NULL) { symbolscan(p->next); }
		if (p->next1 != NULL) { symbolscan(p->next1); }
		break;

	case VARS: 
		mk_symboltable(p->child1);
		if (p->next != NULL) { mk_symboltable(p->next); }
		break;

	case VARS_TAIL:
		if (p->next != NULL) { mk_symboltable(p->next); }
		break;

	case DECLS: 
		mk_symboltable(p->child1);
		if (p->next != NULL) { mk_symboltable(p->next); }
		break;

	case ARGS:
		mk_symboltable(p->child1);
		if (p->next != NULL) { mk_symboltable(p->next); }
		break;

	case ARGS_TAIL:
		if (p->next != NULL) { mk_symboltable(p->next); }
		break;

	case F_CALL:
		mk_symboltable(p->child1);
		if (p->next != NULL) { mk_symboltable(p->next); }
		break;

	case STMTS:
		mk_symboltable(p->child1);
		if (p->next != NULL) { mk_symboltable(p->next); }
		break;

	case ASSIGNSTMT:
		mk_symboltable(p->child1);
		mk_symboltable(p->child2);
		break;

	case READSTMT:
		mk_symboltable(p->child1);
		break;

	case WRITESTMT:
		mk_symboltable(p->child1);
		break;
	} /* end select case */
} /* end build symbol table */

int symbol_find(char *name)
/*	Find index of symbol table entry, -1 if not found */
{
	int i;

	for (i = 0; (i < n_symbol_table); ++i)
		if (strcmp(symbol_table[i].name, name) == 0)
			return i;
	return -1;
}

int symbol_insert(char *name, int entry_type)
/*	Insert new symbol with a given type into symbol table, returns index new value */
/*      Routine is 0, variable is 1 for the entry_type */
{
	if (n_symbol_table >= MAX_SYMBOLS) {
		fprintf(stderr, "Symbol table overflow (%d) entries\n", n_symbol_table);
		exit(1);
	}
	symbol_table[n_symbol_table].name = strdup(name);
	symbol_table[n_symbol_table].declared = 0;
	symbol_table[n_symbol_table].type = entry_type;
	return n_symbol_table++;
}

void symbol_declare(int i)
/*	Mark a symbol in the table as declared */
{
	symbol_table[i].declared = 1;
}

int symbol_declared(int i)
/*	Return declared property of symbol */
{
	return symbol_table[i].declared;
}

char *symbol_name(int i)
/*	Return declared property of symbol */
{
	return symbol_table[i].name;
}
