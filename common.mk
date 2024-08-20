CC = gcc
AR = ar

# Use of DB
USEDB = 1

# DEBUG mode
#DBMODE = 1

# 32bit compile
#CROSS32 = 1

# if use 2MB buffer
#LARGEBUFF = 1

#if use 8MB buffer
HUGEBUFF = 1

# no daemon for babild/babinfo
#NODAEMON = 1

#if wsl
#NOTOUT = 1

CFLAGS = -Wall -O2 -fcommon -I../include -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE
LDFLAGS =


ifdef LARGEBUFF
CFLAGS +=-DLARGEBUFF
endif

ifdef HUGEBUFF
CFLAGS +=-DHUGEBUFF
endif

ifdef USEDB
CFLAGS += -DUSEDB
endif

ifdef DBMODE
CFLAGS += -DDEBUG -g 
endif

ifdef NODAEMON
CFLAGS += -DNODAEMON 
endif


ifdef CROSS32
#CC = i386-linux26-gcc
#AR = i386-linux26-ar
CFLAGS += -m32
LDFLAGS += -m32
endif

ifdef NOTOUT
CFLAGS += -DNOTOUT
endif
