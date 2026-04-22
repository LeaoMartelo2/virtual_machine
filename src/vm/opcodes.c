#define _POSIX_C_SOURCE 200809L

#include "opcodes.h"
#include "error.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/limits.h>
#include <signal.h>
#include <fcntl.h>

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


struct vm_ptr_info_t {
    bool RAM_addr;
    bool ROM_addr;
    i32 addr;
} vm_ptr_info;

static i32 *get_vm_ptr(VM *vm, i32 addr) {
    
    vm_ptr_info = (struct vm_ptr_info_t){0};

    if (addr < 0) return NULL;

    if (addr < MAX_PROGRAM_SIZE) {
        vm_ptr_info.ROM_addr = true;
        vm_ptr_info.addr = addr;
        return &vm->program[addr];
    }

    i32 ram_idx = addr - MAX_PROGRAM_SIZE;
    if(ram_idx < MAX_PROGRAM_SIZE) {
        vm_ptr_info.RAM_addr = true;
        vm_ptr_info.addr = ram_idx;
        return &vm->memory[ram_idx];
    }

    return NULL;

}

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
    printf("        program_counter  = %d\n", vm->program_counter);
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

   // assert(reg_end_ind + 1 <= NAMED_REGISTERS_SPLIT);
    //assert(reg_start_ind <= reg_end_ind);

    printf("Dumping registers [%d] .. [%d]:\n\n", reg_start_ind, reg_end_ind);

    for (i32 i = reg_start_ind; i < reg_end_ind + 1; ++i) {
        printf("        register[%2d] = %2d\n", i, vm->registers[i]);
    }
    printf("##############################\n");

    vm->program_counter++;
}

void program_dump(VM *vm) {
    printf("PROGRAM_DUMP:\n");

    const char *rawdump = "dumped-program.bin";

    FILE *file = fopen(rawdump, "wb");
    if (!file) {
        exit(1);
    }
    fwrite(vm->program, sizeof(i32), vm->program_size, file);
    fclose(file);

    printf("    Dumped loaded program bytecode to %s\n", rawdump);

    vm->program_counter++;
}

void stack_dump(VM *vm) {
    printf("STACK_DUMP:\n");
    printf("##############################\n");

    i32 start = vm->stack_head - 2;

    if (start < 0) start = 0;

    i32 end = start + 5;

    if (end > MAX_STACK_SIZE) {
        end = MAX_STACK_SIZE;

        start = end - 5;

        if (start < 0) start = 0;
    }

    for (int i = start; i < end; ++i) {
        printf("%d", vm->stack[i]);

        if (i == vm->stack_head) {
            printf("    <- stack_head");
        }
        printf("\n");

    }

    printf("##############################\n");

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

    vm_crash(vm, EXCEPTION_ILLEGAL_STATE, 
            .description = vm_text_format("Unable to determine CMP condition output, VALUE %d",
                res),
            .detailed_description = "Value is nether Greater than, less than or equal 0, Nan?",
            .dump_vm_struct = true);
}

void jmp(VM *vm) {
    vm_verbose("JMP: {");
    vm->program_counter++;
    i32 jmp_to = vm->program[vm->program_counter];
    vm_verbose(" program_counter -> %.2d }\n", jmp_to);
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
    vm_verbose(" stack[%d] = %d }\n", vm->stack_head, reg_value);
    vm->stack_head++;


    vm->program_counter++;
}

void i_push(VM *vm) {

    vm_verbose("I_PUSH: {");

    vm->program_counter++;
    i32 value = vm->program[vm->program_counter];

    vm->stack[vm->stack_head] = value;
    vm_verbose(" stack[%d] = %d }\n", vm->stack_head, value);
    vm->stack_head++;


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
    vm_verbose(" program_counter -> %.2d }\n", jmp_to);
    vm->program_counter = jmp_to;
}

void ret(VM *vm) {
    vm_verbose("RET: {");

    vm_verbose(" program_counter = %d }\n", vm->return_address_stack[vm->return_address_head - 1]);
    vm->program_counter = vm->return_address_stack[vm->return_address_head - 1];

    vm->return_address_head--;
}

