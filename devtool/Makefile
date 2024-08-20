ALL : efsfifo1s ddrv rdf2ridf rdf2ridf_seg chkridf chkrdf splitridf \
 srvridf monan extslridf slsrvridf chksumridf slsrvridf curblk \
 dexecuter sexecuter sizeof mkdummyridf seriacc sexecuterts \
 dexecuterts mkdummyts tssorter tscompact segidencorder extblkridf sseriacc \
 fixrunstatridf stsrvridf stclient xmlparse bpidof tcpcli aexecuterscr serclient rawdatapull \
 allsplitridf addevtsegment skipevtsegment combinesegments jsontest mpvtest \
 findbiid anystore

include ../common.mk
LOADLIBES = ../lib/libbabirl.a
#CFLAGS += -I/usr/rtlinux/include
CFLAGS += -g 

findbiid : findbiid.o $(LOADLIBES)
	gcc $(CFLAGS) -o findbiid findbiid.o $(LOADLIBES) -lpthread


efsfifo1s : efsfifo1s.o $(LOADLIBES)

ddrv : ddrv.o $(LOADLIBES)

rdf2ridf : rdf2ridf.o $(LOADLIBES)

rdf2ridf_seg : rdf2ridf_seg.o $(LOADLIBES)

chkridf : chkridf.o $(LOADLIBES) ../lib/bbzfile.o
	gcc $(CFLAGS) -o chkridf chkridf.o $(LOADLIBES) ../lib/bbzfile.o -lz

chkrdf : chkrdf.o $(LOADLIBES) 

srvrdf : srvrdf.o $(LOADLIBES)

monan : monan.o $(LOADLIBES)

splitridf : splitridf.o $(LOADLIBES)

allsplitridf : allsplitridf.o $(LOADLIBES)
	gcc $(CFLAGS) -o allsplitridf allsplitridf.o $(LOADLIBES) ../lib/bbzfile.o -lz

addevtsegment : addevtsegment.o $(LOADLIBES)

skipevtsegment : skipevtsegment.o $(LOADLIBES)

combinesegments : combinesegments.o $(LOADLIBES)

extslridf : extslridf.o $(LOADLIBES)

slsrvrdf : slsrvrdf.o $(LOADLIBES)

chksumridf : chksumridf.o $(LOADLIBES)

slsrvridf : slsrvridf.o $(LOADLIBES)

curblk : curblk.o $(LOADLIBES)

dexecuter : dexecuter.o $(LOADLIBES)

dexecuterts : dexecuterts.o $(LOADLIBES)

aexecuterscr : aexecuterscr.o $(LOADLIBES)

sexecuter : sexecuter.o $(LOADLIBES)

sexecuterts : sexecuterts.o $(LOADLIBES)

serclient : serclient.o $(LOADLIBES)

sizeof : sizeof.o $(LOADLIBES)

tssorter : tssorter.o $(LOADLIBES)

tscompact : tscompact.o $(LOADLIBES)

mkdummyridf : mkdummyridf.o $(LOADLIBES)

extblkridf : extblkridf.o $(LOADLIBES)

mpvtest : mpvtest.o $(LOADLIBES)

mkdummyts : mkdummyts.o $(LOADLIBES) 
	gcc $(CFLAGS) -o mkdummyts mkdummyts.o $(LOADLIBES) -lm

seriacc : seriacc.o

sseriacc : sseriacc.o

segidencorder : segidencorder.o

fixrunstatridf : fixrunstatridf.o

stsrvridf : stsrvridf.o $(LOADLIBES) ../lib/bbzfile.o
	gcc $(CFLAGS) -o stsrvridf stsrvridf.o $(LOADLIBES) ../lib/bbzfile.o -lpthread -lz

stclient : stclient.o 
	gcc $(CFLAGS) -o stclient stclient.o

xmlparse : xmlparse.o ../lib/bbxml.o
	gcc $(CFLAGS) -o xmlparse xmlparse.o ../lib/bbxml.o -lexpat

bpidof : bpidof.o ../lib/bi-pid.o
	gcc $(CFLAGS) -o bpidof bpidof.o ../lib/bi-pid.o

tcpcli : tcpcli.o $(LOADLIBES)

rawdatapull : rawdatapull.o

jsontest : jsontest.o ../lib/bbjson.o

anystore : anystore.o $(LOADLIBES)


clean : 
	rm -f *.o

