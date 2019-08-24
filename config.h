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

#ifndef _CONFIG_H
#define _CONFIG_H

#include <toml.h>

static toml_table_t* conf;
static toml_table_t* package_table;
static toml_table_t* dependency_table;
static toml_array_t* authors_key;

const char* raw;

static char errbuf[200];

/**
 * dependency represents a single dependency
 * containing a name and a version.
 */
struct dependency
{
    char* name;
    char* vers;
};

/**
 * dependencies contains all dependencies from a
 * given Flotsam config file.
 */
struct dependencies
{
    int count;
    struct dependency* dependencies;
};

/**
 * config contains all settings to run flotsam.
 */
struct config
{
    char* name;
    char* pkg_ver;
    char* description;
    char** authors;
    int author_count;
    char* type;
    char* build;
    char* repository;
    char* homepage;
    struct dependencies* dependencies;
};

/**
 * config_init initializes the configuration from a Flotsam.toml.
 */
int
config_init();

char*
config_get_build();

/**
 * config_print prints the current configuration from
 * a valid Flotsam.toml file.
 */
int
config_print();

/**
 * config_print_dependencies prints the current set of
 * dependencies from a valid Flotsam.toml file.
 */
int
config_print_dependencies();

/**
 * config_free frees the memory used in the config struct.
 */
void
config_free();

/**
 * config_get_dependencies
 */
struct dependencies*
config_get_dependencies();

#endif /* _CONFIG_H */
