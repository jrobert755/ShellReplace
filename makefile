CC = gcc
CFLAGS = -Wall -ggdb
LIBNAMES=termedit
LIBS=$(addsuffix .a,$(addprefix lib,$(LIBNAMES)))
LIBFILES=$(addprefix lib/,$(LIBS))
LIBDIR=lib
LIBINCLUDE=$(addprefix -l,$(LIBNAMES))
HINCLUDE=$(addprefix -I$(LIBDIR)/, $(LIBNAMES))
DESTDIR=bin
EXE=sr
EXESOURCE=main.c

all: $(LIBFILES) $(EXE)

$(LIBFILES): $(LIBNAMES)

$(LIBNAMES):
	$(MAKE) -C lib/$@ $(MAKECMDGOALS)

$(EXE): $(LIBFILES) $(EXESOURCE) | $(DESTDIR)
	$(CC) $(CFLAGS) -L$(LIBDIR) $(HINCLUDE) $(EXESOURCE) $(LIBINCLUDE) -o $(DESTDIR)/$(EXE)

$(DESTDIR):
	mkdir $(DESTDIR)

.PHONY: clean

clean: $(LIBFILES)
	rm $(DESTDIR)/$(EXE)
	
