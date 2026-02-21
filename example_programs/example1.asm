mov -5, %1
mov 0, %4
no_op
ld %1, %2
no_op
sto_pc %0
inc %3
cmp %3, %4
je %0
no_op
dec %3
no_op
sto_pc %9
inc %1
cmp %1, %4
jne %9
no_op
mov 22, %6
mov 20, %7
sto_pc %9
dec %6
cmp %6, %7
jge %9
no_op
mov 18, %6
mov 20, %7
sto_pc %9
inc %6
cmp %6, %7
jle %9
no_op
state_dump
program_dump
halt
