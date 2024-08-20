# Makefile for babildrv.c

TARGET = babildrv

obj-m = $(TARGET).o
KERNEL_VERSION = $(shell uname -r)
PWD = $(shell pwd)


default: 
	make -C /lib/modules/$(KERNEL_VERSION)/build SUBDIRS=$(PWD) modules


clean:
	rm -f $(TARGET).ko $(TARGET).o $(TARGET).mod.*
