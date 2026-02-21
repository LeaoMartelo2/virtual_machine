#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "../spec.h"
#include "opcodes.h"

#define ARR_LEN(arr) (sizeof(arr) / sizeof((arr)[0]))

int main(void) {

    //   const char *program_path = "program.vmbin";

    VM vm = {};

    // hard code the program for now

    printf("program_counter = %d\n", vm.program_counter);

    
    i32 loaded_program[] = {MOV, -5, REG_1,
                           MOV, 0, REG_4, /* ensure reg4 is 0 for comparasions */
                           NO_OP,
                           LD, REG_1, REG_2,
                           NO_OP,
                           STO_PC, REG_0,
                           INC, REG_3,
                           CMP, REG_3, REG_4,
                           JE, REG_0,
                           NO_OP, 
                           DEC, REG_3,
                           NO_OP,
                           STO_PC, REG_9,
                           INC, REG_1,
                           CMP, REG_1, REG_4, 
                           JNE, REG_9,
                           NO_OP,
                           MOV, 22, REG_6,
                           MOV, 20, REG_7,
                           STO_PC, REG_9,
                           DEC, REG_6,
                           CMP, REG_6, REG_7,
                           JGE, REG_9,
                           NO_OP,
                           MOV, 18, REG_6,
                           MOV, 20, REG_7,
                           STO_PC, REG_9,
                           INC, REG_6,
                           CMP, REG_6, REG_7,
                           JLE, REG_9,
                           NO_OP,
                           STATE_DUMP,
                           PROGRAM_DUMP,
                           HALT};

    memcpy(vm.program, loaded_program, sizeof(loaded_program));

    vm.program_size = ARR_LEN(loaded_program);

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