void syscall_(VM *vm) {
    vm_verbose("SYSCALL: {");
    vm->program_counter++;

    i32 syscall_num = vm->registers[REG_ARG_A];

    switch(syscall_num) {

        case WRITE_SYSCALL: { /* write(fd, buff_addr, count) */
            i32 fd = vm->registers[REG_ARG_B];
            i32 buff_addr = vm->registers[REG_ARG_C];
            i32 count = vm->registers[REG_ARG_D];
            
            vm_verbose(" write(%d, 0x%x, %d)", fd, buff_addr, count);

            for(i32 i = 0; i < count; ++i) {
                i32 *ptr = get_vm_ptr(vm, buff_addr + i);
                if(ptr) {
                    char c = (char)(*ptr & 0xFF);
                    write(fd, &c, 1);
                }
            }
            fsync(fd);

            vm_verbose(" }\n");
        } break;

        case GETPID_SYSCALL: { /* arg_b = getpid() */
            pid_t pid = getpid();
            i32 self_pid = (i32)pid;

            vm_verbose(" getpid() = %d -> ARG_B }\n", self_pid);

            vm->registers[REG_ARG_B] = self_pid;
        } break;

        case KILL_SYSCALL: { /* kill(arg_b, arg_c) */
            i32 pid = vm->registers[REG_ARG_B];
            i32 signal = vm->registers[REG_ARG_C];

            kill((pid_t)pid, signal);

            vm_verbose(" Killed %d (%d) }\n", pid, signal);
        } break;

        case OPEN_SYSCALL: { /* arg_a = open(arg_b, arg_c, arg_d) */
	    char path_buffer[PATH_MAX];
	    i32 buff_addr = vm->registers[REG_ARG_B];
	    i32 flags = vm->registers[REG_ARG_C];
	    i32 mode = vm->registers[REG_ARG_D];

	    i32 len = 0;
	    while(buff_addr + len < vm->program_size &&
		    vm->program[buff_addr + len] != 0 &&
		    len < (PATH_MAX -1)) {

		path_buffer[len] = (char)(vm->program[buff_addr + len] & 0xFF);
		len++;
	    }
	    /* dont trust the null terminator in the program */
	    path_buffer[len] = '\0'; 

	    /*
	    for(i32 i = 0; i< len; ++i) {
		printf("%c\n", path_buffer[i]);
	    }
	    printf("\n");
	    */

	    int fd = open(path_buffer, flags, mode);
            if(fd == 1) {perror("file failed to open"); exit(1);}
	    vm->registers[REG_ARG_A] = (i32)fd;

	    vm_verbose(" open(\"%s\", %d, %d) -> fd: %d }\n", path_buffer, flags, mode, fd);

        } break;

        case CLOSE_SYSCAL: {
            i32 fd = vm->registers[REG_ARG_B];
            
            close(fd);

            vm_verbose(" close(%d) }\n", fd);

        } break;

        case READ_SYSCALL: {
            i32 fd = vm->registers[REG_ARG_B];
            i32 buff_addr = vm->registers[REG_ARG_C];
            i32 count = vm->registers[REG_ARG_D];

            i32 bytes_read_total = 0;

            while(bytes_read_total < 255) {
                char c;
                ssize_t n = read(fd, &c, 1);

                if (n <= 0) break; /* error or EOF */
                
                if(fd == 0 && c == '\n') break;

                vm->registers[REG_ARG_A] = (i32)n;


                i32 *ptr = get_vm_ptr(vm, buff_addr + bytes_read_total);

                if(ptr) {
                    *ptr = (i32)c;
                    bytes_read_total++;
                }
            }

           
            i32 *null_terminator_ptr = get_vm_ptr(vm, buff_addr + bytes_read_total);
            if(null_terminator_ptr) *null_terminator_ptr = 0;
            
            vm->registers[REG_HEAP_PTR] += (bytes_read_total + 1);
            vm->registers[REG_RET] = bytes_read_total;
            vm_verbose(" read(%d, %d, %d) -> read %d bytes }\n", fd, buff_addr, count, bytes_read_total);

        } break;

    }
}

void strlen_(VM *vm) {
    vm_verbose("STRLEN: {");
    vm->program_counter++;

    i32 buff_addr = vm->program[vm->program_counter];
    vm_verbose(" buff_addr=%d", buff_addr);
    vm->program_counter++;

    i32 dest_reg = vm->program[vm->program_counter];
    vm_verbose(" -> $%d", dest_reg);

    i32 len = 0;
    while(true) {
        i32 *ptr = get_vm_ptr(vm, buff_addr + len);

        if(!ptr || *ptr == 0) break;
        len++;
    }

    vm->registers[dest_reg] = len;
    vm_verbose(" (length=%d) }\n", len);
    vm->program_counter++;
}

void strlen_r(VM *vm) {
    vm_verbose("STRLEN_R: {");
    vm->program_counter++;

    i32 reg_ind = vm->program[vm->program_counter];
    i32 reg_val = vm->registers[reg_ind];
    
    i32 buff_addr = 0;
    buff_addr = reg_val;
    vm_verbose(" buff_addr=%d", buff_addr);
    vm->program_counter++;

    i32 dest_reg = vm->program[vm->program_counter];
    vm_verbose(" -> $%d", dest_reg);

    i32 len = 0;
    while(true) {
        i32 *ptr = get_vm_ptr(vm, buff_addr + len);

        if(!ptr || *ptr == 0) break;
        len++;
    }


    vm->registers[dest_reg] = len;
    vm_verbose(" (length=%d) }\n", len);
    vm->program_counter++;
}

