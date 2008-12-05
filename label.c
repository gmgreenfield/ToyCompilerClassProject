#include "label.h"

int ptr_stack_push(int id) {
    ptr_stack[ptr_size++] = id;
    return ptr_stack[ptr_size - 1];
}

int label_stack_push(int id) {
    label_stack[label_size++] = id;
    return label_stack[label_size - 1];
}

int ptr_stack_pop(void) {
    return ptr_stack[ptr_size--];
}

int label_stack_pop(void) {
    return label_stack[label_size--];
}

void enter_ctrlstmt(void) {
    ptr_stack_push(label_size);
    return;
}

void leave_ctrlstmt(void) {
     int i, j;
     i = label_size;
     ptr_stack_pop();
     j = ptr_stack[ptr_size];
     do { label_stack_pop(); } while(--i > j);
     return;
}

/* add an entry to label_stack */
int add_label(void) {
     static int total = 1;
     return label_stack_push(total++);
}

/* refrence an entry from label_stack */
int refrence_label(int id) {
     return label_stack[label_size - id];
}
