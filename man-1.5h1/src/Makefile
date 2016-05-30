
#
# Those are the compilations options for having the best performance
#
# If NATIVE is defined, the native gcc compiler is used. The same otimization options are used
# 
#
# OPTS    := -O2
# OPTS += -fomit-frame-pointer
# OPTS += -fno-exceptions
# OPTS += -fno-strict-aliasing
# OPTS += -finline-limit=1800 
# OPTS    += -frename-registers -finline-functions -funroll-loops
# OPTS    += -fgcse -fgcse-lm -fgcse-sm

# Added by Hebi Li
OPTS := -g -O1 # if I use O0, the bug will not occur!

DECLS   += -DSTDC_HEADERS -DPOSIX -DDO_COMPRESS -DGREPSILENT=\'q\'


CFLAGS= $(OPTS)
CC    = gcc

OBJS	= man.o manfile.o manpath.o man-config.o man-getopt.o \
	   to_cat.o different.o gripes.o glob.o util.o msg.o


.PHONY: man

all: man

man: $(OBJS)
	$(CC) $(MOPTS) -o $@ $^ $(MLOPTS)
	rm -rf *.o

.c.o:
	$(CC) $(CFLAGS) $(INCLS) $(DECLS) -c $<


clean:
	rm -rf *.s *.o *.c.t* ttt.* ttt 
