// this leaks memory like crazy xd

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../spec.h"

typedef struct {
    char *name;
    i32 address;
} Label;

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
    char *token;
    Label symbol_table[1024];
    int label_count = 0;
    i32 program_buffer[MAX_PROGRAM_SIZE];
    size_t head_pos = 0;
    size_t virtual_head_pos = 0;

    char *buffer_copy = strdup(buffer);

    // 1st pass: fill symbol table

    token = strtok(buffer_copy, tokenizer_split);

    while (token != NULL) {

        if (token[0] == '#') {
            // ignore comments
            token = strtok(NULL, "\n");
            token = strtok(NULL, tokenizer_split);
            continue;
        }

        size_t len = strlen(token);

        if (token[len - 1] == ':') {
            char label_name[256];
            strncpy(label_name, token, len - 1);
            label_name[len - 1] = '\0';

            symbol_table[label_count].name = strdup(label_name);
            symbol_table[label_count].address = (i32)virtual_head_pos;
            label_count++;
        } else {
            // bool found = false;

            for (i32 i = 0; i < OPCODE_COUNT; ++i) {
                if (strcmp(token, ASSEMBLY_TABLE[i].name) == 0) {
                    // found = true;
                    //  jumps the opcode itself + the opcode arguments ammount
                    virtual_head_pos += 1;
                    virtual_head_pos += ASSEMBLY_TABLE[i].arg_count;

                    for (int j = 0; j < ASSEMBLY_TABLE[i].arg_count; ++j) {
                        strtok(NULL, tokenizer_split);
                    }
                    break;
                }
            }
        }

        token = strtok(NULL, tokenizer_split);
    }

    // 2nd pass; actually generate the bytecode now

    token = strtok(buffer, tokenizer_split);

    while (token != NULL) {
        // skip comments
        if (token[0] == '#') {
            token = strtok(NULL, "\n");
            token = strtok(NULL, tokenizer_split);
            continue;
        }

        // skip labels
        size_t len = strlen(token);
        if (len > 0 && token[len - 1] == ':') {
            token = strtok(NULL, tokenizer_split);
            continue;
        }

        bool found_opcode = false;

        // instructions
        for (i32 i = 0; i < OPCODE_COUNT; ++i) {
            if (strcmp(token, ASSEMBLY_TABLE[i].name) == 0) {
                found_opcode = true;
                program_buffer[head_pos++] = i;

                const char *current_instr_name = ASSEMBLY_TABLE[i].name;

                for (int j = 0; j < ASSEMBLY_TABLE[i].arg_count; ++j) {
                    token = strtok(NULL, tokenizer_split);

                    if (token == NULL) {
                        fprintf(stderr, "Error: missing argument for '%s'\n", current_instr_name);
                        exit(1);
                    }

                    // registers
                    if (ASSEMBLY_TABLE[i].arg_types[j] == ARG_REG) {
                        if (token[0] == '%' || token[0] == '$') {

                            const char *reg_name = token + 1; /* skip prefix */

                            char *endptr;
                            int reg_idx = (int)strtol(reg_name, &endptr, 10);

                            if (*endptr == '\0') {
                                /* is a number */

                                if (reg_idx >= 0 && reg_idx < NAMED_REGISTERS_SPLIT) {
                                    program_buffer[head_pos++] = reg_idx;
                                } else {
                                    fprintf(stderr, "Error: invalid register index %d\n", reg_idx);
                                    exit(1);
                                }

                            } else {

                                /* not a number, search for named registers */

                                bool reg_found = false;

                                for (size_t r = 0; r < NAMED_REGISTER_COUNT; ++r) {
                                    if(strcmp(reg_name, NAMED_REGISTERS[r].name) == 0) {
                                        program_buffer[head_pos++] = NAMED_REGISTERS[r].reg_idx;
                                        reg_found = true;
                                        break;
                                    }
                                }

                                if (!reg_found) {
                                    fprintf(stderr, "Error: unknown named register '$%s'\n", reg_name);
                                    exit(1);
                                }

                            }


                        } else {
                            fprintf(stderr, "Error: expected '$' for register, found '%s'\n", token);
                            exit(1);
                        }
                    }
                    // values or labels
                    else if (ASSEMBLY_TABLE[i].arg_types[j] == ARG_VAL) {

                        // labels start with '.'
                        if (token[0] == '.') {
                            const char *label_to_find = token + 1;
                            bool label_found = false;

                            for (int l = 0; l < label_count; ++l) {
                                if (strcmp(label_to_find, symbol_table[l].name) == 0) {
                                    program_buffer[head_pos++] = symbol_table[l].address;
                                    label_found = true;
                                    break;
                                }
                            }

                            if (!label_found) {
                                fprintf(stderr, "Error: Label '%s' not defined.\n", label_to_find);
                                exit(1);
                            }
                        }
                        // literal number
                        else {
                            char *endptr;
                            i32 val = (i32)strtol(token, &endptr, 10);

                            if (*endptr == '\0') {
                                program_buffer[head_pos++] = val;
                            } else {
                                fprintf(stderr, "Error: Expected numeric value or .label, found '%s'\n", token);
                                exit(1);
                            }
                        }
                    }
                }
                break;
            }
        }

        if (!found_opcode) {
            fprintf(stderr, "Error: unknown instruction or stray token '%s'\n", token);
            exit(1);
        }

        token = strtok(NULL, tokenizer_split);
    }

    FILE *output_file = fopen(output_path, "wb");

    if (!output_file) {
        perror("Failed to open output file.");
        exit(1);
    }

    fwrite(program_buffer, sizeof(i32), head_pos, output_file);

    /*
    printf("--- DEBUG: BYTECODE GENERATED ---\n");
    for (size_t i = 0; i < head_pos; i++) {
        printf(

            "[%03zu] %d\n", i, program_buffer[i]);
    }
    printf("------------------------------\n");
    */

    fclose(output_file);
    free(buffer);

    return 0;
}
