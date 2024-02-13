#include <time.h>   /* clock, clock_t, CLOCKS_PER_SEC */
#include <stdio.h>
#include <stdlib.h> /* exit */

#include "vm.h"
#include "gc.h"


stack_entry stack[STACK_SIZE];
instruction byte_code[MAX_BYTECODE];

/* (for debuging, top is printed left)
void stack_dump(const stack_entry* const sp) {
    const stack_entry *i = sp;
    printf("[ ");
    while (i < &stack[STACK_SIZE]) {
        printf("%d ", i->integer);
        ++i;
    }
    printf("]\n");
}
*/

/* little-endian */
int const2(unsigned char a[2]) { return a[1] << 8 | a[0]; }
int const4(unsigned char a[4]) {
    return a[3] << 24 | a[2] << 16 | a[1] << 8 | a[0];
}
/*
int const(int n, unsigned char a[n]) { n must be power of 2
    if (n == 1) return a[0];
    return const(n - 1, &a[n/2]) << n*4 | const(n - 1, a);
}
*/

void error(const char* const message) {
    fprintf(stderr, message);
    fprintf(stderr, "\n");
    exit(1);
}

unsigned char buffer[MAX_BYTECODE];

void loader_copy_args(const int nargs, const int op) {
    for (int k = 0; k < nargs; ++k)
        byte_code[op].args[k] = buffer[op + k + 1];
}

