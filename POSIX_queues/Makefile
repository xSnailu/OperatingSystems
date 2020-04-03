CC=gcc
CFLAGS= -std=gnu99 -Wall
LDLIBS = -lpthread -lrt

all: prog1
prog1: prog1.c	
	gcc -Wall -o prog1 prog1.c -lrt
.PHONY: clean all
clean:
	rm prog1
