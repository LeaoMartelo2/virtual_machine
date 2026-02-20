#ifndef SPEC_H
#define SPEC_H

#include <stdbool.h>
#include <stdint.h>

#define MAX_PROGRAM_SIZE 32767

typedef int32_t i32;

typedef enum : i32 {
    REG_0 = 0,
    REG_1,
    REG_2,
    REG_3,
    REG_4,
    REG_5,
    REG_6,
    REG_7,
    REG_8,
    REG_9,
    REG_10,
    REG_COUNT,
} Registers;

typedef enum : i32 {
    NO_OP = 0,
    HALT,
    STATE_DUMP,
    PROGRAM_DUMP,
    MOV,
    LD,
    INC,
    DEC,
    STO_PC,
    CMP,
    JMP,
    JE,
    JNE,
    JGE,
    JLE,
} Opcodes;

typedef enum : i32 {
    COND_NEGATIVE = -1,
    COND_ZERO     =  0,
    COND_POSITIVE =  1, 
} Cond_flags;

typedef struct {

    i32 program_counter;
    i32 program[MAX_PROGRAM_SIZE];
    i32 program_size;
    i32 registers[REG_COUNT];
    i32 cond_flag;
    bool halted;
    bool verbose;
} VM;

#endif /* SPEC_H */
