.PHONY: clear clean

CC := gcc
CFLAGS := -Wall -Wextra -std=c11 -ggdb

all: main

main: build/main.o build/opcodes.o
	${CC} $^ -o main ${CFLAGS}


build/main.o: src/main.c src/spec.h src/opcodes.h
	${CC} -c src/main.c -o build/main.o ${CFLAGS}


build/opcodes.o: src/opcodes.c src/spec.h src/opcodes.h
	${CC} -c src/opcodes.c -o build/opcodes.o ${CFLAGS}


clean: clear


clear:
	rm main build/*.o dumped-program.obj
