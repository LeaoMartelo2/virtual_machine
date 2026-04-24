.PHONY: clear clean

CC := gcc
CFLAGS := -Wall -Wextra -Wswitch-enum -Wno-override-init -static -ldl -std=c11 -ggdb

VM_VERSION := $(shell expr `cksum src/spec.h | cut -f1 -d' '` % 100000 || 00000)
GIT_HASH := $(shell git describe --always --dirty 2>/dev/null || echo "Not a git environment")
BUILD_DATE := $(shell date "+%Y-%m-%d %H:%M:%S" || echo "Unable to get date(?)")

CFLAGS += -DVM_VERSION=$(VM_VERSION)
CFLAGS += -DGIT_HASH='"$(GIT_HASH)"'
CFLAGS += -DBUILD_DATE='"$(BUILD_DATE)"'

GLOBAL_DEPS = src/spec.h
INTERPRETER_DEPS = src/vm/error.h

all: vm disasm vmasm 


# VM
vm: build/vm/vm.o build/vm/opcodes.o
	${CC} $^ -o vm ${CFLAGS}

build/vm/vm.o: src/vm/main.c src/vm/opcodes.h ${GLOBAL_DEPS} ${INTERPRETER_DEPS}
	${CC} -c src/vm/main.c -o build/vm/vm.o ${CFLAGS}


build/vm/opcodes.o: src/vm/opcodes.c src/vm/opcodes.h ${GLOBAL_DEPS} ${INTERPRETER_DEPS}
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

externlib: 
	make -C example_programs/externlib

clean: clear

clear:
	rm -f *.obj *.bin disassembled.asm file.txt index.html RAM.DATA VM.dump crash_log.txt vm vmasm disasm build/vm/*.o build/disassemble/*.o build/assemble/*.o
