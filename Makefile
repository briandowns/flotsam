CC      ?= cc
DOCKER  ?= docker

UNAME_S = $(shell uname -s)

VERSION = 0.1.0
BINDIR  = bin
DEPDIR  = deps
INCDIR  = include
BINARY  = flotsam
LDFLAGS = -lgit2
CFLAGS  = -std=c99 -Wall -Wextra -fpic -Dbin_name=$(BINARY) -Dflotsam_version=$(VERSION) -Dgit_sha=$(shell git rev-parse HEAD)
ifeq ($(UNAME_S),Darwin)
	LDFLAGS += $(shell pkg-config --libs libgit2 jansson)
	CFLAGS += $(shell pkg-config --cflags libgit2 jansson)
else
	LDFLAGS += -ljansson
endif

PREFIX = /usr/local

MACOS_MANPAGE_LOC = /usr/share/man
LINUX_MAPPAGE_LOC = /usr/local/man/man8

$(BINDIR)/$(BINARY): $(BINDIR) clean
	$(CC) $(CFLAGS) main.c config.c dependency.c -o $(BINDIR)/$(BINARY) $(LDFLAGS)
	
$(BINDIR):
	mkdir -p $(BINDIR)

$(DEPDIR): 
	mkdir -p $(DEPDIR)

.PHONY: test manpage
test:
	$(CC) -o tests/tests tests/tests.c tests/unity/unity.c
	tests/tests
	rm -f tests/tests

install: $(BINDIR)/$(BINARY)
	mkdir -p $(PREFIX)/bin
	cp $(BINDIR)/$(BINARY) $(PREFIX)/bin

.PHONY: manpage
manpage:
ifeq ($(UNAME_S),Linux)
$(LINUX_MAPPAGE_LOC)/$(BINARY).1:
	cp $(INCDIR)/$(BINARY)_manpage $(LINUX_MAPPAGE_LOC)/$(BINARY).1
	gzip $(LINUX_MAPPAGE_LOC)/$(BINARY).1
endif
ifeq ($(UNAME_S),Darwin)
$(MACOS_MANPAGE_LOC)/$(BINARY).1:
	cp $(INCDIR)/$(BINARY)_manpage $(MACOS_MANPAGE_LOC)/$(BINARY).1
	gzip $(MACOS_MANPAGE_LOC)/$(BINARY).1
endif

uninstall:
	rm -f $(PREFIX)/bin/$(BINDIR)/$(BINARY)
	rm -f /usr/local/man/man8/$(BINARY).1

.PHONY: deps
deps: $(DEPDIR)

.PHONY: valgrind
valgrind:
	$(CC) -g -o $@ bluesky.c example.c $(CFLAGS) $(LDFLAGS)
	valgrind --leak-check=full ./valgrind 2>&1 | awk -F':' '/definitely lost:/ {print $2}'
	rm -f valgrind

.PHONY: image
image:
	$(DOCKER) build -t $(BINARY):latest .

.PHONY: push
push:
	$(DOCKER) push briandowns/$(BINARY):latest

.PHONY: clean
clean:
	rm -f $(BINDIR)/*
	rm -f *.gcno
