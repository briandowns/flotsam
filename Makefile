CC      ?= cc
DOCKER  ?= docker

VERSION := 0.1.0
BINDIR  := bin
DEPDIR  := deps
INCDIR  := include
BINARY  := flotsam
LDFLAGS := -ltoml -lgit2
CFLAGS  := -coverage -Dbin_name=$(BINARY) -Dflotsam_version=$(VERSION) -Dgit_sha=$(shell git rev-parse HEAD)

PREFIX = /usr/local

UNAME_S = $(shell uname -s)

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

.PHONY: image
image:
	$(DOCKER) build -t $(BINARY):latest .

.PHONY: push
push:
	$(DOCKER) push briandowns/$(BINARY):latest

.PHONY: clean
clean:
	rm -f $(BINDIR)/*
