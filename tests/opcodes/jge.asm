toggle_verbose 0
mov 0 $0
mov 1, $1
mov 20, $10
mov 10, $20
cmp $10, $20
jge .greater
print_int $0
line_br
halt
greater:
print_int $1
line_br
halt
