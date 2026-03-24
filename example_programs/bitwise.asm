toggle_verbose false

call .reset
and $1, $2
print_int $1
line_br

call .reset
or $1, $2
print_int $1
line_br

call .reset
xor $1, $2
print_int $1
line_br

call .reset
not $1
print_int $1
line_br

call .reset
lsh $1
print_int $1
line_br

call .reset
rsh $1
print_int $1
line_br

mov 2, $10

call .reset
lsha $1, $10
print_int $1
line_br

call .reset
rsha $1, $10
print_int $1
line_br

halt


reset:
    mov 10, $1
    mov 6, $2
    ret
