#ifndef SPEC_H
#define SPEC_H

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#define UNUSED(x) ((void)(x))

#define MAX_PROGRAM_SIZE 32767
#define MAX_STACK_SIZE 256

#define MAX_EXTERNAL_LIBS 5

#define VM_MAGIC 0x525F4D56 /* VM_R */

#ifndef VM_VERSION
    #define VM_VERSION -1
#endif

#ifndef BUILD_DATE
    #define BUILD_DATE "Unknown"
#endif

#ifndef GIT_HASH
    #define GIT_HASH "Unknown"
#endif


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
    REG_11,
    REG_12,
    REG_13,
    REG_14,
    REG_15,
    REG_16, 
    REG_17,
    REG_18,
    REG_19,
    REG_20,

    NAMED_REGISTERS_SPLIT,


    REG_ARG_A = 50,
    REG_ARG_B,
    REG_ARG_C,
    REG_ARG_D,
    REG_RET,

    REG_RAM_START,
    REG_HEAP_PTR,
    REG_NULL,

    REG_COUNT

} Registers;

typedef struct {
    const char *name;
    Registers reg_idx;
} Named_register;

typedef enum : i32 {
    NO_OP = 0,
    HALT,
    STATE_DUMP,
    REGISTER_DUMP,
    PROGRAM_DUMP,
    STACK_DUMP,
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
    CALL,
    RET,
    SYSCALL,
    STRLEN,
    STRLEN_R,
    PRINT_CHAR,
    PRINT_INT,
    IPRINT_CHAR,
    IPRINT_INT,
    LINE_BR,
    LDO,
    LDXO,
    RDINT,
    AND,
    OR, 
    XOR,
    NOT,
    LSH,
    RSH,
    LSHA,
    RSHA,
    STR,
    DLOPEN,
    EXTERN,

    OPCODE_COUNT
} Opcodes;

#define MAX_ARGUMENT_COUNT 4

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

static const Instruction_spec ASSEMBLY_TABLE[] =  {
    [NO_OP]          =  { "no_op", 0,          { ARG_NONE}},
    [HALT]           =  { "halt", 0,           { ARG_NONE}},
    [STATE_DUMP]     =  { "state_dump", 0,     { ARG_NONE}},
    [REGISTER_DUMP]  =  { "register_dump", 2,  { ARG_REG, ARG_REG}},
    [PROGRAM_DUMP]   =  { "program_dump", 0,   { ARG_NONE}},
    [STACK_DUMP]     =  { "stack_dump", 0,     { ARG_NONE}},
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
    [CALL]           =  { "call", 1,           { ARG_VAL}},
    [RET]            =  { "ret", 0,            { ARG_NONE}},
    [SYSCALL]        =  { "syscall", 0,        { ARG_NONE}},
    [STRLEN]         =  { "strlen", 2,         { ARG_VAL, ARG_REG}},
    [STRLEN_R]       =  { "strlen_r", 2,       { ARG_REG, ARG_REG}},
    [PRINT_CHAR]     =  { "print_char", 1,     { ARG_REG}},
    [PRINT_INT]      =  { "print_int", 1,      { ARG_REG}},
    [IPRINT_CHAR]    =  { "iprint_char", 1,    { ARG_VAL}},
    [IPRINT_INT]     =  { "iprint_int", 1,     { ARG_VAL}},
    [LINE_BR]        =  { "line_br", 0,        { ARG_NONE}},
    [LDO]            =  { "ldo", 2,            { ARG_VAL, ARG_REG}},
    [LDXO]           =  { "ldxo", 3,           { ARG_REG, ARG_REG, ARG_REG}},
    [RDINT]          =  { "rdint", 1,          { ARG_REG}},
    [AND]            =  { "and", 2,            { ARG_REG, ARG_REG}},
    [OR]             =  { "or", 2,             { ARG_REG, ARG_REG}},
    [XOR]            =  { "xor", 2,            { ARG_REG, ARG_REG}},
    [NOT]            =  { "not", 1,            { ARG_REG}},
    [LSH]            =  { "lsh", 1,            { ARG_REG}},
    [RSH]            =  { "rsh", 1,            { ARG_REG}},
    [LSHA]           =  { "lsha", 2,           { ARG_REG, ARG_REG}},
    [RSHA]           =  { "rsha", 2,           { ARG_REG, ARG_REG}},
    [STR]            =  { "str", 2,            { ARG_REG, ARG_REG}},
    [DLOPEN]         =  { "dlopen", 2,         { ARG_VAL, ARG_REG}},
    [EXTERN]         =  { "extern", 2,         { ARG_VAL, ARG_REG}},
};

