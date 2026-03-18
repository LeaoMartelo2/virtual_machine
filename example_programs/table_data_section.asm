toggle_verbose 1
jmp .start
    
table:
    mov 10, $arg_b
    cmp $arg_a, $arg_b
    je .table_ten

    mov 20, $arg_b
    cmp $arg_a, $arg_b
    je .table_twenty

    mov 30, $arg_b
    cmp $arg_a, $arg_b
    je .table_thirty

    jmp .table_unknown

    table_ten:
        ldo @ten, $ret
        jmp .table_ret

    table_twenty:
        ldo @twenty, $ret
        jmp .table_ret
    
    table_thirty:
        ldo @thirty, $ret 
        jmp .table_ret

    table_unknown:
        ldo @error, $ret
        jmp .table_ret

    table_ret:
        ret

start:
    mov 10, $arg_a
    call .table
    ld $ret, $1

    mov 20, $arg_a
    call .table
    ld $ret, $2

    mov 30, $arg_a
    call .table
    ld $ret, $3

    mov 185, $arg_a
    call .table
    ld $ret, $4


    register_dump $1, $4

    halt


.data
    ten: 100
    twenty: 200
    thirty: 300
    error: -1


