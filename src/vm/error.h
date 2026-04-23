#ifndef ERROR_H
#define ERROR_H

#include "../spec.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>

#define STRINGFY(x) #x


static inline const char *vm_text_format(const char *text, ...) {
    
#ifndef MAX_TEXTFORMAT_BUFFERS
#define MAX_TEXTFORMAT_BUFFERS 4
#endif

#ifndef MAX_TEXT_BUFFER_LENGTH
#define MAX_TEXT_BUFFER_LENGTH 1024
#endif

    static char buffers[MAX_TEXTFORMAT_BUFFERS][MAX_TEXT_BUFFER_LENGTH];
    static int index = 0;

    char *current_buffer = buffers[index];
    memset(current_buffer, 0, MAX_TEXT_BUFFER_LENGTH);

    va_list args;
    va_start(args, text);
    int required_sz = vsnprintf(current_buffer, MAX_TEXT_BUFFER_LENGTH, text, args);
    va_end(args);

    if (required_sz >= MAX_TEXT_BUFFER_LENGTH) {
        fprintf(stderr, "Too large of a string to format:\n String passed: %s\n", text);
        exit(1);
    }

    index +=1;

    if (index >= MAX_TEXTFORMAT_BUFFERS) index = 0;

    return current_buffer;
}


typedef enum {
    EXCEPTION_ILLEGAL_WRITE,
    EXCEPTION_ILLEGAL_READ,
    EXCEPTION_ILLEGAL_STATE,
    EXCEPTION_UNKNOWN_INSTRUCTION,
    EXCEPTION_DIVISION_BY_ZERO,
    EXCEPTION_JMP_OUT_OF_BOUNDS,
    EXCEPTION_INVALID_SYSCALL,

    ERROR_COUNT
} Error_Type;

static const char *ErrorStrings[] = {
    [EXCEPTION_ILLEGAL_WRITE]       = STRINGFY(EXCEPTION_ILLEGAL_WRITE),
    [EXCEPTION_ILLEGAL_READ]        = STRINGFY(EXCEPTION_ILLEGAL_READ),
    [EXCEPTION_ILLEGAL_STATE]       = STRINGFY(EXCEPTION_ILLEGAL_STATE),
    [EXCEPTION_UNKNOWN_INSTRUCTION] = STRINGFY(EXCEPTION_UNKNOWN_INSTRUCTION),
    [EXCEPTION_DIVISION_BY_ZERO]    = STRINGFY(EXCEPTION_DIVISION_BY_ZERO),
    [EXCEPTION_JMP_OUT_OF_BOUNDS]   = STRINGFY(EXCEPTION_JMP_OUT_OF_BOUNDS),
    [EXCEPTION_INVALID_SYSCALL]     = STRINGFY(EXCEPTION_INVALID_SYSCALL),

};

static_assert((sizeof(ErrorStrings) / sizeof(ErrorStrings[0])) == ERROR_COUNT,
              "Error: ErrorStrings must contain all elements of the Error_Type table");

typedef struct __crash_details_t {
    const char *description;
    const char *detailed_description;
    i32 line_where;
    const char *file_where;
    const char *function_where;
    bool dump_vm_struct;
}crash_details;

_Noreturn static inline void vm_crash_opt(const VM *vm, Error_Type type, crash_details details) {

    FILE *file = fopen("crash_log.txt", "w");
    if(!file) {
        fprintf(stderr, "Failed to open crash_log.txt file\n"
            "There is somehow is a even bigger problem D:\n");
        exit(1);
    }

    fprintf(file, "====== A fatal error has been caught in the VMASM interpreter ======\n");
    fprintf(file, "ERROR: %s\n", ErrorStrings[type]);
    fprintf(file, "Caught in 0x%08lX", (unsigned long)&vm->program[vm->program_counter]);
    fprintf(file, " -> vm->program[%d]\n", vm->program_counter);
    fprintf(file, "Reported at %s:%d in %s()\n", details.file_where, details.line_where, details.function_where);

    if (details.description) fprintf(file, "Description: %s\n", details.description);
    if (details.detailed_description) fprintf(file, "More info: %s\n", details.detailed_description);
    fprintf(file, "=== INTERPRETER INFO ===\n");
    fprintf(file, "vm->program_counter      %d\n", vm->program_counter);
    fprintf(file, "current value            %d\n", vm->program[vm->program_counter]);


    if(details.dump_vm_struct) {

        FILE *structdump = fopen("VM.dump", "wb");
        if(!structdump) {
            fprintf(stderr, ":sob: how??\n");
            exit(1);
        }

        fwrite(vm, sizeof(VM), 1, structdump);
        fclose(structdump);

        fprintf(file, "Wrote 'VM.dump'\n");

    }

    fprintf(file, "===================================================================\n");
    fclose(file);


    printf("\n The VMASM interpreter caught a %s at %s:%d in %s()\n",
            ErrorStrings[type],
            details.file_where, details.line_where, details.function_where);
    
    printf("Details have been written to 'crash_log.txt'\n");

    if (details.dump_vm_struct) fprintf(stderr, "Wrote 'VM.dump'\n");


    exit(1);

}

#define vm_crash(vm_ptr, error_type, ...)  vm_crash_opt(vm_ptr, error_type, (crash_details){ .dump_vm_struct = false, .file_where = __FILE__, .line_where = __LINE__, .function_where = __func__, __VA_ARGS__})


#endif

