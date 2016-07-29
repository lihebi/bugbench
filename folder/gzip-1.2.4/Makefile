
#
# Those are the compilations options for having the best performance
#
# If NATIVE is defined, the native gcc compiler is used. The same otimization options are used
# 
OPTS    := -O2 
OPTS    += -fomit-frame-pointer -fno-exceptions -fno-strict-aliasing -finline-limit=1800 
OPTS    += -frename-registers -finline-functions -funroll-loops
OPTS    += -fgcse -fgcse-lm -fgcse-sm 
# -freduce-all-givs -fmove-all-movables 
CFLAGS= $(OPTS)
CC    = gcc

OBJS	= bits.o deflate.o gzip.o getopt.o inflate.o lzw.o \
	  crypt.c trees.o unlzh.o unlzw.o unpack.o unzip.o util.o zip.o


.PHONY: gzip

all: gzip

gzip: $(OBJS)
	$(CC) $(MOPTS) -o $@ $^ $(MLOPTS)

.c.o:
	$(CC) $(CFLAGS) $(INCLS) $(DECLS) -c $<


clean:
	rm -rf *.s *.o *.c.t* ttt.* ttt
