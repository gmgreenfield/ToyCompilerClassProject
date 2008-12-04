#ifndef GENERATE_H
#define GENERATE_H

struct tree;

char *reg_push(int);
char *reg_refrence(int);
void reg_print(void);
void trav_exp(struct tree *);
void code_gen(struct tree *);
void code_gen_trav(struct tree *);

#define		   STACK_MAX	   300      /* maximum depth of register stack */
#define            RECORD_MAX      100      /* maximum depth of record table */
#define            OUTFILE         "out.ir" /* default name for the tuple code output file */

#endif
