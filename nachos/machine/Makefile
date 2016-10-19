# NOTE: this is a GNU Makefile.  You must use "gmake" rather than "make".

OBJS = ACIA.o ACIA_sysdep.o console.o disk.o interrupt.o	\
       machine.o mipssim.o mmu.o translationtable.o		\
       sysdep.o timer.o

archive.a: $(OBJS)

TOPDIR = ../
include $(TOPDIR)/Makefile.kernel
