#include <stdio.h>
#include <stdlib.h>

#include "../spec.h"

int main(int argc, char **argv) {

    if (argc < 2) {
        printf("Usage: %s <obj file>\n", argv[0]);
        exit(1);
    }

    printf("Trying to disassemble file: '%s'\n", argv[1]);

    FILE *file = fopen(argv[1], "rb");
    if (!file) {
        perror("Error oppening file");
        exit(1);
    }

    i32 read_object[MAX_PROGRAM_SIZE];
    
    size_t read_program_size = fread(read_object, sizeof(i32), MAX_PROGRAM_SIZE, file);
    fclose(file);

    printf("Read %zu bytes from %s\n", read_program_size, argv[1]);

    for(size_t i = 0; i < read_program_size; ++i) {
        printf("%d ", read_object[i]);
    }

    printf("\n");

    return 0;
}
