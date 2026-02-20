.PHONY: clear clean

CC := gcc
CFLAGS := -Wall -Wextra -static -std=c11 -ggdb

all: vm

vm: build/vm/vm.o build/vm/opcodes.o
	${CC} $^ -o vm ${CFLAGS}

build/vm/vm.o: src/vm/main.c src/spec.h src/vm/opcodes.h
	${CC} -c src/vm/main.c -o build/vm/vm.o ${CFLAGS}


build/vm/opcodes.o: src/vm/opcodes.c src/spec.h src/vm/opcodes.h
	${CC} -c src/vm/opcodes.c -o build/vm/opcodes.o ${CFLAGS}


clean: clear


clear:
	rm dumped-program.obj
	rm dumped-program-RAW.obj
	rm vm 
	rm build/vm/*.o
