#ifndef	SYMBOL_H
#define	SYMBOL_H
/*
*	Symbol table management for toy language compiler.
*/
extern int	symbol_insert(char *, int, char *, int);
extern void	symbol_declare(int);

/* extern void     symbolscan(tree *, int, char *, int); */  
extern void     print_symboltable(void);
/* extern int      count_func_args(tree *); */
extern void     mk_symboltable(tree *);
extern void     start_symboltable(tree *);

extern int	symbol_find(char *);
extern char*    symbol_name(int);
extern int	symbol_type(int);
extern int 	symbol_declared(int);
#endif
