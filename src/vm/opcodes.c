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
#include <dlfcn.h>

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

static bool is_readonly_register(i32 reg_idx) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"

    switch(reg_idx) {
	case NAMED_REGISTERS_SPLIT:
        case REG_RAM_START:
        case REG_HEAP_PTR:
        case REG_NULL:
        case REG_COUNT: {
            return true;
        } break;
        default: 
            return false;
        break;
    }

#pragma GCC diagnostic pop
}

static char *vm_get_string(VM *vm, i32 buff_addr, i32 length) {

    if(length < 0 || length > MAX_PROGRAM_SIZE) {
        vm_crash(vm, EXCEPTION_ILLEGAL_STATE,
                .description = "Invalid string length");
    }

    char *string = malloc(length + 1);
    if (!string) return NULL;

    for (i32 i = 0; i < length; ++i) {
        i32 *ptr = get_vm_ptr(vm, buff_addr + i);
        if(ptr) {
            char c = (char)(*ptr & 0xFF);
            string[i] = c;
            if(c == '\0') break;
        } else {
            string[i] = '\0';
            break;
        }
    }
    string[length] = '\0';
    return string;
}

static char *vm_get_cstring(VM *vm, i32 buff_addr) {
    i32 len = 0;
    while (len  < MAX_PROGRAM_SIZE) {
        i32 *ptr = get_vm_ptr(vm, buff_addr + len);
        if(!ptr || (char)(*ptr & 0xFF) == '\0') break;
        len++;
    }

    return vm_get_string(vm, buff_addr, len);
}


static void vm_internal_setstring_all_libraries(VM *vm, const char *arg) {

    for(i32 i = 0; i < vm->extern_handle_count; ++i) {
        void *this_handle = vm->extern_handle[i].handle;

        if(!this_handle) continue;

        _internal_setstring func = (_internal_setstring)dlsym(this_handle, "__vmasm_internal_setstring");

        if(func) {
            i32 ret = func(arg);
            vm->registers[REG_RET] = ret;
        }

    }

}

static void vm_set_register(VM *vm, i32 ind, i32 value) {

    if(is_readonly_register(ind)) {
        vm_crash(vm, EXCEPTION_ILLEGAL_WRITE,
                .description = "Caugth read only register write on external library",
                .detailed_description = "Writing to read only registers is not allowed");
    }

    vm->registers[ind] = value;
}

static i32 vm_get_register(VM *vm, i32 ind) {
    return vm->registers[ind];
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
    i32 reg_idx = vm->program[vm->program_counter];

    if(is_readonly_register(reg_idx)) {
        vm_crash(vm, EXCEPTION_ILLEGAL_WRITE,
            .description = vm_text_format("Caugth attempt of write to Read-Only register %d", reg_idx),
            .detailed_description = "Writing to Read-Only registers is not allowed");
    }

    vm->registers[reg_idx] = tmp;
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


    if(is_readonly_register(reg_target)) {
        vm_crash(vm, EXCEPTION_ILLEGAL_WRITE,
            .description = vm_text_format("Caugth attempt of write to Read-Only register %d", reg_target),
            .detailed_description = "Writing to Read-Only registers is not allowed");
    }


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
            .detailed_description = "Value is neither Greater than, less than or equal 0, NaN?",
            .dump_vm_struct = true);
}

void jmp(VM *vm) {
    vm_verbose("JMP: {");
    vm->program_counter++;

    i32 jmp_to = vm->program[vm->program_counter];

    if(jmp_to > MAX_PROGRAM_SIZE) {
        vm_crash(vm, EXCEPTION_JMP_OUT_OF_BOUNDS,
                .description = "Attempted to unconditionally jump out of program bounds",
                .detailed_description = vm_text_format("Attemped to jump to %d", jmp_to));
    }


    vm_verbose(" program_counter -> %.2d }\n", jmp_to);
    vm->program_counter = jmp_to;
}

