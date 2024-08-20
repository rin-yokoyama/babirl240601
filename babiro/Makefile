TARGET  = TSHisto TSHistoOL
OBJS    = TSHisto.o TSHistoOL.o

ROOTCFLAGS := $(shell root-config --cflags)
ROOTGLIBS  := $(shell root-config --glibs)

CXXFLAGS    = -w $(ROOTCFLAGS) -O2 -Wall -g
CXXLIBS     = $(ROOTGLIBS)

all: $(TARGET)

TSHisto: TSHisto.cpp
	rootcint -f TSHistoDict.cpp -c TSFrame.h TSHistoLinkDef.h 
	g++ $(CXXFLAGS)  -o TSHisto TSHisto.cpp TSHistoDict.cpp $(CXXLIBS)

TSHistoOL: TSHistoOL.cpp
	rootcint -f TSHistoDict.cpp -c TSFrame.h TSHistoLinkDef.h 
	g++ $(CXXFLAGS)  -o TSHistoOL TSHistoOL.cpp TSHistoDict.cpp $(CXXLIBS)


clean:
	rm -f $(TARGET) $(OBJS) *~
