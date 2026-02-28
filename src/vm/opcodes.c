#include "opcodes.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

void vm_verbose_(VM *vm, const char *format, ...) {

    if (!vm || !vm->verbose) {
        return;
    }

    va_list args;
    va_start(args, format);

    vprintf(format, args);

    va_end(args);
}

#define vm_verbose(fmt, ...) vm_verbose_(vm, (fmt), ##__VA_ARGS__)

void no_op(VM *vm) {
    vm_verbose("NO_OP\n");
    vm->program_counter++;
}

void halt(VM *vm) {
    printf("HALT:\n");
    printf("==== MACHINE HALTED ====\n");
    vm->halted = true;
}

void state_dump(VM *vm) {

    printf("STATE_DUMP: \n");
    printf("##############################\n");
    printf("REGISTERS:\n");
    for (i32 i = 0; i < NAMED_REGISTERS_SPLIT; ++i) {
        printf("        register[%2d] = %2d\n", i, vm->registers[i]);
    }
    printf("##############################\n");
    printf("MACHINE INFO:\n");
    printf("        program_couter  = %d\n", vm->program_counter);
    printf("        program_size    = %d\n", vm->program_size);
    printf("        stack_head      = %d\n", vm->stack_head);
    printf("        top of stack    = %d\n", vm->stack[vm->stack_head - 1]);
    printf("        condition_flag  = %d\n", vm->cond_flag);
    printf("        return address  = %d\n", vm->return_address_stack[vm->return_address_head - 1]);
    printf("##############################\n");
    vm->program_counter++;
}

void register_dump(VM *vm) {

    printf("REGISTER_DUMP: \n");
    printf("##############################\n");

    vm->program_counter++;
    i32 reg_start_ind = vm->program[vm->program_counter];

    vm->program_counter++;
    i32 reg_end_ind = vm->program[vm->program_counter];

    assert(reg_end_ind + 1 <= NAMED_REGISTERS_SPLIT);
    assert(reg_start_ind <= reg_end_ind);

    printf("Dumping registers [%d] .. [%d]:\n\n", reg_start_ind, reg_end_ind);

    for (i32 i = reg_start_ind; i < reg_end_ind + 1; ++i) {
        printf("        register[%2d] = %2d\n", i, vm->registers[i]);
    }
    printf("##############################\n");

    vm->program_counter++;
}

void program_dump(VM *vm) {
    printf("PROGRAM_DUMP:\n");

    const char *rawdump = "dumped-program.obj";

    FILE *file = fopen(rawdump, "wb");
    if (!file) {
        exit(1);
    }
    fwrite(vm->program, sizeof(i32), vm->program_size, file);
    fclose(file);

    printf("    Dumped loaded program bytecode to %s\n", rawdump);

    vm->program_counter++;
}

void toggle_verbose(VM *vm) {

    vm->program_counter++;
    i32 value = vm->program[vm->program_counter];

    if (value >= 1) {
        vm->verbose = true;
        vm_verbose("TOGGLE_VERBOSE: { VERBOSE MODE: ENABLED }\n");
    }

    if (value <= 0) {
        vm->verbose = false;
    }

    vm->program_counter++;
}

void mov(VM *vm) {

    vm_verbose("MOV: { ");
    vm->program_counter++;
    i32 tmp = vm->program[vm->program_counter];
    vm_verbose("%d -> ", vm->program[vm->program_counter]);
    vm->program_counter++;
    vm->registers[vm->program[vm->program_counter]] = tmp;
    vm_verbose("register[%d] }", vm->program[vm->program_counter]);
    vm->program_counter++;
    vm_verbose("\n");
}

void ld(VM *vm) {

    vm_verbose("LD: {");
    vm->program_counter++;
    i32 reg_num = vm->program[vm->program_counter];
    vm_verbose(" register[%d] ", reg_num);
    i32 value = vm->registers[reg_num];
    vm_verbose("= %d -> ", value);
    vm->program_counter++;
    i32 reg_target = vm->program[vm->program_counter];
    vm_verbose("register[%d] }", reg_target);
    vm->registers[reg_target] = value;
    vm->program_counter++;
    vm_verbose("\n");
}

void inc(VM *vm) {
    vm_verbose("INC: {");
    vm->program_counter++;
    i32 reg_ind = vm->program[vm->program_counter];
    vm_verbose(" register[%d] = %d", reg_ind, vm->registers[reg_ind]);
    vm->registers[reg_ind]++;
    vm_verbose(" -> %d }", vm->registers[reg_ind]);
    vm->program_counter++;
    vm_verbose("\n");
}

void dec(VM *vm) {
    vm_verbose("DEC: {");
    vm->program_counter++;
    i32 reg_ind = vm->program[vm->program_counter];
    vm_verbose(" register[%d] = %d", reg_ind, vm->registers[reg_ind]);
    vm->registers[reg_ind]--;
    vm_verbose(" -> %d }", vm->registers[reg_ind]);
    vm->program_counter++;
    vm_verbose("\n");
}

