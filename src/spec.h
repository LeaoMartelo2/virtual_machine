#ifndef SPEC_H
#define SPEC_H

#include <assert.h>
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

    OPCODE_COUNT
} Opcodes;

#define MAX_ARGUMENT_COUNT 3

typedef enum {
    ARG_NONE = 0,
    ARG_REG,
    ARG_VAL,
} Argument_type;

typedef struct {
    const char *name;
    int arg_count;
    Argument_type arg_types[MAX_ARGUMENT_COUNT];
} Instruction_spec;

// clang-format off

/*
   Table structure: [OPCODE] [ASM NAME] [ARG COUNT] [ARGUMENT TYPES]
*/

static const Instruction_spec ASSEMBLY_TABLE[] = {
    [NO_OP]        =  { "no_op", 0,        { ARG_NONE}},
    [HALT]         =  { "halt", 0,         { ARG_NONE}},
    [STATE_DUMP]   =  { "state_dump", 0,   { ARG_NONE}},
    [PROGRAM_DUMP] =  { "program_dump", 0, { ARG_NONE}},
    [MOV]          =  { "mov", 2,          { ARG_VAL, ARG_REG}},
    [LD]           =  { "ld", 2,           { ARG_REG, ARG_REG}},
    [INC]          =  { "inc", 1,          { ARG_REG}},
    [DEC]          =  { "dec", 1,          { ARG_REG}},
    [STO_PC]       =  { "sto_pc", 1,       { ARG_REG}},
    [CMP]          =  { "cmp", 2,          { ARG_REG, ARG_REG}},
    [JMP]          =  { "jmp", 1,          { ARG_REG}},
    [JE]           =  { "je", 1,           { ARG_REG}},
    [JNE]          =  { "jne", 1,          { ARG_REG}},
    [JGE]          =  { "jge", 1,          { ARG_REG}},
    [JLE]          =  { "jle", 1,          { ARG_REG}},
};

// :Tabularize /[={]
// align by "="  and "{"



static_assert((sizeof(ASSEMBLY_TABLE) / sizeof(ASSEMBLY_TABLE[0])) == OPCODE_COUNT,
              "Error: ASSEMBLY_TABLE must contain all elements of the OPCODES table");

typedef enum : i32 {
    COND_NEGATIVE = -1,
    COND_ZERO     = 0,
    COND_POSITIVE = 1,
} Cond_flags;

// clang-format on

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
