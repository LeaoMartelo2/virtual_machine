# you can leave comments with a hash 
toggle_verbose 1       # enable verbose output on the machine 
mov 10, $10            # stores '10' in the register 10
loop_start:            # define a Label where the loop starts
inc $1                 # increments the value at register 1 ( reg1++)
cmp $1, $10            # compares register 1 with register 10
jle .loop_start        # goes to the start of the loop of the comparasion was less or equal
state_dump             # displays some information about the machine 
halt                   # finishes machine execution and exits
