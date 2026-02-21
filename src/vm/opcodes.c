#include "opcodes.h"

#include <inttypes.h>
#include <stdio.h>

void no_op(VM *vm) {
    printf("NO_OP\n");
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
    for (i32 i = 0; i < REG_COUNT; ++i) {
        printf("        register[%2d] = %2d\n", i, vm->registers[i]);
    }
    printf("##############################\n");
    printf("MACHINE INFO:\n");
    printf("        program_couter  = %d\n", vm->program_counter);
    printf("        program_size    = %d\n", vm->program_size);
    printf("        condition_flag  = %d\n", vm->cond_flag);
    printf("##############################\n");
    vm->program_counter++;
}

void program_dump(VM *vm) {
    printf("PROGRAM_DUMP:\n");

    const char *rawdump = "dumped-program.obj";

    FILE *file = fopen(rawdump, "wb");
    fwrite(vm->program, sizeof(i32), vm->program_size, file);
    fclose(file);

    printf("    Dumped loaded program bytecode to %s\n", rawdump);

    vm->program_counter++;
}

void mov(VM *vm) {

    printf("MOV: { ");
    vm->program_counter++;
    i32 tmp = vm->program[vm->program_counter];
    printf("%d -> ", vm->program[vm->program_counter]);
    vm->program_counter++;
    vm->registers[vm->program[vm->program_counter]] = tmp;
    printf("register[%d] }", vm->program[vm->program_counter]);
    vm->program_counter++;
    printf("\n");
}

void ld(VM *vm) {

    printf("LD: {");
    vm->program_counter++;
    i32 reg_num = vm->program[vm->program_counter];
    printf(" register[%d] ", reg_num);
    i32 value = vm->registers[reg_num];
    printf("= %d -> ", value);
    vm->program_counter++;
    i32 reg_target = vm->program[vm->program_counter];
    printf("register[%d] }", reg_target);
    vm->registers[reg_target] = value;
    vm->program_counter++;
    printf("\n");
}

void inc(VM *vm) {
    printf("INC: {");
    vm->program_counter++;
    i32 reg_ind = vm->program[vm->program_counter];
    printf(" register[%d] = %d", reg_ind, vm->registers[reg_ind]);
    vm->registers[reg_ind]++;
    printf(" -> %d }", vm->registers[reg_ind]);
    vm->program_counter++;
    printf("\n");
}

void dec(VM *vm) {
    printf("DEC: {");
    vm->program_counter++;
    i32 reg_ind = vm->program[vm->program_counter];
    printf(" register[%d] = %d", reg_ind, vm->registers[reg_ind]);
    vm->registers[reg_ind]--;
    printf(" -> %d }", vm->registers[reg_ind]);
    vm->program_counter++;
    printf("\n");
}

void sto_pc(VM *vm) {
    printf("STO_PC: {");
    i32 pc = vm->program_counter;
    vm->program_counter++;
    i32 reg_ind = vm->program[vm->program_counter];
    vm->registers[reg_ind] = pc + 2; /* pos of the next operation after sto_pc argument */
    printf(" register[%d] = %d }\n", reg_ind, pc + 2);
    vm->program_counter++;
}

void cmp(VM *vm) {
    printf("CMP: {");
    vm->program_counter++;

    i32 arg_a_reg_ind = vm->program[vm->program_counter];
    i32 arg_a_value = vm->registers[arg_a_reg_ind];

    vm->program_counter++;

    i32 arg_b_reg_ind = vm->program[vm->program_counter];
    i32 arg_b_value = vm->registers[arg_b_reg_ind];

    i32 res = arg_a_value - arg_b_value;

    if (res < 0) {
        vm->cond_flag = COND_NEGATIVE;
        printf(" register[%d] < register[%d]; cond: NEGATIVE }\n", arg_a_reg_ind, arg_b_reg_ind);
        vm->program_counter++;
        return;
    }

    if (res == 0) {
        vm->cond_flag = COND_ZERO;
        printf(" register[%d] == register[%d]; cond: ZERO }\n", arg_a_reg_ind, arg_b_reg_ind);
        vm->program_counter++;
        return;
    }

    if (res > 0) {
        vm->cond_flag = COND_POSITIVE;
        printf(" register[%d] > register[%d]; cond: POSITIVE }\n", arg_a_reg_ind, arg_b_reg_ind);
        vm->program_counter++;
        return;
    }

    // crash();
}

void jmp(VM *vm) {
    printf("JMP: {");
    vm->program_counter++;
    i32 reg_ind = vm->program[vm->program_counter];
    i32 jmp_to = vm->registers[reg_ind];
    printf(" program_couter -> %.2d }\n", jmp_to);
    vm->program_counter = jmp_to;
}

void je(VM *vm) {
    printf("JE: {");
    vm->program_counter++;
    i32 reg_ind = vm->program[vm->program_counter];
    i32 jump_to = vm->registers[reg_ind];

    if (vm->cond_flag == COND_ZERO) {
        printf(" cond: == 0; JUMPING }\n");
        vm->program_counter = jump_to;
        return;

    } else {
        printf(" cond != 0; NOT JUMPING }\n");
        vm->program_counter++;
        return;
    }
}

void jne(VM *vm) {
    printf("JNE: {");
    vm->program_counter++;
    i32 reg_ind = vm->program[vm->program_counter];
    i32 jump_to = vm->registers[reg_ind];

    if (vm->cond_flag == COND_NEGATIVE || vm->cond_flag == COND_POSITIVE) {
        printf(" cond: != 0; JUMPING }\n");
        vm->program_counter = jump_to;
        return;

    } else {
        printf(" cond: == 0; NOT JUMPING }\n");
        vm->program_counter++;
        return;
    }
}

void jge(VM *vm) {
    printf("JGE: {");
    vm->program_counter++;
    i32 reg_ind = vm->program[vm->program_counter];
    i32 jump_to = vm->registers[reg_ind];

    if (vm->cond_flag == COND_POSITIVE || vm->cond_flag == COND_ZERO) {
        printf(" cond >= 0; JUMPING }\n");
        vm->program_counter = jump_to;
        return;

    } else {
        printf("cond < 0; NOT JUMPING }\n");
        vm->program_counter++;
        return;
    }
}

void jle(VM *vm) {
    printf("JLE: {");
    vm->program_counter++;
    i32 reg_ind = vm->program[vm->program_counter];
    i32 jump_to = vm->registers[reg_ind];

    if (vm->cond_flag == COND_NEGATIVE || vm->cond_flag == COND_ZERO) {
        printf(" cond <= 0; JUMPING }\n");
        vm->program_counter = jump_to;
        return;

    } else {
        printf(" cond > 0; NOT JUMPING }\n");
        vm->program_counter++;
        return;
    }
}
