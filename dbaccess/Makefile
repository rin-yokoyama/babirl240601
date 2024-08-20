ALL : stdindb inctimereset expdbcom

include ../common.mk

LOADLIBES += -lpq
#CFLAGS += -g -DDEBUG

stdindb : stdindb.o ../lib/bbpsql.o

inctimereset : inctimereset.o ../lib/bbpsql.o

expdbcom : expdbcom.o expdbcomfunc.o ../lib/bbpsql.o

clean : 
	rm -f *.o
