ALL : csvruninfo printncscr prinfo printncscr2 printncscr4 printxmlstatus printncscr5

include ../common.mk
#LOADLIBES = 
#CFLAGS += -I/usr/rtlinux/include
OBJS = ../lib/ridf.o ../lib/bi-file.o ../lib/bbzfile.o

csvruninfo : csvruninfo.o ../lib/ridf.o

# printssmscr : printssmscr.o $(LOADLIBES)

printncscr : printncscr.o ../lib/ridf.o

printncscr2 : printncscr2.o ../lib/ridf.o

printncscr4 : printncscr4.o ../lib/ridf.o
	gcc $(CFLAGS) -o printncscr4 printncscr4.o $(OBJS) -lz

printncscr5 : printncscr5.o ../lib/ridf.o
	gcc $(CFLAGS) -o printncscr5 printncscr5.o $(OBJS) -lz

prinfo : prinfo.o $(OBJS)
	gcc $(CFLAGS) -o prinfo prinfo.o $(OBJS) -lz

printxmlstatus : printxmlstatus.o ../lib/ridf.o
	gcc $(CFLAGS) -o printxmlstatus printxmlstatus.o $(OBJS) -lz

clean : 
	rm -f *.o

