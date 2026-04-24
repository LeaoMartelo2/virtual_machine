#include <stdio.h>
#include <stdint.h>

typedef int32_t i32;

i32 hello_from_lib(void) {
    printf("Hello, from the first library!!\n");
    return 0;
}

