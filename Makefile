.PHONY: clear clean

CC := gcc
CFLAGS := -Wall -Wextra -static -std=c11 -ggdb

all: vm disassemble


# VM
vm: build/vm/vm.o build/vm/opcodes.o
	${CC} $^ -o vm ${CFLAGS}

build/vm/vm.o: src/vm/main.c src/spec.h src/vm/opcodes.h
	${CC} -c src/vm/main.c -o build/vm/vm.o ${CFLAGS}


build/vm/opcodes.o: src/vm/opcodes.c src/spec.h src/vm/opcodes.h
	${CC} -c src/vm/opcodes.c -o build/vm/opcodes.o ${CFLAGS}


# DISASSEMBLER

disassemble: build/disassemble/main.o 
	${CC} $^ -o disassemble ${CFLAGS}

build/disassemble/main.o: src/disassemble/main.c src/spec.h
	${CC} -c src/disassemble/main.c -o build/disassemble/main.o ${CFLAGS}


# MISC

clean: clear

clear:
	rm dumped-program.obj
	rm dumped-program-RAW.obj
	rm vm 
	rm build/vm/*.o
