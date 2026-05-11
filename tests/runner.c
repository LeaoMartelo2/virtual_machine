#define TS_RUNNER_IMPLEMENTATION
#include "ts_runner.h"

String_View vmasm(const char* path) {
    String_Builder sb = {0};

    sb_appendf(&sb, "../vmasm %s -run", path);

    return sb_to_sv(sb);
}

String_View vm(const char *path) {
    String_Builder sb = {0}; 
    sb_appendf(&sb, "../vm %s", path);

    return sb_to_sv(sb);
}

int main(void) {

    TestCase *failed = NULL;

    printf("===== Testing Exceptions =====\n");

    TestCase *exceptions = NULL;

    String_Builder sb = {0};
    

    add_test(&exceptions,
            .name = sv("Exception illegal write"),
            .command = vmasm("exceptions/illegal_write.asm"),
            .expected = sv("EXCEPTION_ILLEGAL_WRITE"));

    add_test(&exceptions,
            .name = sv("Exception illegal read"),
            .command = vmasm("exceptions/illegal_read.asm"),
            .expected = sv("EXCEPTION_ILLEGAL_READ"));

    add_test(&exceptions,
            .name = sv("Exception unknown instruction"),
            .command = vm("exceptions/unknown_instruction.bin"),
            .expected = sv("EXCEPTION_UNKNOWN_INSTRUCTION"));

    add_test(&exceptions,
            .name = sv("Exception division by zero"),
            .command = vmasm("exceptions/division_by_zero.asm"),
            .expected = sv("EXCEPTION_DIVISION_BY_ZERO"));

    add_test(&exceptions,
            .name = sv("Exception jmp out of bounds"),
            .command = vmasm("exceptions/jmp_out_of_bounds.asm"),
            .expected = sv("EXCEPTION_JMP_OUT_OF_BOUNDS"));

    add_test(&exceptions,
            .name = sv("Exception invalid syscall"),
            .command = vmasm("exceptions/invalid_syscall.asm"),
            .expected = sv("EXCEPTION_INVALID_SYSCALL"));

    add_test(&exceptions,
            .name = sv("Exception dlopen fail"),
            .command = vmasm("exceptions/dlopen_fail.asm"),
            .expected = sv("EXCEPTION_DLOPEN_FAIL"));

    add_test(&exceptions,
            .name = sv("Exception external symbol resolution fail"),
            .command = vmasm("exceptions/extsym_resolution_fail.asm"),
            .expected = sv("EXCEPTION_EXTSYM_RESOLUTION_FAIL"));

    add_test(&exceptions,
            .name = sv("Exception too many extern symbols"),
            .command = vmasm("exceptions/too_many_extern_symbols.asm"),
            .expected = sv("EXCEPTION_TOO_MANY_EXTERN_SYMBOLS"));

    add_test(&exceptions,
            .name = sv("Exception open syscall fail"),
            .command = vmasm("exceptions/open_syscall_fail.asm"),
            .expected = sv("EXCEPTION_OPEN_SYSCALL_FAIL"));


    run_entire_suite(exceptions, .save_failed = &failed);


    return 0;
}
