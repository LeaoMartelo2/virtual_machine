#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../spec.h"

void disassemble_data_section(i32 *buffer, i32 data_size, FILE *out, bool clean) {
    if (data_size <= 0) return;
    
    if(!clean)
        fprintf(out, "\n=== DATA SECTION (%d bytes) ===\n\n", data_size);
    
    i32 i = 0;
    while (i < data_size) {

        if (clean) { fprintf(out, "# "); }
        fprintf(out, "0x%04x(%4d): ", i, i);
        if (clean) {  fprintf(out, "\n"); }

        
        fprintf(out, "%4d", buffer[i]);
        if(buffer[i] >= 32 && buffer[i] < 127) { // printable ascii range
            fprintf(out, " # '%c'", (char)buffer[i]);
        }
        fprintf(out, "\n");
        i++;

    }
    fprintf(out, "\n");
}

void disassemble_program(i32 *buffer, i32 program_start, size_t count, FILE *out, bool clean) {

    size_t pc = program_start;

    if (!clean) {
        fprintf(out, "\n=== PROGRAM SECTION (offset %d) ===\n\n", program_start);
    }

    while (pc < count) {

        i32 opcode_value = buffer[pc];

        if (opcode_value < 0 || opcode_value > OPCODE_COUNT) {
            if (!clean) fprintf(out, "0x%08zx(%zu): [!] UNKNOWN OPCODE: %d\n", pc, pc, opcode_value);
            pc++;
            continue;
        }

        const Instruction_spec *spec = &ASSEMBLY_TABLE[opcode_value];

        if (pc + spec->arg_count >= count) {
            if (!clean) fprintf(out, "0x%08zx: [!] ERROR: Incomplete instruction '%s' at the end of the file\n", pc, spec->name);
            break;
        }

        if (!clean) {
            fprintf(out, "0x%04zx(%4zu): %-15s", pc, pc, spec->name);
        } else {
            fprintf(out, "%s", spec->name);
        }

        for (int i = 0; i < spec->arg_count; i++) {
            i32 val = buffer[pc + 1 + i];

            if (spec->arg_types[i] == ARG_REG) {
                fprintf(out, " $%d", val);
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

_Noreturn void version_info(void){
    printf("VMASM disassembler:\n");
    printf("Spec version:           %d\n", VM_VERSION);
    printf("Expected Magic Bytes:   %#x\n", VM_MAGIC);
    printf("Build:                  %s  \n", GIT_HASH);
    printf("Build date:             %s   \n", BUILD_DATE);
    printf("Compiled with %s\n", COMPILER_NAME);
    exit(0);
}

_Noreturn void print_help(char **argv) {
    printf("Usage: %s <program.bin> [flags]\n", argv[0]);
    printf("        -s                 Output disassembled code to 'disassembled.asm'.\n");
    printf("        -V, -version       Print version information and exit.\n");
    printf("        -h, -help          Prints this message\n");
    exit(1);
}

int main(int argc, char **argv) {

    if (argc < 2) {
        printf("Usage: %s <obj file> [-s]\n", argv[0]);
        printf("Use -help for more info.\n");
        exit(1);
    }

    bool save_to_file = false;
    const char *input_path = NULL;

    for (int i = 1; i < argc; ++i) {

        if (strcmp(argv[i], "-s") == 0) { save_to_file = true; }

        if(strcmp(argv[i], "-V") == 0 || strcmp(argv[i], "-version") == 0) { version_info(); }

        if(strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "-help") == 0) { print_help(argv); }

        if (argv[i][0] != '-') {
            /* last argument without -s is input path */
            input_path = argv[i];
        }
    }

    if (!input_path) {
        printf("Error: No input file specified.\n");
        exit(1);
    }

    FILE *file = fopen(input_path, "rb");
    if (!file) {
        perror("Error opening file");
        exit(1);
    }

    // ========== read header ==========
    VMFileHeader header;
    if (fread(&header, sizeof(VMFileHeader), 1, file) != 1) {
        fprintf(stderr, "Error: Failed to read file header.\n");
        fclose(file);
        exit(1);
    }

    if (header.magic != VM_MAGIC) {
        fprintf(stderr, "Error: Invalid file format (bad magic bytes).\n");
        fclose(file);
        exit(1);
    }

    // ========== load everything ==========
    i32 read_object[MAX_PROGRAM_SIZE];
    size_t read_size = fread(read_object, sizeof(i32), MAX_PROGRAM_SIZE, file);
    fclose(file);

    printf("==========================================================\n");
    printf("Header: \n");
    printf("    Magic: %x ('", header.magic);

    for(int i = 0; i <4 ; i++) {
        char c = (header.magic >> (i * 8)) & 0xFF;
        printf("%c", c);
    }
    printf("')\n");

    printf("    Spec format %d\n", header.version);
    printf("    Data section size: %d bytes\n", header.data_size);
    printf("    Program offset %d\n", header.program_start);
    printf("==========================================================\n");

    // disassemble
    if (header.data_size > 0) {
        disassemble_data_section(read_object, header.data_size, stdout, false);
    }
    disassemble_program(read_object, header.program_start, read_size, stdout, false);

    printf("==========================================================\n");

    if (save_to_file) {
        FILE *f_asm = fopen("disassembled.asm", "w");
        if (f_asm) {
            fprintf(f_asm, ".data\n");
            if (header.data_size > 0) {
                fprintf(f_asm, "# ===  WARNING ====\n# .data section disassembly not fully suported yet\n\n");
                disassemble_data_section(read_object, header.data_size, f_asm, true);
            }
            fprintf(f_asm, "\n.text\n");
            disassemble_program(read_object, header.program_start, read_size, f_asm, true);
            fclose(f_asm);
            printf("\nWrote 'disassembled.asm' successfully.\n");
        } else {
            perror("Error opening output file");
        }
    }

    return 0;
}
