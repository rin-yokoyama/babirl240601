ALL : babiexrelay

include ../common.mk
#BABILIB = ../lib/libbabirl.a
LOADLIBES =  -lpthread

#babiexrelay.o : babiexrelay.h ../include/*.h ${BABILIB}

libbabies.o : libbabies.h

babiexrelay.o : 

babiexrelay : babiexrelay.o libbabies.o


clean : 
	rm -f *.o

