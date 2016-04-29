
#
# Those are the compilations options for having the best performance
#
# If NATIVE is defined, the native gcc compiler is used. The same otimization options are used
# 
#

OPTS    := -g -fomit-frame-pointer -fno-exceptions -fno-strict-aliasing -finline-limit=1800 
OPTS    += -frename-registers -finline-functions -funroll-loops
OPTS    += -fgcse -fgcse-lm -fgcse-sm 

DECLS   += -DDIRENT=1 -DSYSDIR=1 -DLSTAT=1 -DUTIME_H=1 -DUSERMEM=800000 -DREGISTERS=20 -DIBUFSIZ=8192 -DOBUFSIZ=8192 -DBYTEORDER=4321 -DNOALLIGN=1 -DCOMPILE_DATE=\"hello\"

CC    = gcc

CFLAGS= $(OPTS)

OBJS	= compress42.o 


.PHONY: compress

all: compress

compress: $(OBJS)
	$(CC) $(OPTS) -o $@ $^ $(LOPTS)
#	rm -rf *.o

.c.o:
	$(CC) $(CFLAGS) $(INCLS) $(DECLS) -c $<


clean:
	rm -rf *.s *.o *.c.t*
