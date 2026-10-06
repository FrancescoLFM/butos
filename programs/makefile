SOURCES_DIR = sources
PROGRAM_SOURCES = $(SOURCES_DIR)/*
BINDIR = bin
TARGETS = $(BINDIR)/example

.PHONY=all
all:
	for i in $(PROGRAM_SOURCES) ; do \
		$(MAKE) -C $$i ; \
	done;

clean:
	for i in $(PROGRAM_SOURCES) ; do \
		$(MAKE) clean -C $$i ; \
	done;