void sto_pc(VM *vm) {
    vm_verbose("STO_PC: {");
    i32 pc = vm->program_counter;
    vm->program_counter++;
    i32 reg_ind = vm->program[vm->program_counter];
    vm->registers[reg_ind] = pc + 2; /* pos of the next operation after sto_pc argument */
    vm_verbose(" register[%d] = %d }\n", reg_ind, pc + 2);
    vm->program_counter++;
}

void cmp(VM *vm) {
    vm_verbose("CMP: {");
    vm->program_counter++;

    i32 arg_a_reg_ind = vm->program[vm->program_counter];
    i32 arg_a_value = vm->registers[arg_a_reg_ind];

    vm->program_counter++;

    i32 arg_b_reg_ind = vm->program[vm->program_counter];
    i32 arg_b_value = vm->registers[arg_b_reg_ind];

    i32 res = arg_a_value - arg_b_value;

    if (res < 0) {
        vm->cond_flag = COND_NEGATIVE;
        vm_verbose(" register[%d] < register[%d]; cond: NEGATIVE }\n", arg_a_reg_ind, arg_b_reg_ind);
        vm->program_counter++;
        return;
    }

    if (res == 0) {
        vm->cond_flag = COND_ZERO;
        vm_verbose(" register[%d] == register[%d]; cond: ZERO }\n", arg_a_reg_ind, arg_b_reg_ind);
        vm->program_counter++;
        return;
    }

    if (res > 0) {
        vm->cond_flag = COND_POSITIVE;
        vm_verbose(" register[%d] > register[%d]; cond: POSITIVE }\n", arg_a_reg_ind, arg_b_reg_ind);
        vm->program_counter++;
        return;
    }

    // crash();
}

void jmp(VM *vm) {
    vm_verbose("JMP: {");
    vm->program_counter++;
    i32 jmp_to = vm->program[vm->program_counter];
    vm_verbose(" program_couter -> %.2d }\n", jmp_to);
    vm->program_counter = jmp_to;
}

void je(VM *vm) {
    vm_verbose("JE: {");
    vm->program_counter++;
    i32 jump_to = vm->program[vm->program_counter];

    if (vm->cond_flag == COND_ZERO) {
        vm_verbose(" cond: == 0; JUMPING }\n");
        vm->program_counter = jump_to;
        return;

    } else {
        vm_verbose(" cond != 0; NOT JUMPING }\n");
        vm->program_counter++;
        return;
    }
}

void jne(VM *vm) {
    vm_verbose("JNE: {");
    vm->program_counter++;
    i32 jump_to = vm->program[vm->program_counter];

    if (vm->cond_flag == COND_NEGATIVE || vm->cond_flag == COND_POSITIVE) {
        vm_verbose(" cond: != 0; JUMPING }\n");
        vm->program_counter = jump_to;
        return;

    } else {
        vm_verbose(" cond: == 0; NOT JUMPING }\n");
        vm->program_counter++;
        return;
    }
}

void jge(VM *vm) {
    vm_verbose("JGE: {");
    vm->program_counter++;
    i32 jump_to = vm->program[vm->program_counter];

    if (vm->cond_flag == COND_POSITIVE || vm->cond_flag == COND_ZERO) {
        vm_verbose(" cond >= 0; JUMPING }\n");
        vm->program_counter = jump_to;
        return;

    } else {
        vm_verbose("cond < 0; NOT JUMPING }\n");
        vm->program_counter++;
        return;
    }
}

void jle(VM *vm) {
    vm_verbose("JLE: {");
    vm->program_counter++;
    i32 jump_to = vm->program[vm->program_counter];

    if (vm->cond_flag == COND_NEGATIVE || vm->cond_flag == COND_ZERO) {
        vm_verbose(" cond <= 0; JUMPING }\n");
        vm->program_counter = jump_to;
        return;

    } else {
        vm_verbose(" cond > 0; NOT JUMPING }\n");
        vm->program_counter++;
        return;
    }
}

void add(VM *vm) {
    vm_verbose("ADD: {");

    vm->program_counter++;
    i32 reg_a_ind = vm->program[vm->program_counter];
    i32 reg_a = vm->registers[reg_a_ind];

    vm->program_counter++;
    i32 reg_b_ind = vm->program[vm->program_counter];
    i32 reg_b = vm->registers[reg_b_ind];

    vm_verbose(" reg[%d] + reg[%d] | ", reg_a_ind, reg_b_ind);

    i32 result = reg_a + reg_b;
    vm_verbose(" %d + %d = %d }\n", reg_a, reg_b, result);

    vm->registers[reg_a_ind] = result;

    vm->program_counter++;
}

