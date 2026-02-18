#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "spec.h"
#include "opcodes.h"


int main(void) {

    //   const char *program_path = "program.vmbin";

    VM vm = {0};

    // hard code the program for now

    printf("program loaded: \n"
           "    mov 30, %%5 \n"
           "    NO_OP\n"
           "    STATE_DUMP\n"
           "    NO_OP\n"
           "    ld %%5, %%7\n"
           "    NO_OP\n"
           "    STATE_DUMP\n"
           "    HALT\n"
           "\n");

    printf("program_counter = %d\n", vm.program_counter);

    /* move 30 in to register 5 */
    /* r5 = 30; */
    vm.program[0] = MOV;
    vm.program[1] = 30;
    vm.program[2] = 5;

    vm.program[3] = NO_OP;

    vm.program[4] = STATE_DUMP;

    vm.program[5] = NO_OP;
    /* load the value of register 5 in to register 7 */
    /* r7 = r5; */
    vm.program[6] = LD;
    vm.program[7] = 5;
    vm.program[8] = 7;

    vm.program[9] = NO_OP;

    vm.program[10] = STATE_DUMP;
    vm.program[11] = HALT;

    vm.program_size = 12;

    for (u32 i = 0; i < vm.program_size; ++i) {

        printf("program[%d] = %d\n", i, vm.program[i]);
    }

    printf("==== VM INIT ===\n");

    vm.halted = false;

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
        }
        break;

        case LD: {
            ld(&vm);
        }
        break;


        default: {
            printf("default\n");
            vm.halted = true;
            break;
        }
        }
    }

    return 0;
}
