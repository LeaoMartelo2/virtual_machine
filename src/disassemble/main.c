#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../spec.h"

void disassemble(i32 *buffer, size_t count, FILE *out, bool clean) {

    size_t pc = 0;

    while (pc < count) {

        i32 opcode_value = buffer[pc];

        if (opcode_value < 0 || opcode_value > OPCODE_COUNT) {
            if (!clean) fprintf(out, "%" PRId32 ": [!] UNKNOWN OPCODE at %zu \n", opcode_value, pc);
            pc++;
            continue;
        }

        const Instruction_spec *spec = &ASSEMBLY_TABLE[opcode_value];

        if (pc + spec->arg_count >= count) {
            if (!clean) fprintf(out, "0x%08zx: [!] ERROR: Incomplete instruction '%s' at the end of the file\n", pc, spec->name);
            break;
        }

        if (!clean) {
            fprintf(out, "0x%08zx: %-12s", pc, spec->name);
        } else {
            fprintf(out, "%s", spec->name);
        }

        for (int i = 0; i < spec->arg_count; i++) {
            i32 val = buffer[pc + 1 + i];

            if (spec->arg_types[i] == ARG_REG) {
                fprintf(out, " %%%d", val);
            } else {
                fprintf(out, " %d", val);
            }

            if (i < spec->arg_count - 1) {
                fprintf(out, ",");
            }
        }

        fprintf(out, "\n");

        pc += 1 + spec->arg_count;
    }
}

int main(int argc, char **argv) {

    if (argc < 2) {
        printf("Usage: %s <obj file> [-s]\n", argv[0]);
        printf("Options:\n -s: Save to 'disassembled.asm'\n");
        exit(1);
    }

    bool save_to_file = false;
    const char *input_path = NULL;

        for (int i = 1; i < argc; ++i) {
            if (strcmp(argv[i], "-s") == 0) {
            save_to_file = true;
        } else if (argv[i][0] != '-') {
            input_path = argv[i];
        }
    }

    if (!input_path) {
        printf("Error: No input file specified.\n");
        exit(1);
    }

    FILE *file = fopen(input_path, "rb");
    if (!file) {
        perror("Error oppening file");
        exit(1);
    }

    i32 read_object[MAX_PROGRAM_SIZE];

    size_t read_program_size = fread(read_object, sizeof(i32), MAX_PROGRAM_SIZE, file);
    fclose(file);

    printf("Read %zu instructions from %s\n\n", read_program_size, argv[1]);

    printf("==========================================================\n");
    disassemble(read_object, read_program_size, stdout, false);
    printf("==========================================================\n");

    if (save_to_file) {
        FILE *f_asm = fopen("disassembled.asm", "w");
        if (f_asm) {
            disassemble(read_object, read_program_size, f_asm, true);
            fclose(f_asm);
            printf("\nWrote 'disassembled.asm' sucessfully.\n");
        } else {
            perror("Error oppening file");
            exit(1);
        }
    }

    return 0;
}
