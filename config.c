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

#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <toml.h>
#include <unistd.h>

#include "config.h"

#define FLOTSAM_CONFIG_FILE "Flotsam.toml"

static toml_table_t* conf;
static toml_table_t* package_table;
static toml_table_t* dependency_table;
static toml_array_t* authors_key;

const char* raw;

static char err[200];

static struct config* config;

int
config_init()
{
    if (config != NULL) {
        return 0;
    }

    config = malloc(sizeof(struct config));

    if (access(FLOTSAM_CONFIG_FILE, F_OK)) {
        perror("error: Flotsam.toml not found");
        return -1;
    }

    FILE* fp = fopen(FLOTSAM_CONFIG_FILE, "r");
    if (fp == NULL) {
        perror("fopen");
        return -1;
    }
    conf = toml_parse_file(fp, err, sizeof(err));
    fclose(fp);
    if (conf == NULL) {
        fprintf(stderr, "error: %s\n", err);
        return -1;
    }

    if ((package_table = toml_table_in(conf, "package")) == 0) {
        perror("error: missing [package]\n");
        toml_free(conf);
        return -1;
    }

    if ((raw = toml_raw_in(package_table, "name")) == 0) {
        perror("error: missing 'name' in [package]");
        toml_free(conf);
        return -1;
    }
    if (toml_rtos(raw, &config->name)) {
        perror("error: bad value in 'name'\n");
        toml_free(conf);
        return -1;
    }

    if ((raw = toml_raw_in(package_table, "description")) == 0) {
        perror("error: missing 'description' in [package]");
        toml_free(conf);
        return -1;
    }
    if (toml_rtos(raw, &config->description)) {
        perror("error: bad value in 'description'\n");
        toml_free(conf);
        return -1;
    }

    if ((raw = toml_raw_in(package_table, "version")) == 0) {
        perror("error: missing 'version' in [package]");
        toml_free(conf);
        return -1;
    }
    if (toml_rtos(raw, &config->pkg_ver)) {
        perror("error: bad value in 'version'");
        toml_free(conf);
        return -1;
    }

    if ((raw = toml_raw_in(package_table, "type")) == 0) {
        perror("error: missing 'type' in [package]");
        toml_free(conf);
        return -1;
    }
    if (toml_rtos(raw, &config->type)) {
        perror("error: bad value in 'type'");
        toml_free(conf);
        return -1;
    }

    if ((raw = toml_raw_in(package_table, "build")) == 0) {
        perror("error: missing 'build' in [package]");
        toml_free(conf);
        return -1;
    }
    if (toml_rtos(raw, &config->build)) {
        perror("error: bad value in 'build'");
        toml_free(conf);
        return -1;
    }

    if ((raw = toml_raw_in(package_table, "repository")) == 0) {
        perror("error: missing 'repository' in [package]");
        toml_free(conf);
        return -1;
    }
    if (toml_rtos(raw, &config->repository)) {
        perror("error: bad value in 'repository'");
        toml_free(conf);
        return -1;
    }

    if ((raw = toml_raw_in(package_table, "homepage")) == 0) {
        perror("error: missing 'homepage' in [package]");
        toml_free(conf);
        return -1;
    }
    if (toml_rtos(raw, &config->homepage)) {
        perror("error: bad value in 'homepage'");
        toml_free(conf);
        return -1;
    }

    if ((authors_key = toml_array_in(package_table, "authors")) == 0) {
        perror("error: missing 'authors' in [package]");
        toml_free(conf);
        return -1;
    }

    config->author_count = toml_array_nelem(authors_key);
    config->authors = malloc(sizeof(char*) * config->author_count);
    for (int i = 0; i < config->author_count; i++) {
        raw = toml_raw_at(authors_key, i);
        if (toml_rtos(raw, &config->authors[i])) {
            perror("error: bad value in field");
            toml_free(conf);
            return -1;
        }
    }

    if ((dependency_table = toml_table_in(conf, "dependencies")) == 0) {
        perror("error: missing [dependencies]");
        toml_free(conf);
        return -1;
    }

    int dep_count = toml_table_nkval(dependency_table);
    config->dependencies = malloc(sizeof(struct dependency));
    config->dependencies->count = dep_count;
    config->dependencies->dependencies = malloc(sizeof(struct dependency) * config->dependencies->count);

    for (int i = 0; i < config->dependencies->count; i++) {
        raw = toml_key_in(dependency_table, i);
        config->dependencies->dependencies[i].name = strdup(raw);

        if ((raw = toml_raw_in(dependency_table, raw)) == 0) {
            perror("error: missing key in [dependencies]");
            toml_free(conf);
            return -1;
        }
        if (toml_rtos(raw, &config->dependencies->dependencies[i].vers)) {
            perror("error: bad value in field");
            toml_free(conf);
            return -1;
        }
    }

    toml_free(conf);

    return 0;
}

char*
config_get_build()
{
    return config->build;
}

void
config_free()
{
    if (config == NULL) {
        return;
    }
    if (config->dependencies != NULL) {
        if (config->dependencies != NULL) {
            for (int i = 0; i < config->dependencies->count; i++) {
                if (config->dependencies->dependencies[i].name != NULL) {
                    free(config->dependencies->dependencies[i].name);
                }
                if (config->dependencies->dependencies[i].vers != NULL) {
                    free(config->dependencies->dependencies[i].vers);
                }
            }
            free(config->dependencies->dependencies);
        }
        free(config->dependencies);
    }
    free(config);
}

struct dependencies*
config_get_dependencies()
{
    return config->dependencies;
}

int
config_dependency_count()
{
    return config->dependencies->count;
}

int
config_print()
{
    if (config == NULL) {
        return -1;
    }

    printf("Package:\n");
    printf("    name:        %s\n", config->name);
    printf("    description: %s\n", config->description);
    printf("    version:     %s\n", config->pkg_ver);
    printf("    build:       %s\n", config->build);
    printf("    respository: %s\n", config->repository);
    printf("    homepage:    %s\n", config->homepage);
    printf("    authors:     ");
    for (int i = 0; i < config->author_count; i++) {
        if (config->author_count > 1) {
            if (i == config->author_count - 1) {
                printf("%s", config->authors[i]);
                break;
            }
            printf("%s, ", config->authors[i]);
            continue;
        }
        printf("%s", config->authors[i]);
    }

    printf("\n\nDependencies:\n    ");
    config_print_dependencies();

    return 0;
}

int
config_print_dependencies()
{
    if (config == NULL) {
        return -1;
    }
    for (int i = 0; i < config->dependencies->count; i++) {
        printf("%s - %s\n", config->dependencies->dependencies[i].name, config->dependencies->dependencies[i].vers);
    }
    return 0;
}
