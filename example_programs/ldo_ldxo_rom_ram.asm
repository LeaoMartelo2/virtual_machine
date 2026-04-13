toggle_verbose 1

ldo @another_label, $1
print_int $1
line_br

no_op


ld $heap, $11
mov read_syscall, $arg_a
mov stdin, $arg_b
ld $heap, $arg_c
mov 1024, $arg_d
syscall

mov 0, $0

ldxo $11, $0, $1
print_char $1
line_br

halt


.data
    rom_value: 10 20 30
    another_label: 40, 50, 60
