
OBJDIR := build
SOURCEDIR := src
INCLUDEDIR := include
BENCHDIR := perf/data
HOST := $(shell hostname)

CC := gcc
CFLAGS += -I$(INCLUDEDIR) -Wall -Wextra -O2 -lm

ifneq ($(seed),)
	CFLAGS += -DRAND_SEED=$(seed)
endif

ifneq ($(debug),)
	CFLAGS += -DDEBUG
endif

objs := common grav
OBJECTS := $(objs:%=$(OBJDIR)/\%-%.o)

.PHONY: singlethreaded multithreaded bench clean cleanbench

all: singlethreaded multithreaded

singlethreaded: CFLAGS += -Wno-unknown-pragmas
singlethreaded: single-quadratic single-barneshut

multithreaded: CFLAGS += -fopenmp
multithreaded: multi-quadratic multi-barneshut

bench: all cleanbench

clean:
	rm -rf $(OBJDIR)/*
	rm -rf {single,multi}-{barneshut,quadratic}

cleanbench:
	rm -rf $(BENCHDIR)/$(HOST)-*

%-quadratic: $(OBJDIR)/%-quadratic.o $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

%-barneshut: $(OBJDIR)/%-barneshut.o $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJDIR)/single-%.o: $(SOURCEDIR)/%.c $(INCLUDEDIR)/%.h
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/multi-%.o: $(SOURCEDIR)/%.c $(INCLUDEDIR)/%.h
	$(CC) $(CFLAGS) -c -o $@ $<

