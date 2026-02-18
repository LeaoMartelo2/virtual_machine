#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "opcodes.h"
#include "spec.h"

#define ARR_LEN(arr) (sizeof(arr) / sizeof(arr)[0])

int main(void) {

    //   const char *program_path = "program.vmbin";

    VM vm = {};

    // hard code the program for now

    printf("program_counter = %d\n", vm.program_counter);

    
    u32 loded_program[] = {MOV, 30, REG_5,
                           NO_OP,
                           LD, REG_5, REG_7,
                           NO_OP,
                           INC, REG_5,
                           STO_PC, REG_0,
                           JMP, REG_0,
                           STATE_DUMP,
                           HALT};

    memcpy(vm.program, loded_program, sizeof(loded_program));

    vm.program_size = ARR_LEN(loded_program);

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

        case MOV: {
            mov(&vm);
        } break;

        case LD: {
            ld(&vm);
        } break;

        case INC: {
            inc(&vm);
        } break;

        case STO_PC: {
            sto_pc(&vm);
        } break;

        case JMP: {
            jmp(&vm);
        } break;

        default: {
            printf("default\n");
            vm.halted = true;
            break;
        }
        }
    }

    return 0;
}
