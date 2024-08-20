ALL : babinfo

include ../common.mk
BABILIB = ../lib/libbabirl.a

#CFLAGS += -DDEBUG

ifdef USEDB
LOADLIBES = ${BABILIB} -lpthread -lexpat -lpq
else
LOADLIBES = ${BABILIB} -lpthread -lexpat
endif

babinfo.o : babinfo.h ../include/*.h ${BABILIB} 

ifdef USEDB
babinfodb.o : ../include/bbpsql.h babinfodb.h
babinfo : babinfo.o babinfoxcom.o babinfodb.o ../lib/bbxml.o ../lib/bbpsql.o
else
babinfo : babinfo.o babinfoxcom.o ../lib/bbxml.o
endif

clean : 
	rm -f *.o
