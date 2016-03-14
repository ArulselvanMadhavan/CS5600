#Sample Makefile for Malloc
CC=gcc
CFLAGS=-g -O0 -fPIC

all:	check

clean:
	rm -rf libmalloc.so malloc.o

libmymalloc.so: mymalloc.o
	$(CC) $(CFLAGS) -shared -Wl,--unresolved-symbols=ignore-all $< -o $@

test1: test1.o
	$(CC) $(CFLAGS) $< -o $@

# For every XYZ.c file, generate XYZ.o.
%.o: %.c
	$(CC) $(CFLAGS) $< -c -o $@

check:	libmymalloc.so test1
	LD_PRELOAD=`pwd`/libmymalloc.so ./test1

dist:
	dir=`basename $$PWD`; cd ..; tar cvf $$dir.tar ./$$dir; gzip $$dir.tar
unit:
	gcc -Wall memlib.c mymalloc.c unittests.c -o tests -lpthread
tt1:
	gcc -Wall memlib.c mymalloc.c t-test1_modified.c -o tt1 -lpthread
