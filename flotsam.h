/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 Brian J. Downs, John K. Moore
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

#ifndef _FLOTSAM_H
#define _FLOTSAM_H

#include <stdio.h>

#define FLOTSAM_TEMPLATE                  \
    "[package]\n"                         \
    "# required\n"                        \
    "name = \"%1$s\"\n\n"                 \
    "# required\n"                        \
    "type = \"%4$s\"\n\n"                 \
    "# required\n"                        \
    "build = \"make\"\n\n"                \
    "# required\n"                        \
    "repository = \"\"\n\n"               \
    "# required\n"                        \
    "version = \"%2$s\"\n\n"              \
    "# optional\n"                        \
    "description = \"\"\n\n"              \
    "# optional\n"                        \
    "homepage = \"\"\n\n"                 \
    "# optional\n"                        \
    "authors = [\"%3$s\"]\n\n"            \
    "# required even if nothing listed\n" \
    "[dependencies]\n\n"

void
flotsam_render(FILE *fd, const char *name, const char *version,
               const char *user, const char *type)
{
    fprintf(fd, FLOTSAM_TEMPLATE, name, version, user, type);
}

#endif /* _MAKEFILE_H */
