ALL : babits

include ../common.mk
CFLAGS += -DDEBUG -g
BABILIB = ../lib/libbabirl.a
LOADLIBES = ${BABILIB}

babits.o : babits.h ../include/*.h ${BABILIB}

babits : babits.o

clean : 
	rm -f *.o

