toggle_verbose 1
mov 30, $1
push $1 
i_push 10
pop $2
void_pop
register_dump $1, $5
mov 10, $arg_a
mov 10, $arg_b
call .add
ld $ret, $5
register_dump $1, $5
halt




add:
add $arg_a, $arg_b
ld $arg_a, $ret
ret
