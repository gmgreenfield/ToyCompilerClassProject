#ifndef	SYMBOL_H
#define	SYMBOL_H
/*
*	Symbol table mgmt and static checks for toy+ language compiler.
*/

struct tree;

/* symbol table generation */
extern int	symbol_insert(char *, int, char *, int);
extern void	symbol_declare(int);

/* symbol table generation */
extern void     symbolscan(struct tree *, int, char *, int);  
extern void     print_symboltable(void);
extern int      count_func_args(struct tree *);
extern void     mk_symboltable(struct tree *);
extern void     start_symboltable(struct tree *);

/* static checks */
extern void     static_checks(struct tree *);
extern int      proc_call_check(struct tree *);
extern int      var_define_check(struct tree *);

/* tuple code generation */
extern char *   variable_type(char *);
extern int      variable_offset(char *);

/* tuple code generation */
extern int      num_global_vars(void);
extern int      num_local_vars(char *);

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

#define		MAX_SYMBOLS	100      /* the maximum capactiy of the symbol table */
#define         ROUTINE         0
#define         VARIABLE        1
#define         NOTFOUND        -1
#define         FP_BASE         4        /* start incrementing registers at these numbers FP_BASE+n where n=0,1,... */
#define         BP_BASE         1

#endif