void print_char(VM *vm) {
    vm_verbose("PRINT_CHAR: {");
    vm->program_counter++;

    i32 reg = vm->program[vm->program_counter];
    i32 value = vm->registers[reg];

    printf("%c", (char)value);
    vm_verbose(" $%d='%c' } \n", reg, (char)value);

    vm->program_counter++;
}

void print_int(VM *vm) {
    vm_verbose("PRINT_INT: {");
    vm->program_counter++;

    i32 reg = vm->program[vm->program_counter];
    i32 value = vm->registers[reg];

    printf("%d", value);
    fflush(stdout);

    vm_verbose(" $%d=='%d' } \n", reg, value);

    vm->program_counter++;
}

void iprint_char(VM *vm) {
    vm_verbose("IPRINT_CHAR: {");
    vm->program_counter++;

    i32 value = vm->program[vm->program_counter];

    printf("%c",(char)value);
    fflush(stdout);

    vm_verbose(" %d=='%c' }\n", value, (char)value);

    vm->program_counter++;
}

void iprint_int(VM *vm) {
    vm_verbose("IPRINT_INT: {");
    vm->program_counter++;

    i32 value = vm->program[vm->program_counter];

    printf("%d",value);

    vm_verbose(" program[%d] == %d }\n", vm->program_counter, value);

    vm->program_counter++;
}

void line_br(VM *vm) {
    putchar('\n');
    vm->program_counter++;
}

void ldo(VM *vm) {
    vm_verbose("LDO: {");
    vm->program_counter++;

    i32 *data_addr = get_vm_ptr(vm, vm->program[vm->program_counter]);

    if(vm_ptr_info.ROM_addr) {
        vm_verbose(" @addr=%d(ROM)", vm_ptr_info.addr);
    }

    if(vm_ptr_info.RAM_addr) {
        vm_verbose(" @addr=%d(RAM)", vm_ptr_info.addr);
    }

    vm->program_counter++;
    i32 dest_reg = vm->program[vm->program_counter];
    vm_verbose(" -> $%d", dest_reg);


    //i32 value = vm->program[data_addr];

    i32 value = *data_addr;

    vm->registers[dest_reg] = value;
    vm_verbose(" (value=%d) }\n", value);

    vm->program_counter++;
}

void ldxo(VM *vm) {
    vm_verbose("LDXO: {");
    vm->program_counter++;

    i32 base_reg_idx = vm->program[vm->program_counter];
    i32 base_addr = vm->registers[base_reg_idx];

    //i32 *base_addr_value = get_vm_ptr(vm, base_addr);

    vm->program_counter++;
    i32 index_reg_idx = vm->program[vm->program_counter];
    i32 index_offset = vm->registers[index_reg_idx];

    vm->program_counter++;
    i32 dest_reg = vm->program[vm->program_counter];


    i32 *final_addr = get_vm_ptr(vm, base_addr + index_offset);

    if(vm_ptr_info.ROM_addr) {
        vm_verbose(" base$%d(%d)(ROM)", base_reg_idx, base_addr);
    }

    if(vm_ptr_info.RAM_addr) {
        vm_verbose(" base$%d(%d)(RAM)", base_reg_idx, base_addr);
    }


    vm_verbose(" + $%d(offset=%d)", index_reg_idx, index_offset);
    vm_verbose(" -> $%d", dest_reg);

    
    i32 value = *final_addr;
    vm->registers[dest_reg] = value;
    vm_verbose(" (value=%d) }\n", value);

    vm->program_counter++;
}

void rdint(VM *vm) {
    vm_verbose("RDINT: {");
    vm->program_counter++;

    i32 reg_idx = vm->program[vm->program_counter];

    i32 value = 0;

    scanf("%d", &value);

    vm->registers[reg_idx] = value;

    vm_verbose(" }\n");

    vm->program_counter++;
}

void and_(VM *vm) {
    vm_verbose("AND: {");
    vm->program_counter++;

    i32 reg_a_idx = vm->program[vm->program_counter];
    i32 reg_a_val = vm->registers[reg_a_idx];

    vm->program_counter++;

    i32 reg_b_idx = vm->program[vm->program_counter];
    i32 reg_b_val = vm->registers[reg_b_idx];

    i32 result = reg_a_val & reg_b_val;

    vm_verbose(" %d & %d = %d", reg_a_val, reg_b_val, result);

    vm->registers[reg_a_idx] = result;

    vm_verbose(" }\n");

    vm->program_counter++;
}

