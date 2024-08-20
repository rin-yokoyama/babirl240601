ALL: csample csamplets bigsample passample

CFLAGS = -Wall -O2
LOADLIBES = -lpthread

libbabies.o : libbabies.h

csample : csample.o libbabies.o

csamplets : csamplets.o libbabies.o

bigsample : bigsample.o libbabies.o

passample : passample.o libbabies.o

clean :
	rm -f *.o
