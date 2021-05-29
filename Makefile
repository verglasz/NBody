
OBJDIR := build
SOURCEDIR := src
INCLUDEDIR := include
BENCHDIR := perf/data
HOST := $(shell hostname)

CC := gcc
CFLAGS += -I$(INCLUDEDIR) -Wall -Wextra -O2 -lm

ifneq ($(HOST),boron)
CFLAGS += -lrt -std=gnu99
endif

ifneq ($(debug),)
CFLAGS += -DDEBUG
endif

objs := common grav
OBJECTS := $(objs:%=$(OBJDIR)/\%-%.o)

.PHONY: singlethreaded multithreaded bench clean cleanbench cleanall

all: singlethreaded multithreaded

#reproducibility
ifneq ($(seed),)
CFLAGS += -DRAND_SEED=$(seed)
else
bench: CFLAGS += -DRAND_SEED=42
endif

bench: clean perf/bench.sh perf/data
	$(MAKE) all


perf/data:
	mkdir $@

singlethreaded: CFLAGS += -Wno-unknown-pragmas
singlethreaded: single-quadratic single-barneshut

multithreaded: CFLAGS += -fopenmp
multithreaded: multi-quadratic multi-barneshut

cleanall: clean cleanbench
	rm -rf {single,multi}-{barneshut,quadratic}

clean:
	rm -rf $(OBJDIR)/*

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

