.PHONY: clear clean

CC := gcc
CFLAGS := -Wall -Wextra -static -std=c11 -ggdb

GLOBAL_DEPS = src/spec.h

all: vm disasm vmasm


# VM
vm: build/vm/vm.o build/vm/opcodes.o
	${CC} $^ -o vm ${CFLAGS}

build/vm/vm.o: src/vm/main.c src/vm/opcodes.h ${GLOBAL_DEPS}
	${CC} -c src/vm/main.c -o build/vm/vm.o ${CFLAGS}


build/vm/opcodes.o: src/vm/opcodes.c src/vm/opcodes.h ${GLOBAL_DEPS}
	${CC} -c src/vm/opcodes.c -o build/vm/opcodes.o ${CFLAGS}


# DISASSEMBLER

disasm: build/disassemble/main.o 
	${CC} $^ -o disasm ${CFLAGS}

build/disassemble/main.o: src/disassemble/main.c ${GLOBAL_DEPS}
	${CC} -c src/disassemble/main.c -o build/disassemble/main.o ${CFLAGS}


## ASSEMBLER

vmasm: build/assemble/main.o 
	${CC} $^ -o vmasm ${CFLAGS}

build/assemble/main.o: src/assembler/main.c ${GLOBAL_DEPS}
	${CC} -c src/assembler/main.c -o build/assemble/main.o ${CFLAGS}

# MISC

clean: clear

clear:
	rm -f dumped-program.obj compiled.obj out.obj disassembled.asm vm vmasm disasm build/vm/*.o build/disassemble/*.o build/assemble/*.o