void je(VM *vm) {
    vm_verbose("JE: {");
    vm->program_counter++;
    i32 jump_to = vm->program[vm->program_counter];

    if(jump_to > MAX_PROGRAM_SIZE) {
        vm_crash(vm, EXCEPTION_JMP_OUT_OF_BOUNDS,
                .description = "Attempted to conditionally jump (JE) out of program bounds",
                .detailed_description = vm_text_format("Attemped to jump to %d", jump_to));
    }


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

    if(jump_to > MAX_PROGRAM_SIZE) {
        vm_crash(vm, EXCEPTION_JMP_OUT_OF_BOUNDS,
                .description = "Attempted to conditionally jump (JNE) out of program bounds",
                .detailed_description = vm_text_format("Attemped to jump to %d", jump_to));
    }


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

    if(jump_to > MAX_PROGRAM_SIZE) {
        vm_crash(vm, EXCEPTION_JMP_OUT_OF_BOUNDS,
                .description = "Attempted to conditionally jump (JGE) out of program bounds",
                .detailed_description = vm_text_format("Attemped to jump to %d", jump_to));
    }


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

    if(jump_to > MAX_PROGRAM_SIZE) {
        vm_crash(vm, EXCEPTION_JMP_OUT_OF_BOUNDS,
                .description = "Caught attempt to conditionally jump (JLE) out of program bounds",
                .detailed_description = vm_text_format("Attemped to jump to %d", jump_to));
    }


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

    if(reg_b == (i32)0) {
        vm_crash(vm, EXCEPTION_DIVISION_BY_ZERO,
                .description = "Caught attemped integer divison by 0",
                .detailed_description = vm_text_format("Attemped operation '%d / %d'", reg_a, reg_b));
    }

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

    if(vm->stack_head >= MAX_STACK_SIZE) {
        vm_crash(vm, EXCEPTION_STACK_OVERFLOW, .description = "Stack limit reached");
    }

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

    if(vm->stack_head >= MAX_STACK_SIZE) {
        vm_crash(vm, EXCEPTION_STACK_OVERFLOW, .description = "Stack limit reached");
    }


    vm->program_counter++;
    i32 value = vm->program[vm->program_counter];

    vm->stack[vm->stack_head] = value;
    vm_verbose(" stack[%d] = %d }\n", vm->stack_head, value);
    vm->stack_head++;


    vm->program_counter++;
}

void pop(VM *vm) {
    vm_verbose("POP: {");

    if(vm->stack_head <= 0) {
        vm_crash(vm, EXCEPTION_STACK_UNDERFLOW, .description = "Pop attempted on empty stack");
    }

    vm->program_counter++;

    i32 reg_ind = vm->program[vm->program_counter];

    vm->registers[reg_ind] = vm->stack[vm->stack_head - 1];
    vm_verbose(" reg[%d] = %d }\n", reg_ind, vm->stack[vm->stack_head - 1]);
    vm->stack_head--;

    vm->program_counter++;
}

void void_pop(VM *vm) {
    vm_verbose("VOID_POP: {");

    if(vm->stack_head <= 0) {
        vm_crash(vm, EXCEPTION_STACK_UNDERFLOW, .description = "Pop attempted on empty stack");
    }


    vm->stack_head--;
    vm_verbose(" vm->stack_head = %d }\n", vm->stack_head);

    vm->program_counter++;
}

void call(VM *vm) {
    vm_verbose("CALL: {");
    vm->program_counter++;

    if(vm->return_address_head >= MAX_STACK_SIZE) {
        vm_crash(vm, EXCEPTION_STACK_UNDERFLOW,
                .description = "Return address stack limit reached",
                .detailed_description = "Too many nested function calls (recurson depth limit)");
    }

    vm->return_address_stack[vm->return_address_head] = vm->program_counter + 1;
    vm->return_address_head++;

    i32 jmp_to = vm->program[vm->program_counter];
    vm_verbose(" program_counter -> %.2d }\n", jmp_to);
    vm->program_counter = jmp_to;
}

void ret(VM *vm) {
    vm_verbose("RET: {");

    if(vm->return_address_head <= 0) {
        vm_crash(vm, EXCEPTION_STACK_UNDERFLOW,
                .description = "RET calleed on empty return address stack",
                .detailed_description = "Attempt of return without matching call");
    }

    vm_verbose(" program_counter = %d }\n", vm->return_address_stack[vm->return_address_head - 1]);
    vm->program_counter = vm->return_address_stack[vm->return_address_head - 1];

    vm->return_address_head--;
}

