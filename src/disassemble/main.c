#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#include "../spec.h"

void disassemble(i32 *buffer, size_t count) {

    size_t pc = 0;

    while (pc < count) {

        i32 opcode_value = buffer[pc];

        if (opcode_value < 0 || opcode_value > OPCODE_COUNT) {
            printf("%" PRId32 ": [!] UNKNOWN OPCODE at %zu \n", opcode_value, pc);
            pc++;
            continue;
        }

        const Instruction_spec *spec = &ASSEMBLY_TABLE[opcode_value];

        if (pc + spec->arg_count >= count) {
            printf("0x%08zx: [!] ERROR: Incomplete instruction '%s' at the end of the file\n", pc, spec->name);
            break;
        }

        printf("0x%08zx: %-12s", pc, spec->name);

        for (int i = 0; i < spec->arg_count; i++) {
            i32 val = buffer[pc + 1 + i];

            if (spec->arg_types[i] == ARG_REG) {
                printf(" reg%d", val);
            } else {
                printf(" %d", val);
            }

            if (i < spec->arg_count - 1) {
                printf(",");
            }
        }

        printf("\n");

        pc += 1 + spec->arg_count;
    }
}

int main(int argc, char **argv) {

    if (argc < 2) {
        printf("Usage: %s <raw obj data>\n", argv[0]);
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

    printf("Read %zu instructions from %s\n\n", read_program_size, argv[1]);

    printf("==========================================================\n");
    disassemble(read_object, read_program_size);
    printf("==========================================================\n");

    return 0;
}
