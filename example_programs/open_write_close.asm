toggle_verbose 1

mov open_syscall, $arg_a
mov @filename, $arg_b

mov O_RDWR, $0
mov O_CREAT, $1
mov O_TRUNC, $2
or $0, $1
or $0, $2
ld $0, $arg_c

mov perm_0644, $arg_d
syscall

ld $arg_a, $10

mov write_syscall, $arg_a
ld $10, $arg_b
mov @msg, $arg_c
strlen @msg, $arg_d
syscall


mov close_syscall, $arg_a
ld $10, $arg_b
syscall


halt






.data 
    filename: "file.txt"
    msg: "This is a really cool message im writing inside the file"
