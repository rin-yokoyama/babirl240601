ALL : babild

include ../common.mk
BABILIB = ../lib/libbabirl.a
LOADLIBES = ${BABILIB}  -lpthread -lexpat -lrdkafka
# -lprofiler


babild.o : babild.h ../include/*.h ${BABILIB}

babildxcom.o : babildxcom.h

babild : babild.o babildxcom.o ../lib/bbxml.o


clean : 
	rm -f *.o