void syscall_(VM *vm) {
    vm_verbose("SYSCALL: {");
    vm->program_counter++;

    i32 syscall_num = vm->registers[REG_ARG_A];

    switch((Syscall_numbers)syscall_num) {

        case WRITE_SYSCALL: { /* write(fd, buff_addr, count) */
            i32 fd = vm->registers[REG_ARG_B];
            i32 buff_addr = vm->registers[REG_ARG_C];
            i32 count = vm->registers[REG_ARG_D];
            
            vm_verbose(" write(%d, 0x%x, %d)", fd, buff_addr, count);

            char host_buffer[1024];
            i32 processed = 0;

            while (processed < count) {
                i32 chunk = (count - processed > 1024) ? 1024 : (count - processed);
                i32 actual_bytes = 0;


                for(i32 i = 0; i < chunk; ++i) {
                    i32 *ptr = get_vm_ptr(vm, buff_addr + processed + i);
                    if(ptr) {
                        host_buffer[actual_bytes++] = (char)(*ptr & 0xFF);
                    }
                }

                if (actual_bytes > 0) {
                    write(fd, host_buffer, actual_bytes);
                }
                processed += chunk;

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
	    i32 buff_addr = vm->registers[REG_ARG_B];
	    i32 flags = vm->registers[REG_ARG_C];
	    i32 mode = vm->registers[REG_ARG_D];

            char *path = vm_get_cstring(vm, buff_addr);
            int fd = open(path, flags, mode);

            if(fd == -1) {
                vm_crash(vm, EXCEPTION_OPEN_SYSCALL_FAIL,
                        .description = vm_text_format("Failed whilest trying to open '%s'", path),
                        .detailed_description = "Could not open() the file");
            }
	    vm->registers[REG_ARG_A] = (i32)fd;

	    vm_verbose(" open(\"%s\", %d, %d) -> fd: %d }\n", path, flags, mode, fd);

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
            char host_buff[1024];

            while(bytes_read_total < count) {
                i32 to_read = (count - bytes_read_total > 1024) ? 1024 : (count - bytes_read_total);
                ssize_t n = read(fd, host_buff, to_read);
                if (n <= 0) break; 

                for (ssize_t i = 0; i < n; ++i) {
                    if (fd == 0 && host_buff[i] == '\n') {
                        n = i + 1;
                        break;
                    }
                    i32 *ptr = get_vm_ptr(vm, buff_addr + bytes_read_total);
                    if(ptr) {
                        *ptr = (i32)host_buff[i];
                        bytes_read_total++;
                    }
                }
                
                if(fd == 0 && host_buff[n-1] == '\n') break;
            }

           
            i32 *null_terminator_ptr = get_vm_ptr(vm, buff_addr + bytes_read_total);
            if(null_terminator_ptr) *null_terminator_ptr = 0;

            if (buff_addr == vm->registers[REG_HEAP_PTR]) {
                vm->registers[REG_HEAP_PTR] += (bytes_read_total + 1);
            }
            
            vm->registers[REG_RET] = bytes_read_total;
            vm_verbose(" read(%d, %d, %d) -> read %d bytes }\n", fd, buff_addr, count, bytes_read_total);

        } break;

        default:
            vm_crash(vm, EXCEPTION_INVALID_SYSCALL,
                    .description = "Caught a attempt at executing a invalid or unimplemented syscall",
                    .detailed_description = vm_text_format("Attempted to execute '%d' as syscall number", syscall_num),
                    .dump_vm_struct = true);

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

    i32 addr_val = vm->program[vm->program_counter];
    i32 *data_addr = get_vm_ptr(vm, addr_val);

    if(data_addr == NULL) {
        vm_crash(vm, EXCEPTION_ILLEGAL_READ,
                .description = vm_text_format("Attempted to LDO from invalid address %d", addr_val),
                .detailed_description = "Address is outsite both ROM and RAM addressing spaces");
    }


    if(vm_ptr_info.ROM_addr) { vm_verbose(" @addr=%d(ROM)", vm_ptr_info.addr); }
    if(vm_ptr_info.RAM_addr) { vm_verbose(" @addr=%d(RAM)", vm_ptr_info.addr); }

    vm->program_counter++;
    i32 dest_reg = vm->program[vm->program_counter];


    if(dest_reg < 0 || dest_reg >= REG_COUNT) {
        vm_crash(vm, EXCEPTION_ILLEGAL_STATE, 
                .description = vm_text_format("Invalid register index %d in LDO", dest_reg));
    }

    if(is_readonly_register(dest_reg)) {
        vm_crash(vm, EXCEPTION_ILLEGAL_WRITE,
            .description = vm_text_format("Caugth attempt of write to Read-Only register %d", dest_reg),
            .detailed_description = "Writing to Read-Only registers is not allowed");
    }


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

    vm->program_counter++;
    i32 index_reg_idx = vm->program[vm->program_counter];
    i32 index_offset = vm->registers[index_reg_idx];

    vm->program_counter++;
    i32 dest_reg = vm->program[vm->program_counter];

    if(dest_reg < 0 || dest_reg >= REG_COUNT) {
        vm_crash(vm, EXCEPTION_ILLEGAL_STATE, 
                .description = vm_text_format("Invalid register index %d in LDO", dest_reg));
    }

    if(is_readonly_register(dest_reg)) {
        vm_crash(vm, EXCEPTION_ILLEGAL_WRITE,
            .description = vm_text_format("Caugth attempt of write to Read-Only register %d", dest_reg),
            .detailed_description = "Writing to Read-Only registers is not allowed");
    }


    i32 target_addr = base_addr + index_offset;
    i32 *final_addr = get_vm_ptr(vm, base_addr + index_offset);

    if(final_addr == NULL) {
        vm_crash(vm, EXCEPTION_ILLEGAL_READ,
                .description = vm_text_format("Attempted to LDXO from invalid address %d", target_addr),
                .detailed_description = "Address is outsite both ROM and RAM addressing spaces");
    }



    if(vm_ptr_info.ROM_addr) { vm_verbose(" base$%d(%d)(ROM)", base_reg_idx, base_addr); }
    if(vm_ptr_info.RAM_addr) { vm_verbose(" base$%d(%d)(RAM)", base_reg_idx, base_addr); }


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
                .description = vm_text_format("Caught attempt to write '%d' to (ROM)'%d'",
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

void dlopen_(VM *vm) {
    vm_verbose("DLOPEN {");
    vm->program_counter++;

    i32 buff_addr = vm->program[vm->program_counter];

    vm->program_counter++;

    i32 reg_szof_addr = vm->program[vm->program_counter];

    i32 length = vm->registers[reg_szof_addr];

    char *string = vm_get_string(vm, buff_addr, length);
    vm_verbose(" --> '%s' }\n", string);


    if(vm->extern_handle_count >= MAX_EXTERNAL_LIBS) {
        vm_crash(vm, EXCEPTION_TOO_MANY_EXTERN_SYMBOLS,
                .description = vm_text_format("Failed whilist trying to load '%s'", string),
                .detailed_description = vm_text_format("Cannot load more than %d external libraries.", MAX_EXTERNAL_LIBS));

    }
    

    vm->extern_handle[vm->extern_handle_count].handle = dlopen(string, RTLD_LAZY | RTLD_GLOBAL);

    if(!vm->extern_handle[vm->extern_handle_count].handle) {
        vm_crash(vm, EXCEPTION_DLOPEN_FAIL,
                .description = vm_text_format("Failed oppening '%s'", string),
                .detailed_description = vm_text_format("dlerror(): %s", dlerror()));
    }
    

    vm->extern_handle_count++;


    i32 (*set_internal)(__vmasm_internal) = dlsym(RTLD_DEFAULT, "__vmasm_internal_set_internal");

    set_internal((__vmasm_internal){.vm = vm,
            .set_register = vm_set_register,
            .get_register = vm_get_register});

       




    free(string);
    vm->program_counter++;
}


void extern_(VM *vm) {
    vm_verbose("EXTERN: {");
    vm->program_counter++;

    i32 buff_addr = vm->program[vm->program_counter];

    vm->program_counter++;

    i32 reg_szof_addr = vm->program[vm->program_counter];

    i32 length = vm->registers[reg_szof_addr];

    char *string = vm_get_string(vm, buff_addr, length);

    
    VMASMObject tmp = (VMASMObject) {
        .arg_a = vm->registers[REG_ARG_A],
        .arg_b = vm->registers[REG_ARG_B],
        .arg_c = vm->registers[REG_ARG_C],
        .arg_d = vm->registers[REG_ARG_D],
        .arg_e = vm->registers[REG_ARG_E],
        .arg_f = vm->registers[REG_ARG_F],
        .arg_g = vm->registers[REG_ARG_G],
        .arg_h = vm->registers[REG_ARG_H],
    };

    void *symbol = dlsym(RTLD_DEFAULT, string);

    if(symbol == NULL) {
        vm_crash(vm, EXCEPTION_EXTSYM_RESOLUTION_FAIL,
                .description = vm_text_format("While trying to resolve external symbol '%s'",
                    string),
                .detailed_description = vm_text_format("Does this symbol exist in the library?"));
    }
    
    extern_signature f = (extern_signature)symbol;

    i32 result = f(tmp);

    vm_verbose(" $ret = %s((VMASMObject){"VMASMObject_Fmt"}); -> %d }\n", string, VMASMObject_Arg(tmp), result);

    vm->registers[REG_RET] = result;


    free(string);
    vm->program_counter++;
}

void extern_str(VM *vm) {

    vm_verbose("EXTERN_STR: {");
    vm->program_counter++;

    i32 buff_addr = vm->program[vm->program_counter];

    vm->program_counter++;

    i32 reg_szof_addr = vm->program[vm->program_counter];

    i32 length = vm->registers[reg_szof_addr];

    char *string = vm_get_string(vm, buff_addr, length);
    vm_verbose(" set global string to -> '%s'}\n", string);

    vm_internal_setstring_all_libraries(vm, string);

    free(string);
    vm->program_counter++;
}

void r_extern(VM *vm) {
    vm_verbose("R_EXTERN: {");
    vm->program_counter++;

    i32 reg_idx = vm->program[vm->program_counter];

    i32 buff_addr = vm->registers[reg_idx];

    vm->program_counter++;

    i32 reg_szof_addr = vm->program[vm->program_counter];

    i32 length = vm->registers[reg_szof_addr];

    char *string = vm_get_string(vm, buff_addr, length);

    VMASMObject tmp = (VMASMObject) {
        .arg_a = vm->registers[REG_ARG_A],
        .arg_b = vm->registers[REG_ARG_B],
        .arg_c = vm->registers[REG_ARG_C],
        .arg_d = vm->registers[REG_ARG_D],
        .arg_e = vm->registers[REG_ARG_E],
        .arg_f = vm->registers[REG_ARG_F],
        .arg_g = vm->registers[REG_ARG_G],
        .arg_h = vm->registers[REG_ARG_H],
    };

    void *symbol = dlsym(RTLD_DEFAULT, string);

    if(symbol == NULL) {
        vm_crash(vm, EXCEPTION_EXTSYM_RESOLUTION_FAIL,
                .description = vm_text_format("While trying to resolve external symbol '%s'",
                    string),
                .detailed_description = vm_text_format("Does this symbol exist in the library?"));
    }
    
    extern_signature f = (extern_signature)symbol;

    i32 result = f(tmp);

    vm_verbose(" $ret = %s((VMASMObject){"VMASMObject_Fmt"}); -> %d }\n", string, VMASMObject_Arg(tmp), result);

    vm->registers[REG_RET] = result;


    free(string);
    vm->program_counter++;
}

void r_extern_str(VM *vm) {
    vm_verbose("R_EXTERN_STR: {");
    vm->program_counter++;

    i32 reg_addr_idx = vm->program[vm->program_counter];
    i32 buff_addr = vm->registers[reg_addr_idx];

    vm_verbose(" buff_addr=$%d(%d)", reg_addr_idx, buff_addr);
    vm->program_counter++;

    i32 reg_szof_idx = vm->program[vm->program_counter];
    i32 length = vm->registers[reg_szof_idx];

    char *string = vm_get_string(vm, buff_addr, length);
    vm_verbose(" set global string to -> '%s' }\n", string);

    vm_internal_setstring_all_libraries(vm, string);

    free(string);
    vm->program_counter++;
}
