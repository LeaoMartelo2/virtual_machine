// this leaks memory like crazy xd

#define _POSIX_C_SOURCE 200809L 

#include <limits.h>
#include <libgen.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "../spec.h"

#define RUNNER_NAME "vm"

char *strdup_vm(const char *s) {
    if (s == NULL) return NULL;
    size_t len = strnlen(s, MAX_PROGRAM_SIZE) + 1;
    char *result = malloc(len);
    if (result != NULL) {
        memcpy(result, s, len);
    }
    return result;
}

void run_after_compile(const char *out_file) {
    char exe_path[PATH_MAX];
    char vm_path[PATH_MAX];
    char temp_path[PATH_MAX];

    ssize_t len = readlink("/proc/self/exe", exe_path, sizeof(exe_path) - 1);

    if (len == -1) {
        perror("readlink");
        return;
    }

    exe_path[len] = '\0';

    strncpy(temp_path, exe_path, sizeof(temp_path));
    char *dir = dirname(temp_path);

    if (snprintf(vm_path, sizeof(vm_path), "%s/" RUNNER_NAME, dir) >= (int)sizeof(vm_path)) {
        fprintf(stderr, "path too long\n");
        return;
    }

    if (access(vm_path, X_OK)) {
        perror("Accessing the runner failed");
        return;
    }

    char *args[] = {vm_path, (char *)out_file, NULL};
    execv(vm_path, args);

    perror("if you got here, execv failed miserably");
    exit(1);
}

typedef struct {
    char *name;
    i32 address;
    bool is_data_label;
} Label;

// custom tokenizer that respects quoted strings
// returns next token, or NULL if end of string
// handles quoted strings as single tokens
typedef struct {
    const char *buffer;
    size_t pos;
} Tokenizer;

Tokenizer tokenizer_create(const char *buffer) {
    Tokenizer t = {buffer, 0};
    return t;
}

char *tokenizer_next(Tokenizer *t) {
    // skip whitespace and delimiters
    while (t->buffer[t->pos] != '\0' && 
           (t->buffer[t->pos] == ' ' || t->buffer[t->pos] == '\t' || 
            t->buffer[t->pos] == '\n' || t->buffer[t->pos] == '\r' || 
            t->buffer[t->pos] == ',')) {
        t->pos++;
    }
    
    if (t->buffer[t->pos] == '\0') {
        return NULL;
    }
    
    size_t start = t->pos;
    
    // check if this is a quoted string
    if (t->buffer[t->pos] == '"') {
        t->pos++;  // skip opening quote
        while (t->buffer[t->pos] != '\0' && t->buffer[t->pos] != '"') {
            t->pos++;
        }
        if (t->buffer[t->pos] == '"') {
            t->pos++;  // skip closing quote
        }
    } else {
        // regular token read until whitespace or delimiter
        while (t->buffer[t->pos] != '\0' && 
               t->buffer[t->pos] != ' ' && t->buffer[t->pos] != '\t' && 
               t->buffer[t->pos] != '\n' && t->buffer[t->pos] != '\r' && 
               t->buffer[t->pos] != ',') {
            t->pos++;
        }
    }
    
    size_t len = t->pos - start;
    char *token = malloc(len + 1);
    if (token) {
        memcpy(token, t->buffer + start, len);
        token[len] = '\0';
    }
    
    return token;
}

