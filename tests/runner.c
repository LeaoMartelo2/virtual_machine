#define TS_RUNNER_IMPLEMENTATION
#include "ts_runner.h"

int main(void) {

    TestCase *failed = NULL;

    printf("===== Testing Exceptions =====\n");

    TestCase *exceptions = NULL;

    add_test(&exceptions,
            .name = sv("Exception illegal write"),
            .command = sv("../vmasm exceptions/illegal_write.asm -run"),
            .expected = sv("caught a EXCEPTION_ILLEGAL_WRITE"));

    add_test(&exceptions,
            .name = sv("Exception illegal read"),
            .command = sv("../vmasm exceptions/illegal_read.asm -run"),
            .expected = sv("caught a EXCEPTION_ILLEGAL_READ"));

    add_test(&exceptions,
            .name = sv("Exception unknown instruction"),
            .command = sv("../vm exceptions/unknown_instruction.bin"),
            .expected = sv("caught a EXCEPTION_UNKNOWN_INSTRUCTION"));

    add_test(&exceptions,
            .name = sv("Exception division by zero"),
            .command = sv("../vmasm exceptions/division_by_zero.asm -run"),
            .expected = sv("caught a EXCEPTION_DIVISION_BY_ZERO"));

    add_test(&exceptions,
            .name = sv("Exception jmp out of bounds"),
            .command = sv("../vm exceptions/jmp_out_of_bounds.bin"),
            .expected = sv("caught a EXCEPTION_JMP_OUT_OF_BOUNDS"));

    run_entire_suite(exceptions, .save_failed = &failed);



    return 0;
}
