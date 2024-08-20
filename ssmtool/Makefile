ALL : ssmrpv130 pulserpv130

include ../common.mk
CFLAGS = -Wall -O2 -I../include -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE
LOADLIBES = ../lib/libbabirl.a

ssmrpv130.o : $(LOADLIBES) ../include/*.h

libnbbqvio.o : libnbbqvio.c

ssmrpv130 : ssmrpv130.o libnbbqvio.o

pulserpv130 : pulserpv130.o libnbbqvio.o

clean : 
	rm -f *.o
