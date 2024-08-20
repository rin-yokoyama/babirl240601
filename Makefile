ALL : all

include ./common.mk

all : 
	make -C lib
	make -C babicon
	make -C babies
	make -C babild
	make -C babinfo
	make -C devtool
	make -C babian
	make -C babimo
	make -C babissm
	make -C babiau
	make -C ridfutil
	make -C babits
ifdef USEDB
	make -C dbaccess
endif
	cp -f babies/babies bin/
	cp -f babimo/babimo bin/
	cp -f babild/babild bin/
	cp -f babicon/babicon bin/
	cp -f babinfo/babinfo bin/
	cp -f babissm/babissm bin/
	cp -f devtool/chkridf bin/
	cp -f devtool/sexecuter bin/
	cp -f devtool/seriacc bin/
	cp -f babiau/babiau bin/
	cp -f babits/babits bin/
	cp -f babian/babian bin/
	cp -f babian/babianpull bin/babianpull
ifdef USEDB
	cp -f dbaccess/stdindb bin/
	cp -f dbaccess/expdbcom bin/
endif


clean : 
	make -C lib clean
	make -C babicon clean
	make -C babies clean
	make -C babild clean
	make -C babinfo clean
	make -C devtool clean
	make -C babian clean
	make -C babimo clean
	make -C babissm clean
	make -C babiau clean
	make -C ridfutil clean
	make -C babits clean
ifdef USEDB
	make -C dbaccess clean
endif

purge : 
	rm -f *~ */*~ */*/*~
