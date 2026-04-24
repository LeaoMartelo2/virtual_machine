.PHONY: clear clean

CC := gcc
CFLAGS := -shared -fPIC

all: libfirst.so libsecond.so

libfirst.so: firstlib.c
	${CC} firstlib.c -o libfirst.so ${CFLAGS}

libsecond.so: secondlib.c
	${CC} secondlib.c -o libsecond.so ${CFLAGS}


clean: clear

clear:
	rm -f *.so
