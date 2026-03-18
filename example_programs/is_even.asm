toggle_verbose false
no_op
mov 0, $0
mov 1, $1
mov 2, $2

mov 5, $5
ld $5, $3

mod $3, $2

cmp $3 $0
je .is_even
jne .is_odd



is_even:
    mov @even, $arg_c
    strlen @even, $arg_d
    call .print_result


is_odd:
    mov @odd, $arg_c
    strlen @odd, $arg_d
    call .print_result


print_result:
    mov write_syscall, $arg_a
    mov stdout, $arg_b
    print_int $5 
    syscall  
    line_br
    halt
    


.data
    even: " is even."
    odd: " is odd."
