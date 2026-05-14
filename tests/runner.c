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

    add_test(&exceptions,
            .name = sv("Exception stack overflow "),
            .command = vmasm("exceptions/stack_overflow.asm"),
            .expected = sv("EXCEPTION_STACK_OVERFLOW"));

    add_test(&exceptions,
            .name = sv("Exception stack underflow"),
            .command = vmasm("exceptions/stack_underflow.asm"),
            .expected = sv("EXCEPTION_STACK_UNDERFLOW"));

    add_test(&exceptions,
            .name = sv("Exception crash intentional"),
            .command = sv("../vmasm exceptions/crash_intentional.asm -run -crash"),
            .expected = sv("EXCEPTION_CRASH_INTENTIONAL"));

    run_entire_suite(exceptions, .save_failed = &failed);

/* @exceptions */

    printf("===== Testing OPCODES =====\n");

    TestCase *opcodes = NULL;

    add_test(&opcodes,
            .name = sv("MOV opcode"),
            .command = vmasm("opcodes/mov.asm"),
            .expected = sv("MOV: { 0 -> register[0] }"));

    add_test(&opcodes,
            .name = sv("LD opcode"),
            .command = vmasm("opcodes/ld.asm"),
            .expected = sv("LD: { register[5] = 5 -> register[10] }"));

    add_test(&opcodes,
            .name = sv("INC opcode"),
            .command = vmasm("opcodes/inc.asm"),
            .expected = sv("INC: { register[0] = 0 -> 1 }"));

    add_test(&opcodes,
            .name = sv("DEC opcode"),
            .command = vmasm("opcodes/dec.asm"),
            .expected = sv("DEC: { register[1] = 1 -> 0 }"));

    add_test(&opcodes,
            .name = sv("JE opcode"),
            .command = vmasm("opcodes/je.asm"),
            .expected = sv("1"));

    add_test(&opcodes,
            .name = sv("JNE opcode"),
            .command = vmasm("opcodes/jne.asm"),
            .expected = sv("1"));

    add_test(&opcodes,
            .name = sv("JGE opcode"),
            .command = vmasm("opcodes/jge.asm"),
            .expected = sv("1"));

    add_test(&opcodes,
            .name = sv("JLE opcode"),
            .command = vmasm("opcodes/jle.asm"),
            .expected = sv("1"));

    add_test(&opcodes,
            .name = sv("ADD opcode"),
            .command = vmasm("opcodes/add.asm"),
            .expected = sv("ADD: { reg[0] + reg[1] |  2 + 2 = 4 }"));

    add_test(&opcodes,
            .name = sv("SUB opcode"),
            .command = vmasm("opcodes/sub.asm"),
            .expected = sv("SUB: { reg[1] - reg[2] |  1 - 2 = -1 }"));

    add_test(&opcodes,
            .name = sv("MUL opcode"),
            .command = vmasm("opcodes/mul.asm"),
            .expected = sv("MUL: { reg[10] * reg[20] |  10 * 10 = 100 }"));

    add_test(&opcodes,
            .name = sv("DIV opcode"),
            .command = vmasm("opcodes/div.asm"),
            .expected = sv("DIV: { reg[10] / reg[20] |  100 / 10 = 10 }"));







    run_entire_suite(opcodes, .save_failed = &failed);

    return 0;
}
