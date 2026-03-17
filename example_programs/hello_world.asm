toggle_verbose 0
mov 10, $1             # store value of "\n" in reg1
no_op             
mov 1, $arg_a          # syscall number 1 = write
mov 1, $arg_b          # set file descriptor for write syscall (1 = stdout)
mov @msg, $arg_c       # pass the string buffer to write syscall
strlen @msg, $arg_d    # pass the string size to write syscall
syscall                # execute the syscall = write(1, &msg, strlen(msg))
print_char $1          # print the newline

halt

.data
    msg: "Hello, World!"

