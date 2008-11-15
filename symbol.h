#ifndef	SYMBOL_H
#define	SYMBOL_H
/*
*	Symbol table management for toy language compiler.
*/
extern int	symbol_insert(char* name, int entry_type);
extern void	symbol_declare(int i);

extern int	symbol_find(char* name);
extern char*    symbol_name(int i);
extern int	symbol_type(int i);
extern int 	symbol_declared(int i);
#endif
