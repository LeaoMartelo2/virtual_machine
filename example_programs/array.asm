toggle_verbose false
mov 8, $8

mov @scores, $1

mov 0, $2
loop_start:
    cmp $2, $8
    jge .loop_end

    ldxo $1, $2, $3
    print_int $3 
    line_br
    
    inc $2
    jmp .loop_start

loop_end:
halt

.data
    scores: 5 10 15 20 25 30 35 40
