#ifndef __VM_H__
#define __VM_H__

#define MAX_BYTECODE 65536
#define STACK_SIZE   4096

/* instructions */
#define HALT   0x00
#define JUMP   0x01
#define JNZ    0x02
#define DUP    0x03
#define SWAP   0x04
#define DROP   0x05
#define PUSH4  0x06
#define PUSH2  0x07
#define PUSH1  0x08
#define ADD    0x09
#define SUB    0x0a
#define MUL    0x0b
#define DIV    0x0c
#define MOD    0x0d
#define EQ     0x0e
#define NE     0x0f
#define LT     0x10
#define GT     0x11
#define LE     0x12
#define GE     0x13
#define NOT    0x14
#define AND    0x15
#define OR     0x16
#define INPUT  0x17
#define OUTPUT 0x18

#define CLOCK  0x2a

#define CONS   0x30
#define HD     0x31
#define TL     0x32

#define RELOAD 0x40


typedef union stack_entry {
    int integer;
    struct cons_cell *heap_pointer;
} stack_entry;

typedef struct instruction {
    void *label; /* Directly Threaded */
    unsigned char args[4];
} instruction;

extern stack_entry stack[STACK_SIZE];


#endif
