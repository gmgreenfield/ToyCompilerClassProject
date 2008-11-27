#include        <stdio.h>			/* for (f)printf(), std{out,int} */
#include        <string.h>			/* for strcmp, strdup & friends */
#include        <stdlib.h>			/* for exit */
#include        <stdio.h>
#include        "globals.h"
#include        "toy.tab.h"			/* token type definitions from .y file */
#include        "symbol.h"
#include        "generate.h"   

extern ENTRY symbol_table[MAX_SYMBOLS];
extern int n_symbol_table;

static FILE *fp; /* local to within this file */

void code_gen(tree *p) {
    fp = fopen(OUTFILE, "w"); /* open file for write only access */
    
    /* begin the actual process */
    code_gen_trav(p);
    
    fprintf(fp,"%s","(EXIT)\n");
    fclose(fp);

    printf("Tuple code has been written to %s.\nExited normally.\n", OUTFILE);
    return;
}

void code_gen_trav(tree *p) {
    switch(p->nodeTag) {
         case PROGRAM:
           if (p->next != NULL) { code_gen_trav(p->next); }
           if (p->next1 != NULL) { code_gen_trav(p->next1); }
         break;

         case WHILE_STMT:
           code_gen_trav(p->child1);
           if (p->next != NULL) { code_gen_trav(p->next); }
         break;

         case IFTHENELSE_STMT:
           code_gen_trav(p->child1);
           if (p->next != NULL) { code_gen_trav(p->next); }
           if (p->next1 != NULL) { code_gen_trav(p->next1); }
         break;

         case PROCS:
	   code_gen_trav(p->child1);
	   if (p->next != NULL) { code_gen_trav(p->next); }
	 break;

         case PROCDECL:
	   code_gen_trav(p->child1);
	   if (p->next != NULL) { code_gen_trav(p->next); }
	   if (p->next1 != NULL) { code_gen_trav(p->next1); }
	   if (p->next2 != NULL) { code_gen_trav(p->next2); }
	 break;

         case VARS:
	      code_gen_trav(p->child1);
	      if (p->next != NULL) { code_gen_trav(p->next); }
	      break;

         case VARS_TAIL:
	      if (p->next != NULL) { code_gen_trav(p->next); }
	      break;

         case DECLS:
	      code_gen_trav(p->child1);
	      if (p->next != NULL) { code_gen_trav(p->next); }
	      break;
	      
         case ARGS:
              code_gen_trav(p->child1);
              if (p->next != NULL) { code_gen_trav(p->next); }
              break;

         case ARGS_TAIL:
              if (p->next != NULL) { code_gen_trav(p->next); }
              break;

         case F_CALL:
              code_gen_trav(p->child1);
              if (p->next != NULL) { code_gen_trav(p->next); }
              break;

         case STMTS:
	      code_gen_trav(p->child1);
	      if (p->next != NULL) { code_gen_trav(p->next); }
              break;

         case DECL:
	     //printf("declare %s",p->nodeName);
              break;

         case ASSIGNSTMT:
	      fprintf(fp, "(ASSIGN,");
              code_gen_trav(p->child1);         
              code_gen_trav(p->child2);
              break;
	      
         case READSTMT:
	     fprintf(fp, "(READI,M[");
	     code_gen_trav(p->child1);
	     fprintf(fp, "])\n");
	     break;

         case WRITESTMT:
              code_gen_trav(p->child1);                       
              break;

         case PLUSEXP:                 
              code_gen_trav(p->child1);         
              code_gen_trav(p->child2);                  
              break;

         case MINUSEXP:                   
              code_gen_trav(p->child1);         
              code_gen_trav(p->child2);                  
              break;

         case LITEXP:
	     fprintf(fp, "%d", p->nodeVal);
	     break;

         case ID:
	     fprintf(fp, "%s+%d", var_type(symbol_find(p->nodeName, VARIABLE)), var_offset(symbol_find(p->nodeName, VARIABLE)));
	     break;
     }
    return;
}
