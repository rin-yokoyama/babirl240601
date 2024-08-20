ALL : babiscr

include ../common.mk
#CFLAGS = -Wall -O2 -I../include -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE
LOADLIBES = ../lib/libbabirl.a

babiscr.o : $(LOADLIBES) scr.c ../include/*.h

libnbbqvio.o : libnbbqvio.c

babiscr : babiscr.o libnbbqvio.o

clean : 
	rm -f *.o
