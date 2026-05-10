CC := gcc

all: runner.c 
	gcc runner.c -o runner
	gcc libtest.c -o libtest.so -shared -fPIC

tests: all
	./runner

