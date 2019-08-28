/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2019 Brian J. Downs, John K. Moore
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef _MAKEFILE_H
#define _MAKEFILE_H

#include <stdio.h>

#define MAKEFILE_BIN_TEMPLATE                                                                                          \
    "CC               ?= cc\n"                                                                                         \
    "DOCKER           ?= docker\n\n"                                                                                   \
    "VERSION          := %2$s\n"                                                                                       \
    "BINDIR           := bin\n"                                                                                        \
    "BINARY           := %1$s\n"                                                                                       \
    "override LDFLAGS +=\n"                                                                                            \
    "override CFLAGS  += -Dapp_name=$(BINARY) -Dgit_sha=$(shell git rev-parse HEAD) -O3\n\n"                           \
    "$(BINDIR)/$(BINARY): $(BINDIR) clean\n"                                                                           \
    "\t$(CC) main.c $(CFLAGS) -o $(BINDIR)/$(BINARY) $(LDFLAGS)\n\n"                                                   \
    "$(BINDIR):\n"                                                                                                     \
    "\tmkdir -p $(BINDIR)\n\n"                                                                                         \
    "$(DEPDIR):\n"                                                                                                     \
    "\tmkdir -p $(DEPDIR)\n\n"                                                                                         \
    ".PHONY: image\n"                                                                                                  \
    "image:\n"                                                                                                         \
    "\t$(DOCKER) build -t $(BINARY):latest .\n\n"                                                                      \
    ".PHONY: push\n"                                                                                                   \
    "push:\n\n"                                                                                                        \
    ".PHONY: deps\n"                                                                                                   \
    "deps: $(DEPDIR)\n\n"                                                                                              \
    ".PHONY: clean\n"                                                                                                  \
    "clean:\n"                                                                                                         \
    "\trm -f $(BINDIR)/*\n\n"

#define MAKEFILE_LIB_TEMPLATE                                                                                          \
    "CC               ?= cc\n\n"                                                                                       \
    "VERSION          := %2$s\n"                                                                                       \
    "NAME             := %1$s\n\n"                                                                                     \
    "INCDIR           := /usr/local/include\n"                                                                         \
    "LIBDIR           := /usr/local/lib\n\n"                                                                           \
    "UNAME_S := $(shell uname -s)\n\n"                                                                                 \
    "ifeq ($(UNAME_S),Linux)\n"                                                                                        \
    "$(NAME).$(VERSION).so:\n"                                                                                         \
    "\t$(CC)  -c $(CFLAGS) -shared -o $(NAME).so %1$s.c  $(LDFLAGS)\n"                                                 \
    "endif\n"                                                                                                          \
    "ifeq ($(UNAME_S),Darwin)\n"                                                                                       \
    "$(NAME).$(VERSION).dylib:\n"                                                                                      \
    "\t$(CC) -c $(CFLAGS) -dynamiclib -o $(NAME).dylib %1$s.c  $(LDFLAGS)\n"                                           \
    "endif\n\n"                                                                                                        \
    "override LDFLAGS +=\n"                                                                                            \
    "override CFLAGS  += -Dgit_sha=$(shell git rev-parse HEAD) -O3\n\n"                                                \
    ".PHONY: clean\n"                                                                                                  \
    "clean:\n"                                                                                                         \
    "\trm -f $(BINDIR)/*\n\n"

/**
 * makefile_bin_render perform the rendering for a Makefile used
 * with a Flotsam application.
 */
void
makefile_bin_render(FILE* fd, const char* name, const char* version)
{
    fprintf(fd, MAKEFILE_BIN_TEMPLATE, name, version);
}

/**
 * makefile_lib_render perform the rendering for a Makefile used
 * with a Flotsam library.
 */
void
makefile_lib_render(FILE* fd, const char* name, const char* version)
{
    fprintf(fd, MAKEFILE_LIB_TEMPLATE, name, version);
}

#endif /* _MAKEFILE_H */
