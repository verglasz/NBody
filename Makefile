
OBJDIR := build
SOURCEDIR := src
INCLUDEDIR := include
SHADERDIR := shaders
TEXDIR := report
BENCHDIR := perf/data
HOST := $(shell hostname)


CC := gcc
libs := glfw GL X11 pthread Xrandr Xi dl m
LIBS := $(libs:%=-l%)
CFLAGS += -I$(INCLUDEDIR) -I$(SHADERDIR) -Wall -Wextra -O2 $(LIBS)

TEXFLAGS := -output-directory=$(TEXDIR) -interaction=nonstopmode -shell-escape

ifneq ($(HOST),boron)
CFLAGS += -lrt -std=gnu99
endif

ifneq ($(debug),)
CFLAGS += -DDEBUG -g3
else
CFLAGS += -g
endif

objs := common grav graphics glad
OBJECTS := $(objs:%=$(OBJDIR)/%.o)
SHADERS := $(wildcard $(SHADERDIR)/*.gl)

.PHONY: singlethreaded multithreaded report bench clean cleanbench cleanall

all: singlethreaded multithreaded


#reproducibility
ifneq ($(seed),)
CFLAGS += -DRAND_SEED=$(seed)
else
bench: CFLAGS += -DRAND_SEED=42
endif

bench: clean perf/bench.sh perf/data
	$(MAKE) all

report: $(TEXDIR)/report.pdf

objs: $(OBJECTS)  $(OBJDIR)/multi-quadratic.o  $(OBJDIR)/single-quadratic.o $(OBJDIR)/single-barneshut.o $(OBJDIR)/multi-barneshut.o

$(TEXDIR)/%.pdf: $(TEXDIR)/%.tex
	lualatex $(TEXFLAGS) $<

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

$(OBJDIR)/%.o: $(SOURCEDIR)/%.c $(INCLUDEDIR)/%.h
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/glad.o: $(SOURCEDIR)/glad.c $(INCLUDEDIR)/glad/glad.h
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/graphics.o: $(SOURCEDIR)/graphics.c $(INCLUDEDIR)/graphics.h $(SHADERS)
	$(CC) $(CFLAGS) -c -o $@ $<

