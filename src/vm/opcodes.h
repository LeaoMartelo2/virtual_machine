#ifndef OPCODES_H
#define OPCODES_H

#include "../spec.h"

void no_op(VM *vm);
void halt(VM *vm);
void state_dump(VM *vm);
void register_dump(VM *vm);
void program_dump(VM *vm);
void toggle_verbose(VM *vm);
void mov(VM *vm);
void ld(VM *vm);
void inc(VM *vm);
void dec(VM *vm);
void sto_pc(VM *vm);
void cmp(VM *vm);
void jmp(VM *vm);
void je(VM *vm);
void jne(VM *vm);
void jge(VM *vm);
void jle(VM *vm);
void add(VM *vm);
void sub(VM *vm);
void mul(VM *vm);
void div_(VM *vm);
void mod(VM *vm);
void push(VM *vm);
void i_push(VM *vm);
void ret(VM *vm);

#endif /* OPCODES_H */
