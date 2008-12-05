#include        <stdio.h>		/* for (f)printf(), std{out,int} */
#include        <string.h>		/* for strcmp, strdup & friends */
#include        <stdlib.h>		/* for exit */
#include        <stdio.h>
#include        "globals.h"
#include        "toy.tab.h"		/* token type definitions from .y file */
#include        "symbol.h"
#include        "generate.h"

static FILE *fp;

/* register assignment */
static int s_size = 1;			/* start naming the registers at R1 */
static char *reg_stack[STACK_MAX];

/* label assignment in label.c */

char *reg_push(int id) {
	char *tmp;
	tmp = (char *)malloc(sizeof (char) * 7);
	sprintf(tmp, "R%d", id);
	reg_stack[id] = strdup(tmp);
	return reg_stack[id];
}

char *reg_refrence(int id) {
	return reg_stack[id];
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
		        fprintf(fp, "(%s,%s,", p->nodeTag == PLUSEXP ? "ADDI" : "SUBI", reg_push(s_size++));
		        code_gen_trav(p->child1);
		        fprintf(fp, ",%s)\n", p->child2->reg_used);
		        p->reg_used = strdup(reg_refrence(s_size - 1));
		}
		else if (p->child1->reg_used != NULL) {
			fprintf(fp, "(%s,%s,%s,",
					p->nodeTag == PLUSEXP ? "ADDI" : "SUBI", reg_push(s_size++),
					p->child1->reg_used);
			code_gen_trav(p->child2);
			fprintf(fp, ")\n");
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
	} /* if */
}

int get_funct_label(char *name) {
     int i;
     i = symbol_find(name, ROUTINE);
     if(i == NOTFOUND) return NOTFOUND;
     return ((i = i+2)*1000);
}

void print_funct_exit(char *f_name) {
     fprintf(fp, "(ASSIGN,%s,M[FP+1])\n", reg_push(s_size++));
     fprintf(fp, "(ASSIGN,SP,M[FP+2])\n(ASSIGN,FP,M[FP+3])\n");
     fprintf(fp, "(JUMP,%s)\n", reg_refrence(s_size - 1));
     return;
}

void code_gen(tree * p)
{
	fp = fopen(OUTFILE, "w");	/* open file for write only access */

	/* tuple code to call main subroutine */
	fprintf(fp, "(ASSIGN,FP,BP)\n(ADDI,SP,BP,%d)\n", (num_global_vars() + 1));
	fprintf(fp, "(ASSIGN,M[SP+1],1000)\n(ASSIGN,M[SP+2],SP)\n(ASSIGN,M[SP+3],FP)\n(ASSIGN,FP,SP)\n(ADDI,SP,SP,%d)\n(JUMP,%d)\n",
		(num_local_vars("main") + 5), get_funct_label("main"));

	/* exit section of tuple code */
	fprintf(fp,"\n(LABEL,1000)\n%s","(EXIT)\n\n");
	
	/* begin the actual process */
	code_gen_trav(p);

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
	        enter_ctrlstmt();
	        fprintf(fp, "# begin while stmt #\n");
		if(p->child1->nodeTag == ID || p->child1->nodeTag == LITEXP) {
		     fprintf(fp, "(LABEL,%d)\n", add_label());
		     fprintf(fp, "(ASSIGN,%s,", reg_push(s_size++));
		     code_gen_trav(p->child1);
		     fprintf(fp, ")\n");
		} else {
		fprintf(fp, "(LABEL,%d)\n", add_label());
		code_gen_trav(p->child1);
		}
		fprintf(fp, "(JNZ,%s,%d)\n", reg_refrence(s_size - 1), add_label());
		fprintf(fp, "(JUMP,%d)\n", add_label());
		fprintf(fp, "(LABEL,%d)\n", refrence_label(2)); /* label_size - 2 */
		if (p->next != NULL) { code_gen_trav(p->next); } /* while loop body */
		fprintf(fp, "(JUMP,%d)\n", refrence_label(3)); /* label_size - 3 */
		fprintf(fp, "(LABEL,%d)\n", refrence_label(1)); /* label_size - 1 */
		fprintf(fp, "# end while stmt #\n");
		leave_ctrlstmt();
		break;

	case IFTHENELSE_STMT:
	        enter_ctrlstmt();
	        fprintf(fp, "# begin if-then-else stmt #\n");
		if(p->child1->nodeTag == ID || p->child1->nodeTag == LITEXP) {
		     fprintf(fp, "(ASSIGN,%s,", reg_push(s_size++));
		     code_gen_trav(p->child1);
		     fprintf(fp, ")\n");
		} else {
		code_gen_trav(p->child1);
		}
		fprintf(fp, "(JNZ,%s,%d)\n", reg_refrence(s_size - 1), add_label());
		if (p->next != NULL) { code_gen_trav(p->next1); } /* then statements */
		fprintf(fp, "(JUMP,%d)\n", add_label());
		fprintf(fp, "(LABEL,%d)\n", refrence_label(2)); /* label_size - 2 */ 
		if (p->next1 != NULL) { code_gen_trav(p->next); } /* else statements */
		fprintf(fp, "(LABEL,%d)\n", refrence_label(1)); /* label_size - 1 */
		fprintf(fp, "# end if-then-else stmt #\n");
		leave_ctrlstmt();
		break;

	case PROCS:
		code_gen_trav(p->child1);
		if (p->next != NULL) {
			code_gen_trav(p->next);
		}
		break;

	case PROCDECL:
	        /* code_gen_trav(p->child1); /* NAME */
		/* if (p->next != NULL) { code_gen_trav(p->next); } /* var list */
	        fprintf(fp, "(LABEL,%d) # %s #\n", get_funct_label(p->child1->nodeName), p->child1->nodeName);
		if (p->next1 != NULL) { code_gen_trav(p->next1); } /* decl list */
		if (p->next2 != NULL) { code_gen_trav(p->next2); } /* stmt list */
		print_funct_exit(p->child1->nodeName);
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
