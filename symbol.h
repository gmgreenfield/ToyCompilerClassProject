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

typedef struct
{
    char *name;
    int declared;		        /* 1 if already declared, 0 else */
    int type;				/* tells if its a routine or a variable */
    char *local_to;			/* who the variable belongs to if in a proc, or null if its global */
    int num_args;                       /* will be 0 in the case of a variable */
    int fp_offset;                      /* variable is found at offset i from the frame pointer (fp and base offsets are mutually exclusive) */
    int base_addr_offset;               /* variable is found at offset i from the constant base address*/
} ENTRY;

#define		MAX_SYMBOLS	100
#define         ROUTINE         0
#define         VARIABLE        1
#define         NOTFOUND        -1
#define         FP_BASE         3

#endif
