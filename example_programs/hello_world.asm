toggle_verbose 0
mov write_syscall, $arg_a   # set syscall type
mov stdout, $arg_b          # set file descriptor for write syscall 
mov @msg, $arg_c            # pass the string buffer for write syscall
strlen @msg, $arg_d         # pass the string size to write syscall
syscall                     # execute the syscall = write(stdout, &msg, strlen(msg))
line_br

halt

.data
    msg: "Hello, World!"

