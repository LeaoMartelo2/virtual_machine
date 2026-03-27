toggle_verbose 0

mov 3, $10

mov write_syscall, $arg_a
mov stdout, $arg_b
mov @msg, $arg_c
strlen @msg, $arg_d
add $arg_d, $10
syscall

halt



.data
    msg: "123456"
    10
    msg2: "9"