// :Tabularize /[={]
// align by "="  and "{"

static_assert((sizeof(ASSEMBLY_TABLE) / sizeof(ASSEMBLY_TABLE[0])) == OPCODE_COUNT,
              "Error: ASSEMBLY_TABLE must contain all elements of the OPCODES table");

typedef enum : i32 {
    COND_NEGATIVE = -1,
    COND_ZERO     =  0,
    COND_POSITIVE =  1,
} Cond_flags;


static const Named_register NAMED_REGISTERS[] = {

     { "arg_a", REG_ARG_A},
     { "arg_b", REG_ARG_B},
     { "arg_c", REG_ARG_C},
     { "arg_d", REG_ARG_D},
     { "ret",   REG_RET},
     { "ram_start", REG_RAM_START},
     { "heap", REG_HEAP_PTR},
     { "ram", REG_HEAP_PTR},
     { "reg_null", REG_NULL},
};

#define NAMED_REGISTER_COUNT (sizeof(NAMED_REGISTERS) / sizeof(Named_register))

typedef struct {
    const char *name;
    i32 value;
} Constant_spec;

typedef enum : i32 {
    WRITE_SYSCALL = 1,
    GETPID_SYSCALL = 2,
    KILL_SYSCALL = 3,
    OPEN_SYSCALL = 4,
    CLOSE_SYSCAL = 5,
    READ_SYSCALL = 6,
} Syscall_numbers;

static const Constant_spec PREDEFINED_CONSTANTS[] = {

    {"true", 1},
    {"false", 0},

    {"stdin", 0},
    {"stdout", 1},
    {"stderr", 2},

    {"O_RDONLY", 0},
    {"O_WRONLY", 1},
    {"O_RDWR", 2},
    {"O_CREAT", 64},
    {"O_TRUNC", 512},
    {"O_APPEND", 1024},

    {"S_IRUSR", 256},   /* 0400 */
    {"S_IWRSR", 128},   /* 0200 */
    {"S_IXUSR", 64},    /* 0100 */
    {"perm_0644", 420}, /* 0644 */
    {"perm_0755", 493}, /* 0755 */

    {"write_syscall", WRITE_SYSCALL},
    {"getpid_syscall", GETPID_SYSCALL},
    {"kill_syscall", KILL_SYSCALL},
    {"open_syscall", OPEN_SYSCALL},
    {"close_syscall", CLOSE_SYSCAL},
    {"read_syscall", READ_SYSCALL},


    {"RAM_START", MAX_PROGRAM_SIZE},
};


#define CONSTANT_COUNT (sizeof(PREDEFINED_CONSTANTS) / sizeof(Constant_spec))

// clang-format on


typedef struct {
    i32 magic;
    i32 version;
    i32 data_size;
    i32 program_start;
} VMFileHeader;

typedef struct {
    void *handle;
} VMExternHandle;

typedef i32 (*extern_signature)(i32, i32, i32, i32);

typedef struct {
    i32 program_counter;
    i32 program[MAX_PROGRAM_SIZE];
    i32 program_size;

    i32 data_offset;
    i32 data_size;

    i32 memory[MAX_PROGRAM_SIZE];

    i32 registers[REG_COUNT];
    i32 cond_flag;

    i32 stack[MAX_STACK_SIZE];
    i32 stack_head;

    i32 return_address_stack[MAX_STACK_SIZE];
    i32 return_address_head;

    VMExternHandle extern_handle[MAX_EXTERNAL_LIBS];
    i32 extern_handle_count;

    bool halted;
    bool verbose;
} VM;

#endif /* SPEC_H */
