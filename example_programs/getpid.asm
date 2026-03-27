toggle_verbose 1

mov getpid_syscall, $arg_a
syscall

print_int $arg_b
line_br

mov kill_syscall, $arg_a
mov 9, $arg_c
syscall

halt
