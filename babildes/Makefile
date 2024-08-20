ALL : babildes

include ../common.mk
BABILIB = ../lib/libbabirl.a
LOADLIBES = ${BABILIB}  -lpthread

babildes.o : babildes.h ../include/*.h ${BABILIB}

babildes : babildes.o 

clean : 
	rm -f *.o

