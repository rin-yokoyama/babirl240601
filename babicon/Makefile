ALL : babicon babicmd babicmdjson babiconjson

include ../common.mk
LOADLIBES = ../lib/libbabirl.a
LOADLIBES += -lreadline -lncurses -lexpat

babicon.o : babicon.h ../include/*.h ../lib/libbabirl.a comfuncs.c
babicmd.o : babicon.h ../include/*.h ../lib/libbabirl.a comfuncs.c
babiconjson.o : babicon.h ../include/*.h ../lib/libbabirl.a comfuncsjson.c
babicmdjson.o : babicon.h ../include/*.h ../lib/libbabirl.a comfuncsjson.c

babicon : babicon.o ../lib/bbxml.o

babicmd : babicmd.o ../lib/bbxml.o

babiconjson : babiconjson.o ../lib/bbxml.o ../lib/bbjson.o

babicmdjson : babicmdjson.o ../lib/bbxml.o ../lib/bbjson.o

clean : 
	rm -f *.o
