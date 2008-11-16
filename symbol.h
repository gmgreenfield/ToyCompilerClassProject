#ifndef	SYMBOL_H
#define	SYMBOL_H
/*
*	Symbol table management for toy language compiler.
*/
extern int	symbol_insert(char *, int, char *);
extern void	symbol_declare(int);

/* extern void     symbolscan(tree *, int, char *); */    
extern void     print_symboltable(void);

extern int	symbol_find(char *);
extern char*    symbol_name(int);
extern int	symbol_type(int);
extern int 	symbol_declared(int);
#endif
