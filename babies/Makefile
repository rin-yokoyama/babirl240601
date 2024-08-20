ALL : babies

include ../common.mk
#LOADLIBES = ../lib/libbabirl.a -lpthread
LOADLIBES =  ../lib/bi-pid.o ../lib/bi-tcp.o ../lib/bi-shm.o
#CFLAGS += -I/usr/rtlinux/include

babies.o : babies.h ../include/*.h  ../lib/bi-pid.o ../lib/bi-tcp.o ../lib/bi-shm.o

babies : babies.o

clean : 
	rm -f *.o
