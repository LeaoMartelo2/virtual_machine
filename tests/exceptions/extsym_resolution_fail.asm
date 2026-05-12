toggle_verbose 0 
strlen @path, $10
dlopen @path, $10
strlen @func, $10
extern @func, $10
halt
.data
    path: "./libtest.so"
    func: "unexistent_symbol"
