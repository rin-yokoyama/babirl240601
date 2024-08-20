ALL : babimo babimocli

include ../common.mk
LOADLIBES = ${BABILIB}
#CFLAGS += -DDEBUG -g

babimo.o : babimo.h ../include/libbabirl.h

babimocli.o : ../include/libbabirl.h

babimo : babimo.o ../lib/bi-file.o ../lib/bi-pid.o ../lib/bi-tcp.o

babimocli : babimocli.o ../lib/bi-tcp.o

clean : 
	rm -f *.o
