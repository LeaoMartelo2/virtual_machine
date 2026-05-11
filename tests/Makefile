CC := gcc

all: 
	gcc runner.c -o runner
	gcc libtest.c -o libtest.so -shared -fPIC
	../vmasm exceptions/unknown_instruction_pre.asm -o exceptions/unknown_instruction.bin
	gcc binpatch.c -o binpatch
	./binpatch


tests: all
	./runner
clear:
	rm -f out.bin VM.dump runner libtest.so crash_log.txt binpatch
