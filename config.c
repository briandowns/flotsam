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

#include <stdlib.h> 
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <jansson.h>

#include "config.h"

#define FLOTSAM_CONFIG_FILE "Flotsam.json"

static json_t *conf;
static json_t *package_table;
static json_t *dependency_table;
static json_t *authors_key;

static struct config *config;

static const char *name = {0};
static const char *type = {0};
static const char *build = {0};
static const char *repository = {0};
static const char *version = {0};
static const char *description = {0};
static const char *homepage = {0};

int
config_init()
{
    if (config != NULL) {
        return 0;
    }

    config = calloc(1, sizeof(struct config));
    if (config == NULL) {
        perror("unable to allocate memory for config");
        return -1;
    }

    if (access(FLOTSAM_CONFIG_FILE, F_OK)) {
        perror("error: Flotsam.json not found");
        return -1;
    }

    json_error_t error;

    json_t *root = json_load_file(FLOTSAM_CONFIG_FILE, 0, &error);
    if (root == NULL) {
        return 1;
    }

    const char *name_obj;
    const char *type_obj;
    const char *build_obj;
    const char *repository_obj;
    const char *version_obj;
    const char *description_obj;
    const char *homepage_obj;

    json_unpack(root, "{s: {s:s, s:s, s:s, s:s, s:s, s:s, s:s}}",
                "package",
                "name", &name_obj, "type", &type_obj, "build", &build_obj,
                "repository", &repository_obj, "version", &version_obj,
                "description", &description_obj, "homepage", &homepage_obj);

    json_t *dependencies_obj = json_object_get(root, "dependencies");
    if (!json_is_array(dependencies_obj)) {
        fprintf(stderr, "error: dependencies_obj is not an array\n");
        json_decref(root);
        return 1;
    }

    size_t array_size = json_array_size(dependencies_obj);
    for (size_t i = 0; i < array_size; i++) {
        json_t *item = json_array_get(dependencies_obj, i);
        if (!json_is_object(item)) {
            fprintf(stderr, "error: dependency item is not an object\n");
            json_decref(root);
            return 1;
        }

        const char *name;
        const char *version;

        json_unpack(item, "{s:s, s:s}", "name", &name, "version", &version);

        printf("Name: %s, Version: %s\n", name, version);
    }

    printf("Package:\n");
    printf("    name:        %s\n", name_obj);
    printf("    description: %s\n", description_obj);
    printf("    version:     %s\n", version_obj);
    printf("    build:       %s\n", build_obj);
    printf("    respository: %s\n", repository_obj);
    printf("    homepage:    %s\n", homepage_obj);

    json_decref(root);

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
        if (config->dependencies->dependencies != NULL) {
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
    printf("    name:        %s\n", name);
    printf("    description: %s\n", description);
    printf("    version:     %s\n", version);
    printf("    build:       %s\n", build);
    printf("    respository: %s\n", repository);
    printf("    homepage:    %s\n", homepage);
    // printf("    authors:     ");
    // for (int i = 0; i < config->author_count; i++) {
    //     if (config->author_count > 1) {
    //         if (i == config->author_count - 1) {
    //             printf("%s", config->authors[i]);
    //             break;
    //         }
    //         printf("%s, ", config->authors[i]);
    //         continue;
    //     }
    //     printf("%s", config->authors[i]);
    // }

    // printf("\n\nDependencies:\n    ");
    // config_print_dependencies();

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
