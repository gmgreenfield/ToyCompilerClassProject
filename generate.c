#include        <stdio.h>		/* for (f)printf(), std{out,int} */
#include        <string.h>		/* for strcmp, strdup & friends */
#include        <stdlib.h>		/* for exit */
#include        <stdio.h>
#include        "globals.h"
#include        "toy.tab.h"		/* token type definitions from .y file */
#include        "symbol.h"
#include        "generate.h"

static FILE *fp;

static int s_size = 1;			/* start naming the registers at M1 */
static char *reg_stack[STACK_MAX];

static int ss_size = 1;
static int label_stack[STACK_MAX];

char *reg_push(int id)
{
	char *tmp;
	tmp = (char *)malloc(sizeof (char) * 7);
	sprintf(tmp, "R%d", id);
	reg_stack[id] = strdup(tmp);
	return reg_stack[id];
}

char *reg_refrence(int id)
{
	return reg_stack[id];
}

int label_push(int id) {
     label_stack[id] = id;
     return label_stack[id];
}

int label_refrence(int *id) {
     return label_stack[*id--];
}

void trav_exp(tree * p)
{
	if (p->child1 != NULL) {
		trav_exp(p->child1);
	}

	if (p->child2 != NULL) {
		trav_exp(p->child2);
	}

	if (p->nodeTag == PLUSEXP || p->nodeTag == MINUSEXP) {
		if (p->child1->reg_used != NULL && p->child2->reg_used != NULL) {
			fprintf(fp, "(%s,%s,%s,%s)\n",
					p->nodeTag == PLUSEXP ? "ADDI" : "SUBI", reg_push(s_size++),
					p->child1->reg_used, p->child2->reg_used);
			p->reg_used = strdup(reg_refrence(s_size - 1));
		}
		else if (p->child2->reg_used != NULL) {
			fprintf(fp, "(%s,%s,%d,%s)\n",
					p->nodeTag == PLUSEXP ? "ADDI" : "SUBI", reg_push(s_size++),
					p->child1->nodeVal, p->child2->reg_used);
			p->reg_used = strdup(reg_refrence(s_size - 1));
		}
		else if (p->child1->reg_used != NULL) {
			fprintf(fp, "(%s,%s,%s,%d)\n",
					p->nodeTag == PLUSEXP ? "ADDI" : "SUBI", reg_push(s_size++),
					p->child1->reg_used, p->child2->nodeVal);
			p->reg_used = strdup(reg_refrence(s_size - 1));
		}
		else {
			fprintf(fp, "(%s,%s,", p->nodeTag == PLUSEXP ? "ADDI" : "SUBI", reg_push(s_size++));
			code_gen_trav(p->child1);
			fprintf(fp, ",");
			code_gen_trav(p->child2);
			fprintf(fp, ")\n");
			p->reg_used = strdup(reg_refrence(s_size - 1));
		}
	}
}

void code_gen(tree * p)
{
	fp = fopen(OUTFILE, "w");	/* open file for write only access */

	/* begin the actual process */
	code_gen_trav(p);

	/* the last line in the tuple code */
	/* fprintf(fp,"%s","(EXIT)\n"); */
	fclose(fp);

	printf("Tuple code has been written to %s.\nExited normally.\n", OUTFILE);
	return;
}

