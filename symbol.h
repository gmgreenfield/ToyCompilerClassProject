#ifndef	SYMBOL_H
#define	SYMBOL_H
/*
*	Symbol table management for toy language compiler.
*/

struct tree;

extern int	symbol_insert(char *, int, char *, int);
extern void	symbol_declare(int);

extern void     symbolscan(struct tree *, int, char *, int);  
extern void     print_symboltable(void);
extern int      count_func_args(struct tree *);
extern void     mk_symboltable(struct tree *);
extern void     start_symboltable(struct tree *);
extern void     static_checks(struct tree *);
extern int      proc_call_check(struct tree *);
extern int      var_define_check(struct tree *);

extern int	symbol_find(char *, int);
extern char*    symbol_name(int);
extern int	symbol_type(int);
extern int 	symbol_declared(int);
#endif
