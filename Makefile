DEPEN_LIBCDS_INC=../libcds/includes/
DEPEN_LIBCDS=../libcds/lib/libcds.a

CC=g++
DEBUG = -g -DDEBUG
CFLAGS=-c -Wall -I$(DEPEN_LIBCDS_INC) -O3
LDFLAGS=-Wall $(DEPEN_LIBCDS) -lm

EXT=

SOURCES=tgs.cpp arraysort.cpp
OBJECTS=$(SOURCES:.cpp=.o)

MAINSRC=use.cpp create.cpp benchmark.cpp
MAINOBJ=$(MAINSRC:.cpp=.o)

EXECUTABLE=create use benchmark

#all: CFLAGS += -O9
all: $(SOURCES) $(MAINSRC) $(MAINOBJ) $(OBJECTS) $(EXECUTABLE) 

debug: CPP += $(DEBUG)
debug: clean all

#$(EXECUTABLE): $(MAINOBJ) $(OBJECTS)
#	$(CC) $(LDFLAGS) $(OBJECTS) $(DEPENOBJS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean: cleanobj cleanexe

cleanobj:
	rm -f $(OBJECTS) $(MAINOBJ)

cleanexe:
	rm -f $(EXECUTABLE)

create: create.o
	$(CC) $(OBJECTS) $(DEPENOBJS) $< -o $@$(EXT) $(LDFLAGS)

use: use.o tgs.o
	$(CC) $(OBJECTS) $(DEPENOBJS) $< -o $@$(EXT) $(LDFLAGS)

benchmark: benchmark.o
	$(CC) $(OBJECTS) $(DEPENOBJS) $< -o $@$(EXT) $(LDFLAGS)
