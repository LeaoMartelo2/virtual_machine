toggle_verbose 0
mov 0 $0
mov 1, $1
mov 5, $10
mov 20, $20
cmp $10, $20
jle .less
print_int $0
line_br
halt
less:
print_int $1
line_br
halt
