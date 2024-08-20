include ../common.mk


ifdef USEDB
DBOBJ = bbpsql.o
else
DBOBJ = 
endif

ALL : libbabirl.a bbxml.o $(DBOBJ) bbzfile.o bbterm.o bbjson.o

LDFLAGS = 

OBJ = bi-tcp.o bi-shm.o bi-com.o bi-time.o bbcpri.o bi-mem.o
OBJ += bi-file.o bi-pid.o ridf.o rdf.o bi-sim.o bi-log.o

$(OBJ) : ../include/*.h

libbabirl.a : $(OBJ)
	ar rv libbabirl.a $(OBJ)
	ranlib libbabirl.a

bbxml.o : 

ifdef USEDB
bbpsql.o : ../include/bbpsql.h
endif

bbjson.o : ../include/bbjson.h

clean : 
	rm -f libbabirl.a *.o
