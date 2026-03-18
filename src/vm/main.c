#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../spec.h"
#include "opcodes.h"

#define ARR_LEN(arr) (sizeof(arr) / sizeof((arr)[0]))

int main(int argc, char **argv) {

    if (argc < 2) {
        printf("Usage: %s <program.obj>\n", argv[0]);
        exit(1);
    }

    const char *program_file_path = argv[1];

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
        fprintf(stderr, "ROM version might be incompatible with interpreter version (Expected %d, got %d)\n", VM_VERSION, header.version);
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
    vm.verbose = false;
    vm.stack_head = 0;
    memset(vm.stack, (i32)0, sizeof(vm.stack));
    memset(vm.registers, (i32)0, sizeof(vm.registers));
    memset(vm.return_address_stack, (i32)0, sizeof(vm.return_address_stack));
    vm.return_address_head = 0;

    printf("==== VM INIT ===\n");
    
    while (!vm.halted) {

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
        
        case OPCODE_COUNT: 
        default: {
            printf("BAD OPCODE, HALTING\n");
            vm.halted = true;
            break;
        }
        }
    }

    return 0;
}
