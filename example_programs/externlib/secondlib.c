#include <stdio.h>
#include <stdint.h>

typedef int32_t i32;

i32 hello_another_lib(void) {
    printf("Hello, from a second library!\n");
    return 0;
}
