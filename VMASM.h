#ifndef VMASM_H
#define VMASM_H

#ifndef VMASM_INTERNAL

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef int32_t i32;
typedef int32_t WORD;


typedef struct VM VM;


#define UNUSED(x) (void)(x)

#endif /* VMASM_INTERNAL */

typedef void (*set_register_ptr)(VM *, i32, i32);
typedef i32 (*get_register_ptr)(VM *, i32);

typedef struct VMASMObject{
    i32 arg_a;
    i32 arg_b;
    i32 arg_c;
    i32 arg_d;
    i32 arg_e;
    i32 arg_f;
    i32 arg_g;
    i32 arg_h;
} VMASMObject;

#define VMASMObject_Fmt "%d, %d, %d, %d, %d, %d, %d, %d"
#define VMASMObject_Arg(obj) (obj).arg_a, (obj).arg_b, (obj).arg_c, (obj).arg_d, (obj).arg_e, (obj).arg_f, (obj).arg_g, (obj).arg_h


typedef struct {
    char *global_string;

    VM *vm;
    set_register_ptr set_register;
    get_register_ptr get_register;

} __vmasm_internal;


#ifndef VMASM_INTERNAL

__vmasm_internal __internal = {
    .global_string = NULL,
};

i32 __vmasm_internal_set_internal(__vmasm_internal external) {

    //printf("__vmasm_internal_set_internal called\n");

    __internal.vm = external.vm;
    __internal.set_register = external.set_register;
    __internal.get_register = external.get_register;
    return 0;
}

i32 __vmasm_internal_setstring(const char *str) {

    if(__internal.global_string != NULL) {
        free(__internal.global_string);
        __internal.global_string = NULL;
    }

    if (str == NULL) {
        return 0;
    }

    __internal.global_string = strdup(str);

    if(__internal.global_string == NULL) {
        return 1;
    }

    return 0;
}

const char *vmasm_get_globalstring() {
    return __internal.global_string;
}

void vmasm_set_register(i32 index, i32 value) {
    __internal.set_register(__internal.vm, index, value);
}

i32 vmasm_get_register(i32 index) {
    return __internal.get_register(__internal.vm, index);
}


#endif /* VMASM_INTERNAL */
#endif /* VMASM_H */
