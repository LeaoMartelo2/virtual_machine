toggle_verbose 1

strlen @path, $10

start:
    dlopen @path, $10
jmp .start

halt

.data
    path: "./libtest.so"
