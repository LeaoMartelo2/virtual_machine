#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../spec.h"

int main(int argc, char **argv) {

    char *input_path = NULL;
    // default to out.obj if not specified
    char *output_path = "out.obj";

    if (argc < 2) {
        printf("Usage: %s <input file> [-o <output>]\n", argv[0]);
        printf("Options:\n  -o: specify output path, otherwise 'out.obj'\n");
        exit(1);
    }

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-o") == 0) {
            if (i + 1 < argc) {
                output_path = argv[++i];
            } else {
                fprintf(stderr, "Error: -o needs a file path to be specified.\n");
                exit(1);
            }
        } else if (argv[i][0] != '-') {
            input_path = argv[i];
        }
    }

    if (!input_path) {
        fprintf(stderr, "Error: input file not specified.\n");
        exit(1);
    }

    FILE *file = fopen(input_path, "r");
    if (!file) {
        perror("Failed to open input file.");
        exit(1);
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = malloc(file_size + 1);
    if (!buffer) {
        perror("malloc failed lmaoooo");
        fclose(file);
        exit(1);
    }

    fread(buffer, 1, file_size, file);
    buffer[file_size] = '\0';
    fclose(file);

    const char *tokenizer_split = " ,\t\n\r";
    char *token = strtok(buffer, tokenizer_split);
    i32 program_buffer[MAX_PROGRAM_SIZE];
    size_t head_pos = 0;

    while (token != NULL) {
        bool found = false;

        for (i32 i = 0; i < OPCODE_COUNT; ++i) {
            if (strcmp(token, ASSEMBLY_TABLE[i].name) == 0) {
                found = true;
                program_buffer[head_pos++] = i;

                const char *current_instr_name = ASSEMBLY_TABLE[i].name;

                for (int j = 0; j < ASSEMBLY_TABLE[i].arg_count; ++j) {
                    token = strtok(NULL, tokenizer_split);

                    if (token == NULL) {
                        fprintf(stderr, "Error: missing argument for '%s'\n", current_instr_name);
                        exit(1); 
                    }

                    if (ASSEMBLY_TABLE[i].arg_types[j] == ARG_REG) {
                        if (token[0] == '%') {
                            if (token[1] == '\0') {
                                fprintf(stderr, "Error: empty register name '%%'\n");
                                exit(1);
                            }
                            int reg_idx = atoi(token + 1);

                            if (reg_idx >= 0 && reg_idx < REG_COUNT) {
                                program_buffer[head_pos++] = reg_idx;
                            } else {
                                fprintf(stderr, "Error: register index invalid %d.\n", reg_idx);
                                exit(1);
                            }
                        } else {
                            fprintf(stderr, "Error: expected prefix '%%' for register, found '%s'.\n", token);
                            exit(1);
                        }
                    } else if (ASSEMBLY_TABLE[i].arg_types[j] == ARG_VAL) {
                        program_buffer[head_pos++] = (i32)atoi(token);
                    }
                }
                break; 
            }
        }

        if (!found) {
            fprintf(stderr, "Error: unknown instruction '%s'.\n", token);
            return 1;
        }

        token = strtok(NULL, tokenizer_split);
    }

    /*
    for (size_t i = 0; i < head_pos; ++i) {
        printf("%d ", program_buffer[i]);
    }
    printf("\n");

    */



    FILE *output_file = fopen(output_path, "wb");

    if(!output_file) {
        perror("Failed to open output file.");
        exit(1);
    }

    fwrite(program_buffer, sizeof(i32), head_pos, output_file);

    fclose(output_file);
    free(buffer);

    return 0;
}
