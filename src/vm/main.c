#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../spec.h"
#include "opcodes.h"

#define ARR_LEN(arr) (sizeof(arr) / sizeof((arr)[0]))

void dump_ram_memory(VM *vm);

_Noreturn void print_usage(int argc, char **argv) {
    UNUSED(argc);
    printf("Usage: %s <program.bin> [flags]\n", argv[0]);
    printf("        -v, -verbose       Force verbose mode on.\n");
    printf("        -V, -version       Print version information and exit.\n");
    printf("        -md, -memdump      Dump RAM at program halt. (RAM.DATA)\n");
    printf("        -h, -help          Prints this message\n");
    exit(1);

}

_Noreturn void version_info(void){
    printf("VMASM Interpreter:\n");
    printf("Spec version:           %d\n", VM_VERSION);
    printf("Expected Magic Bytes:   %#x\n", VM_MAGIC);
    printf("Build:                  %s  \n", GIT_HASH);
    printf("Build date:             %s   \n", BUILD_DATE);
    exit(0);
}

int main(int argc, char **argv) {

    if (argc < 2) {
        //print_usage(argc, argv);
        printf("Usage: %s <program.bin> [flags]\nUse -help for more information.\n", argv[0]);
        exit(1);
    }

    bool forced_verbose = false;
    bool dump_ram = false;
    const char *program_file_path;

    for (int i = 1; i <argc; ++i) {

        if(strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "-verbose") == 0) { forced_verbose = true;}

        if(strcmp(argv[i], "-md") == 0 || strcmp(argv[i], "-memdump") == 0) { dump_ram = true;}

        if(strcmp(argv[i], "-V") == 0 || strcmp(argv[i], "-version") == 0 ) { version_info(); }

        /* first flag that does not start with - is file path*/
        if(argv[i][0] != '-') {program_file_path = argv[i];}

    }


    FILE *file = fopen(program_file_path, "rb");
    if (!file) {
        perror("Error oppening file.");
        exit(1);
    }

    // ========= read file header ===========

    VMFileHeader header;
    if(fread(&header, sizeof(VMFileHeader), 1, file) != 1) {
        fprintf(stderr, "Error: Failed to read file header\n");
        fclose(file);
        exit(1);
    }

    // validate
    if (header.magic != VM_MAGIC) {
        fprintf(stderr, "Error: invalid file format (bad magic bytes)\n");
        fclose(file);
        exit(1);
    }

    if (header.version != VM_VERSION) {
        fprintf(stderr, "ROM Spec version might be incompatible with interpreter version (Expected HASH:%d, got HASH:%d)\n", VM_VERSION, header.version);
    }

    // ============ load it in to program array =============

    i32 loaded_program[MAX_PROGRAM_SIZE];
    size_t total_loaded = fread(loaded_program, sizeof(i32), MAX_PROGRAM_SIZE, file);
    fclose(file);
    
    // ========= initialize vm ==============

    VM vm = {};

    memcpy(vm.program, loaded_program, sizeof(loaded_program));

    vm.program_size = (i32)total_loaded;
    vm.data_offset = header.program_start;
    vm.data_size = header.data_size;

    vm.program_counter = header.program_start;

    vm.halted = false;
    vm.verbose = forced_verbose;
    vm.stack_head = 0;
    memset(vm.stack, (i32)0, sizeof(vm.stack));
    memset(vm.registers, (i32)0, sizeof(vm.registers));
    // intentionally not clearing out the ram on program start
    //memset(vm.memory, (i32)0, sizeof(vm.memory));
    vm.registers[REG_RAM_START] = MAX_PROGRAM_SIZE;
    vm.registers[REG_HEAP_PTR] = MAX_PROGRAM_SIZE;
    memset(vm.return_address_stack, (i32)0, sizeof(vm.return_address_stack));
    vm.return_address_head = 0;

    printf("==== VM INIT ====\n");
    if(forced_verbose) printf("==== FORCING VERBOSE OUTPUT ====\n");
    
    while (!vm.halted) {
        
        if(forced_verbose && vm.verbose == false) {vm.verbose = true;}
        
        switch ((Opcodes)vm.program[vm.program_counter]) {

        case NO_OP: {
            no_op(&vm);
        } break;

        case HALT: {
            halt(&vm);
        } break;

        case STATE_DUMP: {
            state_dump(&vm);
        } break;

        case REGISTER_DUMP: {
            register_dump(&vm);
        } break;

        case PROGRAM_DUMP: {
            program_dump(&vm);
        } break;

        case STACK_DUMP: {
            stack_dump(&vm);
        } break;

        case TOGGLE_VERBOSE: {
            toggle_verbose(&vm);
        } break;

        case MOV: {
            mov(&vm);
        } break;

        case LD: {
            ld(&vm);
        } break;

        case INC: {
            inc(&vm);
        } break;

        case DEC: {
            dec(&vm);
        } break;

        case STO_PC: {
            sto_pc(&vm);
        } break;

        case CMP: {
            cmp(&vm);
        } break;

        case JMP: {
            jmp(&vm);
        } break;

        case JE: {
            je(&vm);
        } break;

        case JNE: {
            jne(&vm);
        } break;

        case JGE: {
            jge(&vm);
        } break;

        case JLE: {
            jle(&vm);
        } break;

        case ADD: {
            add(&vm);
        } break;

        case SUB: {
            sub(&vm);
        } break;

        case MUL: {
            mul(&vm);
        } break;

        case DIV: {
            div_(&vm);
        } break;

        case MOD: {
            mod(&vm);
        } break;

        case PUSH: {
            push(&vm);
        } break;

        case I_PUSH: {
            i_push(&vm);
        } break;

        case POP: {
            pop(&vm);
        } break;

        case VOID_POP: {
            void_pop(&vm);
        } break;

        case CALL: {
            call(&vm);
        } break;

        case RET: {
            ret(&vm);
        } break;

        case SYSCALL: {
            syscall_(&vm);
        } break;

        case STRLEN: {
            strlen_(&vm);
        } break;

        case STRLEN_R: {
            strlen_r(&vm);
        } break;

        case PRINT_CHAR: {
            print_char(&vm);
        } break;

        case PRINT_INT: {
            print_int(&vm);
        } break;

        case IPRINT_CHAR: {
            iprint_char(&vm);
        } break;

        case IPRINT_INT: {
            iprint_int(&vm);
        } break;

        case LINE_BR: {
            line_br(&vm);
        } break;

        case LDO: {
            ldo(&vm);
        } break;

        case LDXO: {
            ldxo(&vm);
        } break;

        case RDINT: {
            rdint(&vm);
        } break;

        case AND: {
            and_(&vm);
        } break;

        case OR: {
            or_(&vm);
        } break;

        case XOR: {
            xor_(&vm);
        } break;

        case NOT: {
            not_(&vm);
        } break;

        case LSH: {
            lsh(&vm);
        } break;

        case RSH: {
            rsh(&vm);
        } break;

        case LSHA: {
            lsha(&vm);
        } break;

        case RSHA: {
            rsha(&vm);
        } break;

        case STR: {
            str(&vm);
        } break;

        case OPCODE_COUNT: 
        default: {
            printf("BAD OPCODE, HALTING\n");
            vm.halted = true;
            break;
        }
        }
    }

    if(dump_ram) dump_ram_memory(&vm);

    return 0;
}


void dump_ram_memory(VM *vm) {
    
    FILE *file = fopen("RAM.DATA", "wb");
    if(!file) {
        exit(1);
    }

    fwrite(vm->memory, sizeof(i32), MAX_PROGRAM_SIZE, file);
    fclose(file);

    printf("Dumped RAM memory to 'RAM.DATA'\n");

}