int main(const int argc, const char* const argv[]) {
    if (argc != 2) error("[Usage Error] Usage: ./vm byte-code.b");

    reload_label: /* dynamic code reloading */
        FILE *fp = fopen(argv[1], "r");
        if (fp == NULL) error("[File Error] Couldn't open file");

        const int n = fread(buffer, 1, MAX_BYTECODE, fp);
        fclose(fp);
        if (n > MAX_BYTECODE) error("[Size Error] Program too long\n");

        /* loader pass (can't put in seperate function because of labels) */
        int i;
        for (i = 0; i < n; ++i)
            switch (buffer[i]) {
                case HALT: byte_code[i].label = &&halt_label; break;
                case JUMP:
                    byte_code[i].label = &&jump_label;
                    *(byte_code[i].args) = const2(&buffer[i + 1]);
                    break;
                case JNZ:
                    byte_code[i].label = &&jnz_label;
                    *(byte_code[i].args) = const2(&buffer[i + 1]);
                    break;
                case DUP:
                    byte_code[i].label = &&dup_label;
                    byte_code[i].args[0] = buffer[i + 1];
                    break;
                case SWAP: /* combining instructions */
                    if (buffer[i + 2] == SWAP && buffer[i + 1] == buffer[i + 3])
                        byte_code[i].label = &&double_swap_label;
                    else {
                        byte_code[i].label = &&swap_label;
                        byte_code[i].args[0] = buffer[i + 1];
                    }
                    break;
                case DROP:   byte_code[i].label = &&drop_label;   break;
                case PUSH4:
                    byte_code[i].label = &&push4_label;
                    *(byte_code[i].args) = const4(&buffer[i + 1]);
                    break;
                case PUSH2:
                    byte_code[i].label = &&push2_label;
                    *(byte_code[i].args) = const2(&buffer[i + 1]);
                    break;
                case PUSH1:
                    byte_code[i].label = &&push1_label;
                    byte_code[i].args[0] = buffer[i + 1];
                    break;
                case ADD:    byte_code[i].label = &&add_label;    break;
                case SUB:    byte_code[i].label = &&sub_label;    break;
                case MUL:    byte_code[i].label = &&mul_label;    break;
                case DIV:    byte_code[i].label = &&div_label;    break;
                case MOD:    byte_code[i].label = &&mod_label;    break;
                case EQ:     byte_code[i].label = &&eq_label;     break;
                case NE:     byte_code[i].label = &&ne_label;     break;
                case LT:     byte_code[i].label = &&lt_label;     break;
                case GT:     byte_code[i].label = &&gt_label;     break;
                case LE:     byte_code[i].label = &&le_label;     break;
                case GE:     byte_code[i].label = &&ge_label;     break;
                case NOT:    byte_code[i].label = &&not_label;    break;
                case AND:    byte_code[i].label = &&and_label;    break;
                case OR:     byte_code[i].label = &&or_label;     break;
                case INPUT:  byte_code[i].label = &&input_label;  break;
                case OUTPUT: byte_code[i].label = &&output_label; break;
                case CLOCK:  byte_code[i].label = &&clock_label;  break;
                case CONS: switch (buffer[i + 1]) { /* combining instructions */
                    case HD: byte_code[i].label = &&cons_hd_label; break;
                    case TL: byte_code[i].label = &&cons_tl_label; break;
                    default: byte_code[i].label = &&cons_label;    break;
                } break;
                case HD:     byte_code[i].label = &&hd_label;     break;
                case TL:     byte_code[i].label = &&tl_label;     break;
                case RELOAD: byte_code[i].label = &&reload_label; break; 

                default:
                    byte_code[i].label = &&invalid_label;
                    *(byte_code[i].args) = i;
            }
        byte_code[i].label = &&halt_label;
        /* program ends after last instruction
         * (should have declared byte_code[MAX_BYTECODE + 1] but whatever)
         */


        /* begin interpreting */
        stack_entry *sp = &stack[STACK_SIZE]; /* grows down */
        instruction *pc = &byte_code[0];
        #define NEXT_INSTRUCTION goto *pc->label
        gc_reset();

        unsigned arg, sp_index;
        stack_entry tmp;

        clock_t start, now;
        start = clock();
        NEXT_INSTRUCTION;
    jump_label:
        pc = &byte_code[*pc->args];
        NEXT_INSTRUCTION;
    jnz_label:
        pc = sp[0].integer ? &byte_code[*pc->args] : pc + 3;
        ++sp;
        NEXT_INSTRUCTION;
    dup_label:
        arg = pc->args[0];
        pc += 2; /* prefetching */
        sp_index = sp - stack;
        gc_dup_root(sp_index - 1, sp_index + arg);
        *(sp - 1) = sp[arg];
        --sp;
        NEXT_INSTRUCTION;
    swap_label:
        arg = pc->args[0];
        pc += 2;
        sp_index = sp - stack;
        gc_swap_root(sp_index, sp_index + arg);
        tmp = sp[arg];
        sp[arg] = sp[0];
        sp[0] = tmp;
        NEXT_INSTRUCTION;
    drop_label:
        ++pc;
        gc_del_root(sp - stack);
        ++sp;
        NEXT_INSTRUCTION;
    push4_label:
        arg = *pc->args;
        pc += 5;
        (--sp)->integer = arg;
        NEXT_INSTRUCTION;
    push2_label:
        arg = *pc->args;
        pc += 3;
        (--sp)->integer = arg;
        NEXT_INSTRUCTION;
    push1_label:
        arg = pc->args[0];
        pc += 2;
        (--sp)->integer = arg;
        NEXT_INSTRUCTION;
    add_label:
        ++pc;
        sp[1].integer += sp[0].integer;
        ++sp;
        NEXT_INSTRUCTION;
    sub_label:
        ++pc;
        sp[1].integer -= sp[0].integer;
        ++sp;
        NEXT_INSTRUCTION;
    mul_label:
        ++pc;
        sp[1].integer *= sp[0].integer;
        ++sp;
        NEXT_INSTRUCTION;
    div_label:
        ++pc;
        sp[1].integer /= sp[0].integer;
        ++sp;
        NEXT_INSTRUCTION;
    mod_label:
        ++pc;
        sp[1].integer %= sp[0].integer;
        ++sp;
        NEXT_INSTRUCTION;
    eq_label:
        ++pc;
        sp[1].integer = sp[1].integer == sp[0].integer;
        ++sp;
        NEXT_INSTRUCTION;
    ne_label:
        ++pc;
        sp[1].integer = sp[1].integer != sp[0].integer;
        ++sp;
        NEXT_INSTRUCTION;
    lt_label:
        ++pc;
        sp[1].integer = sp[1].integer < sp[0].integer;
        ++sp;
        NEXT_INSTRUCTION;
    gt_label:
        ++pc;
        sp[1].integer = sp[1].integer > sp[0].integer;
        ++sp;
        NEXT_INSTRUCTION;
    le_label:
        ++pc;
        sp[1].integer = sp[1].integer <= sp[0].integer;
        ++sp;
        NEXT_INSTRUCTION;
    ge_label:
        ++pc;
        sp[1].integer = sp[1].integer >= sp[0].integer;
        ++sp;
        NEXT_INSTRUCTION;
    not_label:
        ++pc;
        sp[0].integer = !sp[0].integer;
        NEXT_INSTRUCTION;
    and_label:
        ++pc;
        sp[1].integer = sp[1].integer && sp[0].integer;
        ++sp;
        NEXT_INSTRUCTION;
    or_label:
        ++pc;
        sp[1].integer = sp[1].integer || sp[0].integer;
        ++sp;
        NEXT_INSTRUCTION;
    input_label:
        ++pc;
        (--sp)->integer = getchar();
        NEXT_INSTRUCTION;
    output_label:
        ++pc;
        putchar(sp[0].integer);
        ++sp;
        NEXT_INSTRUCTION;
    clock_label:
        ++pc;
        now = clock();
        printf("%0.6lf\n", (double) (now - start)/CLOCKS_PER_SEC);
        NEXT_INSTRUCTION;
    cons_label:
        ++pc;
        sp_index = sp - stack;
        gc_allocate(sp_index + 1, sp_index + 1, sp_index);
        gc_del_root(sp_index);
        ++sp;
        NEXT_INSTRUCTION;
    hd_label:
        ++pc;
        sp[0] = sp[0].heap_pointer->head;
        gc_root_update(sp - stack);
        NEXT_INSTRUCTION;
    tl_label:
        ++pc;
        sp[0] = sp[0].heap_pointer->tail;
        gc_root_update(sp - stack);
        NEXT_INSTRUCTION;

    /* combined instructions */
    double_swap_label:
        pc += 4;
        NEXT_INSTRUCTION;
    cons_hd_label:
        pc += 2;
        gc_del_root(sp - stack);
        ++sp;
        NEXT_INSTRUCTION;
    cons_tl_label:
        pc += 2;
        sp_index = sp - stack;
        gc_swap_root(sp_index, sp_index + 1);
        gc_del_root(sp_index);
        sp[1] = sp[0];
        ++sp;
        NEXT_INSTRUCTION;

    /* other ideas
    hd_cons, tl_cons ?
    jeq_labe:
    jne_label:
    jlt_label:
    ...
    push4_add_label: (add4_label ?)
    ...
    [instruction]_swap_label: in general should be good
    put_str, get_str for multiple output/input
    */

    invalid_label:
        printf(
            "[Runtime Error] Invalid bytecode encountered at byte %d\n",
            *(int*)& pc->args
        );
        exit(1);

    halt_label:
    /* (print some stats)
    now = clock();
    double time = (double) (now - start)/CLOCKS_PER_SEC;
    printf("\n---------------\n");
    printf("Machine Halted.\n");
    printf("Time: %0.6lfs\n", time);
    */
    return 0;
}
