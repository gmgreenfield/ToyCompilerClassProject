#ifndef GENERATE_H
#define GENERATE_H

void code_gen(struct tree *);
void code_gen_trav(struct tree *);

#define OUTFILE    "out.ty" /* default name for the tuple code file */

#endif
