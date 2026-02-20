.PHONY: clear clean

CC := gcc
CFLAGS := -Wall -Wextra -static -std=c11 -ggdb

all: vm

vm: build/vm.o build/opcodes.o
	${CC} $^ -o vm ${CFLAGS}


build/vm.o: src/vm/main.c src/spec.h src/vm/opcodes.h
	${CC} -c src/vm/main.c -o build/vm.o ${CFLAGS}


build/opcodes.o: src/vm/opcodes.c src/spec.h src/vm/opcodes.h
	${CC} -c src/vm/opcodes.c -o build/opcodes.o ${CFLAGS}


clean: clear


clear:
	rm vm build/*.o dumped-program.obj
