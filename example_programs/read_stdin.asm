toggle_verbose 1

# --- Step 1: Open file ---
mov open_syscall, $arg_a
mov @filename, $arg_b
mov O_RDWR, $0
mov O_CREAT, $1
mov O_TRUNC, $2
or $0, $1
or $0, $2
ld $0, $arg_c
mov perm_0644, $arg_d
syscall
ld $arg_a, $10          # $10 = File Descriptor

# --- Step 2: Read into RAM ---
ld $heap, $11           # Save the start of our string in $11
mov read_syscall, $arg_a
mov stdin, $arg_b
ld $heap, $arg_c        # Destination = current heap
mov 1024, $arg_d        # Max buffer
syscall                 # VM blocks here until you press Enter

# --- Step 3: Get Length and Write ---
# Use the register-based strlen because $11 is a RAM address
strlen_r $11, $12       # $12 = length of string in RAM

mov write_syscall, $arg_a
ld $10, $arg_b          # FD
ld $11, $arg_c          # RAM Address
ld $12, $arg_d          # Length
syscall

# --- Step 4: Cleanup ---
mov close_syscall, $arg_a
ld $10, $arg_b
syscall

halt

.data
    filename: "file.txt"
