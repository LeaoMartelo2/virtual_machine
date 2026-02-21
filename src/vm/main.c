#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
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
    if(!file) {
        perror("Error oppening file.");
        exit(1);
    }
    
    i32 loaded_program[MAX_PROGRAM_SIZE];

    size_t loaded_program_size = fread(loaded_program, sizeof(i32), MAX_PROGRAM_SIZE, file);
    fclose(file);

    VM vm = {};
    
    memcpy(vm.program, loaded_program, sizeof(loaded_program));

    vm.program_size = (i32)loaded_program_size;

    printf("==== VM INIT ===\n");

    vm.halted = false;
    vm.verbose = true;

    while (!vm.halted) {

        switch (vm.program[vm.program_counter]) {

        case NO_OP: {
            no_op(&vm);
        } break;

        case HALT: {
            halt(&vm);
        } break;

        case STATE_DUMP: {
            state_dump(&vm);
        } break;

        case PROGRAM_DUMP: {
            program_dump(&vm);
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

        default: {
            printf("BAD OPCODE, HALTING\n");
            vm.halted = true;
            break;
        }
        }
    }

    return 0;
}
