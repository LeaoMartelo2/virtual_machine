toggle_verbose 1
call .fill_stack         # fills 8 slots of the stack with numbers in order
void_pop                 # pops a few of them
void_pop                 # to showcase that it does not clear the stack
void_pop                 # but only changes where the stack head points
void_pop                 
void_pop
void_pop
stack_dump               # prints info about the value of stack_head and its surrounding values
halt



fill_stack:
mov 0, $1
mov 7, $2
start:
push $1
inc $1
cmp $1, $2
jle .start
ret


