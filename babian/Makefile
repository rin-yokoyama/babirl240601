ALL : babian babianpull

include ../common.mk
LOADLIBES = ../lib/libbabirl.a

#CFLAGS += -g -DDEBUG

babian.o : $(LOADLIBES)

babian : babian.o

babianpull : babianpull.o

clean : 
	rm -f *.o
