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
#include <stdarg.h>

#include "../spec.h"

#define RUNNER_NAME "vm"

typedef struct {
    char *value;
    int line;
    int col;
    size_t pos_in_buffer;
} Token;

typedef struct {
    const char *buffer;
    size_t pos;
    int current_line;
    int current_col;
} Tokenizer;


char *strdup_vm(const char *s) {
    if (s == NULL) return NULL;
    size_t len = strnlen(s, MAX_PROGRAM_SIZE) + 1;
    char *result = malloc(len);
    if (result != NULL) {
        memcpy(result, s, len);
    }
    return result;
}

void report_error(const char *filename, const char *buffer, int line, int col, size_t pos, const char *fmt, ...) {
    va_list args;
    // make error text red
    fprintf(stderr, "%s:%d:%d: \033[1;31merror:\033[0m ", filename, line, col);
    
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");

    // find the start of the line
    size_t line_start = pos;
    while (line_start > 0 && buffer[line_start - 1] != '\n') {
        line_start--;
    }

    // find the end of the line
    size_t line_end = pos;
    while (buffer[line_end] != '\0' && buffer[line_end] != '\n' && buffer[line_end] != '\r') {
        line_end++;
    }

    // print the whole line
    fprintf(stderr, " %5d | ", line);
    for (size_t i = line_start; i < line_end; i++) {
        fprintf(stderr, "%c", buffer[i]);
    }
    fprintf(stderr, "\n");

    // print the alignment for the marker
    fprintf(stderr, "       | ");
    for (int i = 0; i < col - 1; i++) {
        fprintf(stderr, " ");
    }
    // make the ^ marker green
    fprintf(stderr, "\033[1;32m^\033[0m\n");
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


Tokenizer tokenizer_create(const char *buffer) {
    return (Tokenizer){buffer, 0, 1, 1};
}

void token_free(Token *t) {
    if (t) {
        free(t->value);
        free(t);
    }
}

Token *tokenizer_next(Tokenizer *t) {
    while (t->buffer[t->pos] != '\0') {
        char c = t->buffer[t->pos];
        
        // @COMMENTS
        if (c == '#') {
            while (t->buffer[t->pos] != '\0' && t->buffer[t->pos] != '\n') {
                t->pos++;
            }
            continue;
        }

        if (c == ' ' || c == '\t' || c == '\r' || c == ',') {
            t->pos++;
            t->current_col++;
            continue;
        }

        if (c == '\n') {
            t->pos++;
            t->current_line++;
            t->current_col = 1;
            continue;
        }

        break;
    }

    if (t->buffer[t->pos] == '\0') return NULL;

    Token *token = malloc(sizeof(Token));
    token->line = t->current_line;
    token->col = t->current_col;
    token->pos_in_buffer = t->pos;
    
    size_t start = t->pos;

    if (t->buffer[t->pos] == '"') {
        t->pos++; t->current_col++;
        while (t->buffer[t->pos] != '\0' && t->buffer[t->pos] != '"') {
            if (t->buffer[t->pos] == '\n') { t->current_line++; t->current_col = 1; }
            else t->current_col++;
            t->pos++;
        }
        if (t->buffer[t->pos] == '"') {
            t->pos++; t->current_col++;
        }
    } else {
        while (t->buffer[t->pos] != '\0' && 
               t->buffer[t->pos] != ' ' && t->buffer[t->pos] != '\t' && 
               t->buffer[t->pos] != '\n' && t->buffer[t->pos] != '\r' && 
               t->buffer[t->pos] != ',' && t->buffer[t->pos] != '#') {
            t->pos++;
            t->current_col++;
        }
    }

    size_t len = t->pos - start;
    token->value = malloc(len + 1);
    memcpy(token->value, t->buffer + start, len);
    token->value[len] = '\0';
    
    return token;
}


int main(int argc, char **argv) {
    char *input_path = NULL;
    char *output_path = "out.bin";
    bool run_flag = false;

    if (argc < 2) {
        printf("Usage: %s <input file> [-o <output>] [-run]\n", argv[0]);
        exit(1);
    }

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-o") == 0) {
            if (i + 1 < argc) output_path = argv[++i];
            else { fprintf(stderr, "Error: -o needs a path.\n"); exit(1); }
        } else if (strcmp(argv[i], "-run") == 0) {
            run_flag = true;
        } else if (argv[i][0] != '-') {
            input_path = argv[i];
        }
    }

    if (!input_path) { fprintf(stderr, "Error: no input file.\n"); exit(1); }

    FILE *file = fopen(input_path, "r");
    if (!file) { perror("Failed to open input file"); exit(1); }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = malloc(file_size + 1);
    fread(buffer, 1, file_size, file);
    buffer[file_size] = '\0';
    fclose(file);

    Label symbol_table[1024];
    int label_count = 0;
    i32 program_buffer[MAX_PROGRAM_SIZE];
    i32 data_buffer[MAX_PROGRAM_SIZE];
    
    size_t program_head = 0, data_head = 0;
    size_t virtual_program_pos = 0, virtual_data_pos = 0;
    bool in_data_section = false;

    // PASS 1: symbol table
    Tokenizer t1 = tokenizer_create(buffer);
    Token *tk;

    while ((tk = tokenizer_next(&t1)) != NULL) {
        if (strcmp(tk->value, ".data") == 0) {
            in_data_section = true;
        } else if (strcmp(tk->value, ".text") == 0) {
            in_data_section = false;
        } else {
            size_t len = strlen(tk->value);
            if (len > 1 && tk->value[len - 1] == ':') {
                char label_name[256];
                strncpy(label_name, tk->value, len - 1);
                label_name[len - 1] = '\0';

                symbol_table[label_count].name = strdup_vm(label_name);
                symbol_table[label_count].is_data_label = in_data_section;
                symbol_table[label_count].address = (i32)(in_data_section ? virtual_data_pos : virtual_program_pos);
                label_count++;
            } else if (!in_data_section) {
                bool found = false;
                for (i32 i = 0; i < OPCODE_COUNT; ++i) {
                    if (strcmp(tk->value, ASSEMBLY_TABLE[i].name) == 0) {
                        virtual_program_pos += 1 + ASSEMBLY_TABLE[i].arg_count;
                        for (int j = 0; j < ASSEMBLY_TABLE[i].arg_count; ++j) {
                            Token *arg = tokenizer_next(&t1);
                            token_free(arg);
                        }
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    report_error(input_path, buffer, tk->line, tk->col, tk->pos_in_buffer, "Unknown instruction or invalid label definition '%s'", tk->value);
                    exit(1);
                }
            } else {
                if (tk->value[0] == '"') {
                    size_t i = 1;
                    while (tk->value[i] != '"' && tk->value[i] != '\0') { virtual_data_pos++; i++; }
                    virtual_data_pos++;
                } else {
                    virtual_data_pos++;
                }
            }
        }
        token_free(tk);
    }

    i32 total_data_size = (i32)virtual_data_pos;
    for(int i = 0; i < label_count; ++i) {
        if(!symbol_table[i].is_data_label) symbol_table[i].address += total_data_size;
    }

    // PASS 2: .data section
    in_data_section = false;
    Tokenizer t2 = tokenizer_create(buffer);
    while ((tk = tokenizer_next(&t2)) != NULL) {
        if (strcmp(tk->value, ".data") == 0) in_data_section = true;
        else if (strcmp(tk->value, ".text") == 0) in_data_section = false;
        else if (in_data_section && tk->value[strlen(tk->value)-1] != ':') {
            if (tk->value[0] == '"') {
                for (size_t i = 1; tk->value[i] != '"' && tk->value[i] != '\0'; i++) 
                    data_buffer[data_head++] = (i32)tk->value[i];
                data_buffer[data_head++] = 0;
            } else {
                char *endptr;
                i32 val = (i32)strtol(tk->value, &endptr, 10);
                data_buffer[data_head++] = val;
            }
        }
        token_free(tk);
    }

    // PASS 3: bytecode
    in_data_section = false;
    Tokenizer t3 = tokenizer_create(buffer);
    while ((tk = tokenizer_next(&t3)) != NULL) {
        if (strcmp(tk->value, ".data") == 0) in_data_section = true;
        else if (strcmp(tk->value, ".text") == 0) in_data_section = false;
        else if (!in_data_section && tk->value[strlen(tk->value)-1] != ':') {
            bool found_opcode = false;
            for (i32 i = 0; i < OPCODE_COUNT; ++i) {
                if (strcmp(tk->value, ASSEMBLY_TABLE[i].name) == 0) {
                    found_opcode = true;
                    program_buffer[program_head++] = i;
                    for (int j = 0; j < ASSEMBLY_TABLE[i].arg_count; ++j) {
                        Token *arg = tokenizer_next(&t3);
                        if (!arg) {
                            report_error(input_path, buffer, tk->line, tk->col, tk->pos_in_buffer, "Instruction '%s' expects %d arguments", tk->value, ASSEMBLY_TABLE[i].arg_count);
                            exit(1);
                        }

                        if (ASSEMBLY_TABLE[i].arg_types[j] == ARG_REG) {
                            if (arg->value[0] == '%' || arg->value[0] == '$') {
                                char *endptr;
                                int reg_idx = (int)strtol(arg->value + 1, &endptr, 10);
                                if (*endptr == '\0') {
                                    if (reg_idx < REG_COUNT) program_buffer[program_head++] = reg_idx;
                                    else { report_error(input_path, buffer, arg->line, arg->col, arg->pos_in_buffer, "Invalid register index %d", reg_idx); exit(1); }
                                } else {
                                    bool r_found = false;
                                    for (size_t r = 0; r < NAMED_REGISTER_COUNT; r++) {
                                        if (strcmp(arg->value+1, NAMED_REGISTERS[r].name) == 0) {
                                            program_buffer[program_head++] = NAMED_REGISTERS[r].reg_idx;
                                            r_found = true; break;
                                        }
                                    }
                                    if (!r_found) { report_error(input_path, buffer, arg->line, arg->col, arg->pos_in_buffer, "Unknown register name '%s'", arg->value); exit(1); }
                                }
                            } else { report_error(input_path, buffer, arg->line, arg->col, arg->pos_in_buffer, "Expected register, got literal or constant"); exit(1); }
                        } else {
                            // ARG_VAL: labels or literals
                            if (arg->value[0] == '.' || arg->value[0] == '@') {
                                bool is_data_ref = (arg->value[0] == '@');
                                bool l_found = false;
                                for (int l = 0; l < label_count; l++) {
                                    if (strcmp(arg->value + 1, symbol_table[l].name) == 0 && symbol_table[l].is_data_label == is_data_ref) {
                                        program_buffer[program_head++] = symbol_table[l].address;
                                        l_found = true; break;
                                    }
                                }
                                if (!l_found) { report_error(input_path, buffer, arg->line, arg->col, arg->pos_in_buffer, "Undefined label '%s'", arg->value); exit(1); }
                            } else {
                                char *endptr;
                                i32 val = (i32)strtol(arg->value, &endptr, 10);
                                if (*endptr == '\0') program_buffer[program_head++] = val;
                                else {
                                    bool c_found = false;
                                    for(size_t c=0; c<CONSTANT_COUNT; c++) {
                                        if(strcmp(arg->value, PREDEFINED_CONSTANTS[c].name) == 0) {
                                            program_buffer[program_head++] = PREDEFINED_CONSTANTS[c].value;
                                            c_found = true; break;
                                        }
                                    }
                                    if(!c_found) { report_error(input_path, buffer, arg->line, arg->col, arg->pos_in_buffer, "Invalid literal or constant '%s'", arg->value); exit(1); }
                                }
                            }
                        }
                        token_free(arg);
                    }
                    break;
                }
            }
            if (!found_opcode) { report_error(input_path, buffer, tk->line, tk->col, tk->pos_in_buffer, "Unexpected token '%s'", tk->value); exit(1); }
        }
        token_free(tk);
    }

    FILE *output_file = fopen(output_path, "wb");
    VMFileHeader header = { .magic = VM_MAGIC, .version = VM_VERSION, .data_size = total_data_size, .program_start = (i32)data_head };
    fwrite(&header, sizeof(VMFileHeader), 1, output_file);
    fwrite(data_buffer, sizeof(i32), data_head, output_file);
    fwrite(program_buffer, sizeof(i32), program_head, output_file);
    fclose(output_file);

    free(buffer);
    for(int i=0; i<label_count; i++) free(symbol_table[i].name);

    if (run_flag) run_after_compile(output_path);

    return 0;
}
