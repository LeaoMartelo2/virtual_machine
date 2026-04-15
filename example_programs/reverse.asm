toggle_verbose 0


strlen @text, $10
dec $10
mov @text, $0
mov 0, $5

loop:

    ldxo $0, $10, $1

    print_char $1
    dec $10
    cmp $10, $5
    jge .loop

line_br


halt

.data 
    text: "BUT IT WILL BE REVERSE"