void code_gen_trav(tree * p)
{
	switch (p->nodeTag) {
	case PROGRAM:
		if (p->next != NULL) {
			code_gen_trav(p->next);
		}
		if (p->next1 != NULL) {
			code_gen_trav(p->next1);
		}
		break;

	case WHILE_STMT:
	        fprintf(fp, "# begin while stmt #\n");
		if(p->child1->nodeTag == LITEXP || p->child1->nodeTag == ID) {
		     fprintf(fp, "(ADDI,%s,", reg_push(s_size++));
		     code_gen_trav(p->child1);
		     fprintf(fp, ",0)\n");
		} else {
		     code_gen_trav(p->child1);
		}
		fprintf(fp, "(JNZ,%s,%d)\n", reg_refrence(s_size - 1), ss_size++);
		fprintf(fp, "(JUMP,%d)\n", ss_size++);
		fprintf(fp, "(LABEL,%d)\n", ss_size - 2);
		if (p->next != NULL) { code_gen_trav(p->next); } /* while loop body */
		fprintf(fp, "(LABEL,%d)\n", ss_size - 1);
		fprintf(fp, "# end while stmt #\n");
		break;

	case IFTHENELSE_STMT:
	        fprintf(fp, "# begin if-then-else stmt #\n");
		if(p->child1->nodeTag == LITEXP || p->child1->nodeTag == ID) {
		     fprintf(fp, "(ADDI,%s,", reg_push(s_size++));
		     code_gen_trav(p->child1);
		     fprintf(fp, ",0)\n");
		} else {
		     code_gen_trav(p->child1);
		}
		fprintf(fp, "(JNZ,%s,%d)\n", reg_refrence(s_size - 1), ss_size++);
		if (p->next != NULL) { code_gen_trav(p->next1); } /* then statements */
		fprintf(fp, "(JUMP,%d)\n", ss_size++);
		fprintf(fp, "(LABEL,%d)\n", ss_size-2); 
		if (p->next1 != NULL) { code_gen_trav(p->next); } /* else statements */
		fprintf(fp, "(LABEL,%d)\n", ss_size-1);
		fprintf(fp, "# end if-then-else stmt #\n");
		break;

	case PROCS:
		code_gen_trav(p->child1);
		if (p->next != NULL) {
			code_gen_trav(p->next);
		}
		break;

	case PROCDECL:
		code_gen_trav(p->child1);
		if (p->next != NULL) {
			code_gen_trav(p->next);
		}
		if (p->next1 != NULL) {
			code_gen_trav(p->next1);
		}
		if (p->next2 != NULL) {
			code_gen_trav(p->next2);
		}
		break;

	case VARS:
		code_gen_trav(p->child1);
		if (p->next != NULL) {
			code_gen_trav(p->next);
		}
		break;

	case VARS_TAIL:
		if (p->next != NULL) {
			code_gen_trav(p->next);
		}
		break;

	case DECLS:
		code_gen_trav(p->child1);
		if (p->next != NULL) {
			code_gen_trav(p->next);
		}
		break;

	case ARGS:
		code_gen_trav(p->child1);
		if (p->next != NULL) {
			code_gen_trav(p->next);
		}
		break;

	case ARGS_TAIL:
		if (p->next != NULL) {
			code_gen_trav(p->next);
		}
		break;

	case F_CALL:
		code_gen_trav(p->child1);
		if (p->next != NULL) {
			code_gen_trav(p->next);
		}
		break;

	case STMTS:
		code_gen_trav(p->child1);
		if (p->next != NULL) {
			code_gen_trav(p->next);
		}
		break;

	case DECL:
		/* printf("declare %s",p->nodeName); */
		break;

	case ASSIGNSTMT:
		if (p->child2->nodeTag == PLUSEXP || p->child2->nodeTag == MINUSEXP) {
			code_gen_trav(p->child2);
			fprintf(fp, "(ASSIGN,");
			code_gen_trav(p->child1);
			fprintf(fp, ",%s)\n", reg_refrence(s_size - 1));
		}
		else {
			fprintf(fp, "(ASSIGN,");
			code_gen_trav(p->child1);
			fprintf(fp, ",");
			code_gen_trav(p->child2);
			fprintf(fp, ")\n");
		}
		break;

	case READSTMT:
		fprintf(fp, "(READI,");
		code_gen_trav(p->child1);
		fprintf(fp, ")\n");
		break;

	case WRITESTMT:
		if (p->child1->nodeTag == PLUSEXP || p->child1->nodeTag == MINUSEXP) {
			code_gen_trav(p->child1);
			fprintf(fp, "(WRITEI,%s)\n", reg_refrence(s_size - 1));
		}
		else {
			fprintf(fp, "(WRITEI,");
			code_gen_trav(p->child1);
			fprintf(fp, ")\n");
		}
		break;

	case PLUSEXP:
	case MINUSEXP:
		trav_exp(p);
		break;

	case LITEXP:
		fprintf(fp, "%d", p->nodeVal);
		break;

	case ID:
		fprintf(fp, "M[%s+%d]", var_type(p->nodeName), var_offset(p->nodeName));
		break;
	}
	return;
}
