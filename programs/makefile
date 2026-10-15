SOURCES_DIR = sources
PROGRAM_SOURCES = $(SOURCES_DIR)/*
BINDIR = bin
TARGETS = $(BINDIR)/example

.PHONY=all
all:
	$(MAKE) -C libs
	for i in $(PROGRAM_SOURCES) ; do \
		$(MAKE) -C $$i ; \
	done;

clean:
	$(MAKE) clean -C libs
	for i in $(PROGRAM_SOURCES) ; do \
		$(MAKE) clean -C $$i ; \
	done;