void or_(VM *vm) {
    vm_verbose("OR: {");
    vm->program_counter++;

    i32 reg_a_idx = vm->program[vm->program_counter];
    i32 reg_a_val = vm->registers[reg_a_idx];

    vm->program_counter++;

    i32 reg_b_idx = vm->program[vm->program_counter];
    i32 reg_b_val = vm->registers[reg_b_idx];

    i32 result = reg_a_val | reg_b_val;

    vm_verbose(" %d | %d = %d", reg_a_val, reg_b_val, result);

    vm->registers[reg_a_idx] = result;

    vm_verbose(" }\n");

    vm->program_counter++;
}

void xor_(VM *vm) {
    vm_verbose("XOR: {");
    vm->program_counter++;

    i32 reg_a_idx = vm->program[vm->program_counter];
    i32 reg_a_val = vm->registers[reg_a_idx];

    vm->program_counter++;

    i32 reg_b_idx = vm->program[vm->program_counter];
    i32 reg_b_val = vm->registers[reg_b_idx];

    i32 result = reg_a_val ^ reg_b_val;

    vm_verbose(" %d ^ %d = %d", reg_a_val, reg_b_val, result);

    vm->registers[reg_a_idx] = result;

    vm_verbose(" }\n");

    vm->program_counter++;
}

void not_(VM *vm) {
    vm_verbose("NOT: {");
    vm->program_counter++;

    i32 reg_a_idx = vm->program[vm->program_counter];
    i32 reg_a_val = vm->registers[reg_a_idx];

    i32 result = ~reg_a_val;

    vm->registers[reg_a_idx] = result;

    vm_verbose(" ~%d = %d }\n", reg_a_val, result);

    vm->program_counter++;
}

void lsh(VM *vm) {
    vm_verbose("LSH: {");
    vm->program_counter++;

    i32 reg_a_idx = vm->program[vm->program_counter];
    i32 reg_a_val = vm->registers[reg_a_idx];

    i32 result = reg_a_val << 1;

    vm->registers[reg_a_idx] = result;

    vm_verbose(" %d << 1 = %d }\n", reg_a_val, result);

    vm->program_counter++;
}

void rsh(VM *vm) {
    vm_verbose("RSH: {");
    vm->program_counter++;

    i32 reg_a_idx = vm->program[vm->program_counter];
    i32 reg_a_val = vm->registers[reg_a_idx];

    i32 result = reg_a_val >> 1;

    vm->registers[reg_a_idx] = result;

    vm_verbose(" %d >> 1 = %d }\n", reg_a_val, result);

    vm->program_counter++;
}

void lsha(VM *vm) {
    vm_verbose("LSHA: {");
    vm->program_counter++;

    i32 reg_a_idx = vm->program[vm->program_counter];
    i32 reg_a_val = vm->registers[reg_a_idx];

    vm->program_counter++;

    i32 reg_b_idx = vm->program[vm->program_counter];
    i32 reg_b_val = vm->registers[reg_b_idx];

    i32 result = reg_a_val << reg_b_val;

    vm->registers[reg_a_idx] = result;

    vm_verbose(" %d << $d = %d }\n", reg_a_val, reg_b_val, result);

    vm->program_counter++;
}

void rsha(VM *vm) {
    vm_verbose("RSHA: {");
    vm->program_counter++;

    i32 reg_a_idx = vm->program[vm->program_counter];
    i32 reg_a_val = vm->registers[reg_a_idx];

    vm->program_counter++;

    i32 reg_b_idx = vm->program[vm->program_counter];
    i32 reg_b_val = vm->registers[reg_b_idx];

    i32 result = reg_a_val >> reg_b_val;

    vm->registers[reg_a_idx] = result;

    vm_verbose(" %d >> %d = %d }\n", reg_a_val, reg_b_val, result);

    vm->program_counter++;
}

void str(VM *vm) {
    vm_verbose("STR: {");
    vm->program_counter++;

    i32 reg_a_idx = vm->program[vm->program_counter];
    i32 reg_a_val = vm->registers[reg_a_idx];

    vm->program_counter++;

    i32 reg_b_idx = vm->program[vm->program_counter];
    i32 reg_b_val = vm->registers[reg_b_idx];


    i32 *addr = get_vm_ptr(vm, reg_b_val);

    if(vm_ptr_info.ROM_addr) {
        vm_crash(vm,
                EXCEPTION_ILLEGAL_WRITE,
                .description = vm_text_format("Attempted to write '%d' to (ROM)'%d'",
                    reg_a_val, vm_ptr_info.addr),
                .detailed_description = "Writting to Read Only Memory is not allowed.",
                .dump_vm_struct = true);
    }

    if(vm_ptr_info.RAM_addr) {
        *addr = reg_a_val;
        vm_verbose(" @addr=%d(RAM) -> %d }\n", vm_ptr_info.addr, reg_a_val);
    }

    vm->program_counter++;
}
