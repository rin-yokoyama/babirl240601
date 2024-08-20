ALL : babiau

include ../common.mk
BABILIB = ../lib/libbabirl.a
LOADLIBES = ${BABILIB} -lpthread -lexpat

babiau.o : babiau.h ../include/*.h ${BABILIB}

babiau : babiau.o austrcom.o ../lib/bbxml.o

clean : 
	rm -f *.o

