toggle_verbose 0

strlen @libfirst, $10
dlopen @libfirst, $10

strlen @libsecond, $10
dlopen @libsecond, $10

strlen @first_func, $5
extern @first_func, $5

strlen @second_func, $5
extern @second_func, $5


halt


.data

    libfirst: "./example_programs/externlib/libfirst.so"
    libsecond: "./example_programs/externlib/libsecond.so"
    first_func: "hello_from_lib"
    second_func: "hello_another_lib"