int main(int argc, char **argv) {

    char *input_path = NULL;
    char *output_path = "out.bin";
    bool run_flag = false;

    if (argc < 2) {
        printf("Usage: %s <input file> [-o <output>] [-run]\n", argv[0]);
        printf("Options:\n");
        printf("  -o: specify output path, otherwise 'out.bin'\n");
        printf("  -run: automatically run the compiled program\n");
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
        } else if (strcmp(argv[i], "-run") == 0) {
            run_flag = true;
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
        perror("malloc failed");
        fclose(file);
        exit(1);
    }

    fread(buffer, 1, file_size, file);
    buffer[file_size] = '\0';
    fclose(file);

    Label symbol_table[1024];
    int label_count = 0;
    i32 program_buffer[MAX_PROGRAM_SIZE];
    i32 data_buffer[MAX_PROGRAM_SIZE];
    
    size_t program_head = 0;
    size_t data_head = 0;
    size_t virtual_program_pos = 0;
    size_t virtual_data_pos = 0;
    size_t max_limit = 256;
    bool in_data_section = false;

    // ========== PASS 1: build symbol table and detect sections ==========
    Tokenizer t1 = tokenizer_create(buffer);
    char *token;

    while ((token = tokenizer_next(&t1)) != NULL) {

        if (token[0] == '#') {
            free(token);
            // skip until end of line
            while (t1.buffer[t1.pos] != '\0' && t1.buffer[t1.pos] != '\n') {
                t1.pos++;
            }
            continue;
        }

        // check for section markers
        if (strcmp(token, ".data") == 0) {
            in_data_section = true;
            free(token);
            continue;
        }

        if (strcmp(token, ".text") == 0) {
            in_data_section = false;
            free(token);
            continue;
        }

        size_t len = strnlen(token, max_limit);

        // handle labels (must be length > 1 and end with ':')
        if (len > 1 && token[len - 1] == ':') {
            char label_name[256];
            strncpy(label_name, token, len - 1);
            label_name[len - 1] = '\0';

            symbol_table[label_count].name = strdup_vm(label_name);
            symbol_table[label_count].is_data_label = in_data_section;
            
            if (in_data_section) {
                symbol_table[label_count].address = (i32)virtual_data_pos;
            } else {
                symbol_table[label_count].address = (i32)virtual_program_pos;
            }
            label_count++;
            free(token);
        } else if (!in_data_section) {
            // check if it's an opcode (not in data section)
            for (i32 i = 0; i < OPCODE_COUNT; ++i) {
                if (strcmp(token, ASSEMBLY_TABLE[i].name) == 0) {
                    virtual_program_pos += 1;  // opcode itself
                    virtual_program_pos += ASSEMBLY_TABLE[i].arg_count;  // arguments

                    // skip the arguments in tokenization
                    for (int j = 0; j < ASSEMBLY_TABLE[i].arg_count; ++j) {
                        char *arg = tokenizer_next(&t1);
                        free(arg);
                    }
                    break;
                }
            }
            free(token);
        } else if (in_data_section) {
            // in data section but not a label should be the string or integer value
            if (token[0] == '"') {
                // we found a string count its length (without quotes)
                size_t i = 1;
                while (token[i] != '"' && token[i] != '\0') {
                    virtual_data_pos++;
                    i++;
                }
                virtual_data_pos++;  // for null terminator
            } else if (token[0] != '.' && token[0] != '@') {
                // try parsing as integer
                char *endptr;
                strtol(token, &endptr, 10);
                
                if (*endptr == '\0') {
                    // valid integer literal, takes 1 slot
                    virtual_data_pos++;
                }
            }
            free(token);
        }
    }

    i32 data_size = (i32)virtual_data_pos;

    // ========== PASS 2: Load data section ==========
    in_data_section = false;
    Tokenizer t2 = tokenizer_create(buffer);

    while ((token = tokenizer_next(&t2)) != NULL) {
        if (token[0] == '#') {
            free(token);
            while (t2.buffer[t2.pos] != '\0' && t2.buffer[t2.pos] != '\n') {
                t2.pos++;
            }
            continue;
        }

        if (strcmp(token, ".data") == 0) {
            in_data_section = true;
            free(token);
            continue;
        }

        if (strcmp(token, ".text") == 0) {
            in_data_section = false;
            free(token);
            continue;
        }

        size_t len = strnlen(token, max_limit);

        // skip labels in data section (length > 1 and ends with ':')
        if (in_data_section && len > 1 && token[len - 1] == ':') {
            free(token);
            continue;
        }

        // process string data
        if (in_data_section && token[0] == '"') {
            // store string as ASCII values (excluding quotes)
            size_t i = 1;
            while (token[i] != '"' && token[i] != '\0') {
                data_buffer[data_head++] = (i32)token[i];
                i++;
            }
            
            data_buffer[data_head++] = 0;  // null terminator
            free(token);
            continue;
        }
        
        // process integer literals (no quotes)
        if (in_data_section && token[0] != '"' && token[0] != '.' && token[0] != '@') {
            char *endptr;
            i32 val = (i32)strtol(token, &endptr, 10);
            
            if (*endptr == '\0') {
                // valid integer literal
                data_buffer[data_head++] = val;
                free(token);
                continue;
            }
        }

        free(token);
    }

    // ========== PASS 3: Generate program bytecode ==========
    in_data_section = false;
    Tokenizer t3 = tokenizer_create(buffer);

    while ((token = tokenizer_next(&t3)) != NULL) {
        if (token[0] == '#') {
            free(token);
            while (t3.buffer[t3.pos] != '\0' && t3.buffer[t3.pos] != '\n') {
                t3.pos++;
            }
            continue;
        }

        // handle section markers FIRST before anything else
        if (strcmp(token, ".data") == 0) {
            in_data_section = true;
            free(token);
            continue;
        }

        if (strcmp(token, ".text") == 0) {
            in_data_section = false;
            free(token);
            continue;
        }

        // if we're in data section, skip everything
        if (in_data_section) {
            size_t len = strnlen(token, max_limit);
            
            // skip label (length > 1, ends with ':') and its string value
            if (len > 1 && token[len - 1] == ':') {
                free(token);
                continue;
            }
            
            free(token);
            continue;
        }

        size_t len = strnlen(token, max_limit);

        // skip program labels (length > 1, ends with ':')
        if (len > 1 && token[len - 1] == ':') {
            free(token);
            continue;
        }

        bool found_opcode = false;

        // instructions
        for (i32 i = 0; i < OPCODE_COUNT; ++i) {
            if (strcmp(token, ASSEMBLY_TABLE[i].name) == 0) {
                found_opcode = true;
                program_buffer[program_head++] = i;

                const char *current_instr_name = ASSEMBLY_TABLE[i].name;

                for (int j = 0; j < ASSEMBLY_TABLE[i].arg_count; ++j) {
                    char *arg_token = tokenizer_next(&t3);

                    if (arg_token == NULL) {
                        fprintf(stderr, "Error: missing argument for '%s'\n", current_instr_name);
                        exit(1);
                    }

                    // registers
                    if (ASSEMBLY_TABLE[i].arg_types[j] == ARG_REG) {
                        if (arg_token[0] == '%' || arg_token[0] == '$') {

                            const char *reg_name = arg_token + 1;  // skip prefix

                            char *endptr;
                            int reg_idx = (int)strtol(reg_name, &endptr, 10);

                            if (*endptr == '\0') {
                                // is a number
                                if (reg_idx >= 0 && reg_idx < REG_COUNT) {
                                    program_buffer[program_head++] = reg_idx;
                                } else {
                                    fprintf(stderr, "Error: invalid register index %d\n", reg_idx);
                                    exit(1);
                                }

                            } else {
                                // not a number, search for named registers
                                bool reg_found = false;

                                for (size_t r = 0; r < NAMED_REGISTER_COUNT; ++r) {
                                    if (strcmp(reg_name, NAMED_REGISTERS[r].name) == 0) {
                                        program_buffer[program_head++] = NAMED_REGISTERS[r].reg_idx;
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
                            fprintf(stderr, "Error: expected '$' for register, found '%s'\n", arg_token);
                            exit(1);
                        }
                    }
                    // values or labels
                    else if (ASSEMBLY_TABLE[i].arg_types[j] == ARG_VAL) {

                        // program labels start with '.'
                        if (arg_token[0] == '.') {
                            const char *label_to_find = arg_token + 1;
                            bool label_found = false;

                            for (int l = 0; l < label_count; ++l) {
                                if (strcmp(label_to_find, symbol_table[l].name) == 0 && !symbol_table[l].is_data_label) {
                                    program_buffer[program_head++] = symbol_table[l].address;
                                    label_found = true;
                                    break;
                                }
                            }

                            if (!label_found) {
                                fprintf(stderr, "Error: Label '%s' not defined.\n", label_to_find);
                                exit(1);
                            }
                        }
                        // data references start with '@'
                        else if (arg_token[0] == '@') {
                            const char *label_to_find = arg_token + 1;
                            bool label_found = false;

                            for (int l = 0; l < label_count; ++l) {
                                if (strcmp(label_to_find, symbol_table[l].name) == 0 && symbol_table[l].is_data_label) {
                                    program_buffer[program_head++] = symbol_table[l].address;
                                    label_found = true;
                                    break;
                                }
                            }

                            if (!label_found) {
                                fprintf(stderr, "Error: Data label '%s' not defined.\n", label_to_find);
                                exit(1);
                            }
                        }
                        // literal number
                        else {
                            char *endptr;
                            i32 val = (i32)strtol(arg_token, &endptr, 10);

                            if (*endptr == '\0') {
                                program_buffer[program_head++] = val;
                            } else {
                                fprintf(stderr, "Error: Expected numeric value, .label, or @data_label, found '%s'\n", arg_token);
                                exit(1);
                            }
                        }
                    }
                    
                    free(arg_token);
                }
                break;
            }
        }

        if (!found_opcode) {
            fprintf(stderr, "Error: unknown instruction or stray token '%s'\n", token);
            exit(1);
        }

        free(token);
    }

    // ========== Write output file with header ==========
    FILE *output_file = fopen(output_path, "wb");

    if (!output_file) {
        perror("Failed to open output file.");
        exit(1);
    }

    VMFileHeader header = {
        .magic = VM_MAGIC,
        .version = VM_VERSION,
        .data_size = data_size,
        .program_start = (i32)data_head
    };

    fwrite(&header, sizeof(VMFileHeader), 1, output_file);
    fwrite(data_buffer, sizeof(i32), data_head, output_file);
    fwrite(program_buffer, sizeof(i32), program_head, output_file);

    fclose(output_file);
    free(buffer);

    /*
    printf("Assembled successfully!\n");
    printf("  Data size: %d bytes (%zu i32s)\n", data_size, data_head);
    printf("  Program size: %zu i32s\n", program_head);
    printf("  Total size: %zu i32s\n", data_head + program_head);
    printf("  Output: %s\n", output_path);

    */

    if (run_flag) run_after_compile(output_path);

    return 0;
}
