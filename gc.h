#ifndef __GC_H__
#define __GC_H__

#include "vm.h"

typedef struct cons_cell {
    stack_entry head, tail;
} cons_cell;


void gc_root_update(const int i);
void gc_allocate(const int to, const int head, const int tail);
void gc_del_root(const int i);
void gc_dup_root(const int to, const int from);
void gc_swap_root(const int i, const int j);
void gc_collect(void);
void gc_reset(void);


#endif
