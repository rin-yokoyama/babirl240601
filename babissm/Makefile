ALL : babissm xpssm gtossm

include ../common.mk
BABILIB = ../lib/libbabirl.a
LOADLIBES = ${BABILIB}

babissm.o : babissm.h ../include/*.h ${BABILIB}

babissm : babissm.o

xpssm.o : ../include/*.h

xpssm : xpssm.o ../lib/bi-tcp.o

gtossm : gtossm.o ../lib/bi-tcp.o

clean : 
	rm -f *.o *.o
