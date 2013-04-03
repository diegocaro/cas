DEPEN_WT=../wt/

CC=gcc
DEBUG = -g -DDEBUG
CFLAGS=-c -Wall -I$(DEPEN_WT) -O9
LDFLAGS=-Wall -lm

EXT=

SOURCES=basic.c bitrankw32int.c tgs.c arraysort.c
OBJECTS=$(SOURCES:.c=.o)

MAINSRC=use.c create.c benchmark.c
MAINOBJ=$(MAINSRC:.c=.o)

OBJS_WT=wt.o bit_array.o
DEPENOBJS=$(addprefix $(DEPEN_WT), $(OBJS_WT))

EXECUTABLE=create use benchmark

#all: CFLAGS += -O9
all: cleanobj wt $(SOURCES) $(MAINSRC) $(MAINOBJ) $(OBJECTS) $(EXECUTABLE) 

debug: CC += $(DEBUG)
debug: clean all

wt: 
	$(MAKE) -C $(DEPEN_WT)

#$(EXECUTABLE): $(MAINOBJ) $(OBJECTS)
#	$(CC) $(LDFLAGS) $(OBJECTS) $(DEPENOBJS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean: cleanobj cleanexe

cleanobj:
	rm -f $(OBJECTS) $(MAINOBJ)

cleanexe:
	rm -f $(EXECUTABLE)

create: create.o
	$(CC) $(OBJECTS) $(DEPENOBJS) $< -o $@$(EXT) $(LDFLAGS)

use: use.o
	$(CC) $(OBJECTS) $(DEPENOBJS) $< -o $@$(EXT) $(LDFLAGS)

benchmark: benchmark.o
	$(CC) $(OBJECTS) $(DEPENOBJS) $< -o $@$(EXT) $(LDFLAGS)
