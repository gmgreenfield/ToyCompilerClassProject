#ifndef LABEL_H
#define LABEL_H

#include "generate.h"

int ptr_stack_push(int);
int label_stack_push(int);
int ptr_stack_pop(void);
int label_stack_pop(void);
void enter_ctrlstmt(void);
void leave_ctrlstmt(void);
int add_label(void);
int refrence_label(int);

static int ptr_size = 0; /* size of pointer stack */
static int label_size = 1; /* size of label stack */
static int ptr_stack[STACK_MAX]; /* pointers to the nested levels of labels */
static int label_stack[RECORD_MAX]; /* stores the active labels in use */

#endif LABEL_H
