#include "vm.h"
#include "gc.h"

#define HEAP_SIZE 65536

char is_root[STACK_SIZE]; /* initialized to 0 */
cons_cell heap[HEAP_SIZE];
cons_cell *from_space = &heap[0],
          *to_space   = &heap[HEAP_SIZE/2],
          *next = &heap[0], /* = from_space, */
          *scan;

void gc_reset(void) {
    int i;
    for (i = 0; i < STACK_SIZE; ++i)
        is_root[i] = 0;
    next = from_space;
}

char points_to(const cons_cell* const space, cons_cell* const p);
void gc_root_update(const int i) {
    if (!points_to(from_space, stack[i].heap_pointer))
        is_root[i] = 0;
} /* as it is it might mistake an int for a root */

void gc_allocate(const int to, const int head, const int tail) {
    if (next > from_space + HEAP_SIZE/2 - 1) gc_collect();
    is_root[head] = is_root[tail] = 0;
    is_root[to] = 1;
    next->head = stack[head];
    next->tail = stack[tail];
    stack[to].heap_pointer = next++;
}

/* these 3 are needed becasue this gc moves things around in the heap */
void gc_del_root(const int i) { is_root[i] = 0; }
void gc_dup_root(const int to, const int from) { is_root[to] = is_root[from]; }
void gc_swap_root(const int i, const int j) {
    const char temp = is_root[i];
    is_root[i] = is_root[j];
    is_root[j] = temp;
}


/* Cheney's Copying Garbage Collector */

char points_to(const cons_cell* const space, cons_cell* const p) {
    return &space[0] <= p && p < &space[HEAP_SIZE/2];
} /* possibly a heap pointer */

cons_cell* forward(cons_cell* const p) {
    if (points_to(from_space, p))
        if (points_to(to_space, p->head.heap_pointer))
            return p->head.heap_pointer;
        else {
            next->head = p->head;
            next->tail = p->tail;
            p->head.heap_pointer = next;
            ++next; /* next = next + size(*p) */
            return p->head.heap_pointer;
        }
    else
        return p;
}
/* small issue: if an item happens to have a value in it's first field that 
 * looks like a to_space pointer, it will get collected lol -> probably seg
 * fault for the vm (I was borred to implement types for every instruction)
 * (I could set (and check) to both fields the same value but there'd still
 * be a similar issue) (could also instead add forwarded field to cons_cell
 * head and tail)
 */

void gc_collect(void) {
    next = scan = &to_space[0];
    int i;
    for (i = STACK_SIZE - 1; i >= 0; --i) /* i >= sp_index; ? */
        if (is_root[i])
            stack[i].heap_pointer = forward(stack[i].heap_pointer);
    while (scan < next) {
        scan->head.heap_pointer = forward(scan->head.heap_pointer);
        scan->tail.heap_pointer = forward(scan->tail.heap_pointer);
        ++scan; /* scan = scan + size(*scan) */
    }

    cons_cell *temp = from_space;
    from_space = to_space;
    to_space = temp;
}
