toggle_verbose 0
mov 30, $1
toggle_verbose 1
push $1 
i_push 10
register_dump $1, $1
halt
