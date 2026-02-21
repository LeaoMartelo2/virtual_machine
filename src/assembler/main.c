#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "../spec.h"

int main(int argc, char **argv) {

    char *input_path = NULL;
    // default to out.obj if not specified
    char *output_path = "out.obj";

    if(argc < 2) {
        printf("Usage: %s <input file> [-o <output>]\n", argv[0]);
        printf("Options:\n  -o: specify output path, otherwise 'out.obj'\n");
        exit(1);
    }

    for(int i = 1; i < argc; ++i) {
        if(strcmp(argv[i], "-o") == 0) {
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
    if(!file) {
        perror("Failed to open input file.");
        exit(1);
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = malloc(file_size + 1);
    if(!buffer) {
        perror("malloc failed lmaoooo");
        fclose(file);
        exit(1);
    }

    fread(buffer, 1, file_size, file);
    buffer[file_size] = '\0';
    fclose(file);

    char *token = strtok(buffer, " \t\n\r");

    while(token != NULL) {
        printf("%s\n", token);

        token = strtok(NULL, " \t\n\r");
    }

    free(buffer);



    return 0;
}
