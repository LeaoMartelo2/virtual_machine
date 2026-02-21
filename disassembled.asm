mov -5, reg1
mov 0, reg4
no_op
ld reg1, reg2
no_op
sto_pc reg0
inc reg3
cmp reg3, reg4
je reg0
no_op
dec reg3
no_op
sto_pc reg9
inc reg1
cmp reg1, reg4
jne reg9
no_op
mov 22, reg6
mov 20, reg7
sto_pc reg9
dec reg6
cmp reg6, reg7
jge reg9
no_op
mov 18, reg6
mov 20, reg7
sto_pc reg9
inc reg6
cmp reg6, reg7
jle reg9
no_op
state_dump
program_dump
halt
