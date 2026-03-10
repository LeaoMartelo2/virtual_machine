toggle_verbose 1
jmp .main

addition:
    add $arg_a, $arg_b
    ld $arg_a, $ret
    ret

subtraction:
    sub $arg_a, $arg_b 
    ld $arg_a, $ret 
    ret

multiplication:
    mul $arg_a, $arg_b 
    ld $arg_a, $ret 
    ret

division:
    div $arg_a, $arg_b 
    ld $arg_a, $ret 
    ret


main:
mov 10, $arg_a
mov 20, $arg_b

call .addition
ld $ret, $1 

call .subtraction
ld $ret, $2 

call .multiplication
ld $ret, $3

call .division
ld $ret, $4

register_dump $1, $4
halt
