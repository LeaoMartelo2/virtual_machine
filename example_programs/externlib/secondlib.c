#include <stdio.h>
#include "../../VMASM.h"

WORD hello_another_lib(void) {
    printf("Hello, %s, this is from a second library!\n", vmasm_get_globalstring());

    printf("setting register 5 to 10\n");
    vmasm_set_register(5, 10);

    printf("register 5 value: %d\n", vmasm_get_register(5));
    return 0;
}
