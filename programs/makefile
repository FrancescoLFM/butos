SOURCES_DIR = sources
PROGRAM_SOURCES = $(SOURCES_DIR)/*
BINDIR = bin
TARGETS = $(BINDIR)/example

.PHONY=all
all:
	for i in $(PROGRAM_SOURCES) ; do \
		make -C $$i ; \
	done;

clean:
	for i in $(PROGRAM_SOURCES) ; do \
		make clean -C $$i ; \
	done;
