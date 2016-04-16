CC = gcc
CFLAGS = -Wall

all: rmdup lsdir
rmdup: rmdup.c rmdup.h
	$(CC) rmdup.c -o bin/rmdup $(CFLAGS)
lsdir: lsdir.c
	$(CC) lsdir.c -o bin/lsdir $(CFLAGS)
