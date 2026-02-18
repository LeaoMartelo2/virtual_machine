#ifndef OPCODES_H
#define OPCODES_H

#include "spec.h"

void no_op(VM *vm);
void halt(VM *vm);
void state_dump(VM *vm);
void mov(VM *vm);
void ld(VM *vm);
void inc(VM *vm);
void sto_pc(VM *vm);
void jmp(VM *vm);

#endif /* OPCODES_H */
