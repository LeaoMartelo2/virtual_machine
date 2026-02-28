#ifndef SPEC_H
#define SPEC_H

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#define MAX_PROGRAM_SIZE 32767
#define MAX_STACK_SIZE 256

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
    REGISTER_DUMP,
    PROGRAM_DUMP,
    TOGGLE_VERBOSE,
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
    ADD,
    SUB,
    MUL,
    DIV,
    MOD,
    PUSH,
    I_PUSH,
    POP,
    VOID_POP,
    RET,

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
    [NO_OP]          =  { "no_op", 0,          { ARG_NONE}},
    [HALT]           =  { "halt", 0,           { ARG_NONE}},
    [STATE_DUMP]     =  { "state_dump", 0,     { ARG_NONE}},
    [REGISTER_DUMP]  =  { "register_dump", 2,  { ARG_REG, ARG_REG}},
    [PROGRAM_DUMP]   =  { "program_dump", 0,   { ARG_NONE}},
    [TOGGLE_VERBOSE] =  { "toggle_verbose", 1, { ARG_VAL}},
    [MOV]            =  { "mov", 2,            { ARG_VAL, ARG_REG}},
    [LD]             =  { "ld", 2,             { ARG_REG, ARG_REG}},
    [INC]            =  { "inc", 1,            { ARG_REG}},
    [DEC]            =  { "dec", 1,            { ARG_REG}},
    [STO_PC]         =  { "sto_pc", 1,         { ARG_REG}},
    [CMP]            =  { "cmp", 2,            { ARG_REG, ARG_REG}},
    [JMP]            =  { "jmp", 1,            { ARG_VAL}},
    [JE]             =  { "je", 1,             { ARG_VAL}},
    [JNE]            =  { "jne", 1,            { ARG_VAL}},
    [JGE]            =  { "jge", 1,            { ARG_VAL}},
    [JLE]            =  { "jle", 1,            { ARG_VAL}},
    [ADD]            =  { "add", 2,            { ARG_REG, ARG_REG}},
    [SUB]            =  { "sub", 2,            { ARG_REG, ARG_REG}},
    [MUL]            =  { "mul", 2,            { ARG_REG, ARG_REG}},
    [DIV]            =  { "div", 2,            { ARG_REG, ARG_REG}},
    [MOD]            =  { "mod", 2,            { ARG_REG, ARG_REG}},
    [PUSH]           =  { "push", 1,           { ARG_REG}},
    [I_PUSH]         =  { "i_push", 1,         { ARG_VAL}},
    [POP]            =  { "pop", 1,            { ARG_REG}},
    [VOID_POP]       =  { "void_pop", 0,       { ARG_NONE}},
    [RET]            =  { "ret", 1,            { ARG_VAL}},
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
    i32 stack[MAX_STACK_SIZE];
    i32 stack_head;
    bool halted;
    bool verbose;
} VM;

#endif /* SPEC_H */
