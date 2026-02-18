#ifndef SPEC_H
#define SPEC_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_PROGRAM_SIZE 32767

typedef uint32_t u32;

typedef enum {
    REG_0 = 0,
    REG_1,
    REG_2,
    REG_3,
    REG_4,
    REG_5,
    REG_6,
    REG_7,
    REG_COUNT,
} Registers;

typedef enum {
    NO_OP = 0,
    HALT,
    STATE_DUMP,
    MOV, 
    LD,  

} Opcodes;

typedef struct {

    u32 program_counter;
    u32 program[MAX_PROGRAM_SIZE];
    u32 program_size;
    u32 registers[REG_COUNT];
    bool halted;
} VM;

#endif /* SPEC_H */