void sub(VM *vm) {
    vm_verbose("SUB: {");

    vm->program_counter++;
    i32 reg_a_ind = vm->program[vm->program_counter];
    i32 reg_a = vm->registers[reg_a_ind];

    vm->program_counter++;
    i32 reg_b_ind = vm->program[vm->program_counter];
    i32 reg_b = vm->registers[reg_b_ind];

    vm_verbose(" reg[%d] - reg[%d] | ", reg_a_ind, reg_b_ind);

    i32 result = reg_a - reg_b;
    vm_verbose(" %d - %d = %d }\n", reg_a, reg_b, result);

    vm->registers[reg_a_ind] = result;

    vm->program_counter++;
}

void mul(VM *vm) {
    vm_verbose("MUL: {");

    vm->program_counter++;
    i32 reg_a_ind = vm->program[vm->program_counter];
    i32 reg_a = vm->registers[reg_a_ind];

    vm->program_counter++;
    i32 reg_b_ind = vm->program[vm->program_counter];
    i32 reg_b = vm->registers[reg_b_ind];

    vm_verbose(" reg[%d] * reg[%d] | ", reg_a_ind, reg_b_ind);

    i32 result = reg_a * reg_b;
    vm_verbose(" %d * %d = %d }\n", reg_a, reg_b, result);

    vm->registers[reg_a_ind] = result;

    vm->program_counter++;
}

void div_(VM *vm) {
    vm_verbose("DIV: {");

    vm->program_counter++;
    i32 reg_a_ind = vm->program[vm->program_counter];
    i32 reg_a = vm->registers[reg_a_ind];

    vm->program_counter++;
    i32 reg_b_ind = vm->program[vm->program_counter];
    i32 reg_b = vm->registers[reg_b_ind];

    vm_verbose(" reg[%d] / reg[%d] | ", reg_a_ind, reg_b_ind);

    i32 result = reg_a / reg_b;
    vm_verbose(" %d / %d = %d }\n", reg_a, reg_b, result);

    vm->registers[reg_a_ind] = result;

    vm->program_counter++;
}

void mod(VM *vm) {
    vm_verbose("MOD: {");

    vm->program_counter++;
    i32 reg_a_ind = vm->program[vm->program_counter];
    i32 reg_a = vm->registers[reg_a_ind];

    vm->program_counter++;
    i32 reg_b_ind = vm->program[vm->program_counter];
    i32 reg_b = vm->registers[reg_b_ind];

    vm_verbose(" reg[%d] %% reg[%d] | ", reg_a_ind, reg_b_ind);

    i32 result = reg_a % reg_b;
    vm_verbose(" %d %% %d = %d }\n", reg_a, reg_b, result);

    vm->registers[reg_a_ind] = result;

    vm->program_counter++;
}

void push(VM *vm) {
    vm_verbose("PUSH: {");

    vm->program_counter++;
    i32 reg_ind = vm->program[vm->program_counter];
    i32 reg_value = vm->registers[reg_ind];

    vm->stack[vm->stack_head] = reg_value;
    vm->stack_head++;

    vm_verbose(" stack[%d] = %d }\n", vm->stack_head, reg_value);

    vm->program_counter++;
}

void i_push(VM *vm) {

    vm_verbose("I_PUSH: {");

    vm->program_counter++;
    i32 value = vm->program[vm->program_counter];

    vm->stack[vm->stack_head] = value;
    vm->stack_head++;

    vm_verbose(" stack[%d] = %d }\n", vm->stack_head, value);

    vm->program_counter++;
}

void pop(VM *vm) {

    vm_verbose("POP: {");

    vm->program_counter++;

    i32 reg_ind = vm->program[vm->program_counter];

    vm->registers[reg_ind] = vm->stack[vm->stack_head - 1];
    vm_verbose(" reg[%d] = %d }\n", reg_ind, vm->stack[vm->stack_head - 1]);
    vm->stack_head--;

    vm->program_counter++;
}

void void_pop(VM *vm) {
    vm_verbose("VOID_POP: {");

    vm->stack_head--;
    vm_verbose(" vm->stack_head = %d }\n", vm->stack_head);

    vm->program_counter++;
}

void call(VM *vm) {
    vm_verbose("CALL: {");
    vm->program_counter++;

    vm->return_address_stack[vm->return_address_head] = vm->program_counter + 1;
    vm->return_address_head++;

    i32 jmp_to = vm->program[vm->program_counter];
    vm_verbose(" program_couter -> %.2d }\n", jmp_to);
    vm->program_counter = jmp_to;
}

void ret(VM *vm) {
    vm_verbose("RET: {");

    vm_verbose(" program_couter = %d }\n", vm->return_address_stack[vm->return_address_head - 1]);
    vm->program_counter = vm->return_address_stack[vm->return_address_head - 1];

    vm->return_address_head--;
}
