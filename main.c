/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 Brian J. Downs
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

#include <dirent.h>
#include <ftw.h>
#include <git2.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#ifdef __linux__
#include <limits.h>
#else
#include <sys/syslimits.h>
#endif
#include <sys/types.h>
#include <unistd.h>

#include "config.h"
#include "dependency.h"
#include "dockerfile.h"
#include "flotsam.h"
#include "gitignore.h"
#include "main.h"
#include "makefile.h"
#include "readme.h"

#define STR1(x) #x
#define STR(x) STR1(x)

#define USAGE                                                                 \
    "usage: %s [-vh]\n"                                                       \
    "  -v          version\n"                                                 \
    "  -h          help\n\n"                                                  \
    "commands:\n"                                                             \
    "  new          --bin <name> create new binary application\n"             \
    "               --lib <name> create new library\n"                        \
    "  build        builds the project with the given build constraint.\n"    \
    "  config       display the current project configuration.\n"             \
    "  deps         displays the project's dependencies.\n"                   \
    "  update       retrieves newly added dependencies.\n"                    \
    "  clean        cleans the current project based on the build parameter\n"

#define MAX_NEW_CMD_ARG_COUNT 5
#define DEFAULT_VERSION       "0.1.0"
#define LIB_PREFIX            "lib"

/**
 * FLOTSAM_BASE_DIRECTORY initializes a the flotsam_dir variable to contain the
 * default location of the flotsam directory for that use on that system.
 */
#define FLOTSAM_BASE_DIRECTORY                               \
    char flotsam_dir[PATH_MAX];                              \
    flotsam_dir[0] = '\0';                                   \
    strcat(strcpy(flotsam_dir, getenv("HOME")), "/.flotsam")

/**
 * initialize_flotsom_dir creates a hidden directory called
 * .flotsam in the user's home directory.
 */
#define INITIALIZE_FLOTSAM_DIR \
    FLOTSAM_BASE_DIRECTORY;    \
    mkdir(flotsam_dir, 0700)

/**
 * project_type represents all of the different
 * types of projects that can be generated.
 */
enum project_type {
    bin,
    lib
};

// project_directories contains the list of directories that
// need to be generated at project creation.
static const char *project_directories[] = { "tests" };

// project_files contains the list of files that need to be
// generated at project creation.
static const char *project_files[] = {".gitignore", "Makefile", "README.md",
                                      "LICENSE", "Flotsam.toml", "Dockerfile"};

// license_types contains the list of supported licenses that can be selected
// at project generation time.
static const char *license_types[] = {"bsd2", "bsd3", "mit", "isc"};

// with_dockerfile stores whether or not a Dockerfile should be rendered.
static int with_dockerfile = 0;

/**
 * render_templates creates and populates files with the necessary contents.
 */
int
render_templates(const enum project_type pt, const char *name)
{
    char *lib_name = strdup(name);
    FILE *fd;
    for (int i = 0; i < (sizeof(project_files) / sizeof(char*)); i++) {
        if (strcmp(project_files[i], "Dockerfile") == 0 && pt != bin) {
            continue;
        }
        fd = fopen(project_files[i], "w");
        if (strcmp(project_files[i], ".gitignore") == 0) {
            gitignore_render(fd, name);
        }
        if (strcmp(project_files[i], "Makefile") == 0) {
            FILE* fd2;
            switch (pt) {
                case bin:
                    makefile_bin_render(fd, name, DEFAULT_VERSION);
                    break;
                case lib:
                    makefile_lib_render(fd, name, DEFAULT_VERSION);
                    break;
            }
        }
        if (strcmp(project_files[i], "README.md") == 0) {
            readme_render(fd, name, DEFAULT_VERSION);
        }
        if (strcmp(project_files[i], "Flotsam.toml") == 0) {
            FILE *fd2;
            switch (pt) {
                case bin:
                    flotsam_render(fd, name, DEFAULT_VERSION,
                                   getenv("USER"), "bin");
                    fd2 = fopen("main.c", "w");
                    main_render(fd2, name, DEFAULT_VERSION);
                    fclose(fd2);
                    break;
                case lib:
                    flotsam_render(fd, name, DEFAULT_VERSION,
                                   getenv("USER"), "lib");
                    lib_name = realloc(lib_name, 3);
                    strcat(lib_name, ".h");
                    fd2 = fopen(lib_name, "w");
                    fclose(fd2);
                    free(lib_name);
                    break;
            }
        }
        if (strcmp(project_files[i], "Dockerfile") == 0 && pt == bin) {
            if (with_dockerfile) {
                dockerfile_render(fd, name);
            }
        }
        fclose(fd);
    }

    return 0;
}

/**
 * create_project_dirs creates all of the necessary dirs for
 * the given project type.
 */
static void
create_project_dirs(const enum project_type pt)
{
    for (int i = 0; i < (sizeof(project_directories) / sizeof(char*)); i++) {
        mkdir(project_directories[i], 0700);
    }
    if (pt == bin) {
        mkdir("bin", 0700);
    }
}

/**
 * strip_lib removes the "lib" prefix from the
 * given string if it's present.
 */
static void
strip_chars(char *s, const char *sc)
{
    while ((s = strstr(s, sc))) {
        memmove(s, s + strlen(sc), 1 + strlen(s + strlen(sc)));
    }
}

int
main(int argc, char **argv)
{
    if (argc < 2) {
        printf(USAGE, STR(bin_name));
        return 1;
    }

    git_libgit2_init();

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0) {
            printf("version: %s - git: %s\n", STR(flotsam_version),
                   STR(git_sha));
            break;
        }
        if (strcmp(argv[i], "-h") == 0) {
            printf(USAGE, STR(bin_name));
            break;
        }
        if (strcmp(argv[i], "new") == 0) {
            if ((argc > MAX_NEW_CMD_ARG_COUNT) || (argc < MAX_NEW_CMD_ARG_COUNT - 1)) {
                fprintf(stderr, "new requires at least 2 args; --bin or --lib and project_name\n");
                return 1;
            }

            char project_type[4] = "";
            char* project_name = strdup(argv[i + 2]);

            if (strcmp(argv[i + 1], "--bin") == 0) {
                if (argc == 5) {
                    if ((strcmp(argv[3], "--with-docker") == 0) || (strcmp(argv[4], "--with-docker") == 0)) {
                        printf("here...\n");
                        with_dockerfile = 1;
                    }
                }

                strcpy(project_type, "bin");

                if (mkdir(argv[i + 2], 0700) == -1) {
                    fprintf(stderr, "project already exists\n");
                    return 1;
                }

                chdir(argv[i + 2]);
                git_repository* repo = NULL;
                if (git_repository_init(&repo, ".", 0) != 0) {
                    fprintf(stderr, "error: failed to initilize git repository\n");
                    return 1;
                }
                git_repository_free(repo);

                create_project_dirs(bin);

                if (render_templates(bin, project_name) != 0) {
                    return 1;
                }

                chdir(getenv("OLDPWD"));

                break;
            }

            if (strcmp(argv[i + 1], "--lib") == 0) {
                strcpy(project_type, "lib");

                if (mkdir(argv[i + 2], 0700) == -1) {
                    fprintf(stderr, "project already exists\n");
                    return 1;
                }

                chdir(argv[i + 2]);
                git_repository* repo = NULL;
                if (git_repository_init(&repo, ".", 0) != 0) {
                    fprintf(stderr, "error: failed to initilize git repository\n");
                    return 1;
                }
                git_repository_free(repo);

                create_project_dirs(lib);

                if (render_templates(lib, project_name) != 0) {
                    return 1;
                }

                chdir(getenv("OLDPWD"));

                break;
            } else {
                fprintf(stderr, "%s\n", "unrecognized flag");
                return 1;
            }

            free(project_name);

            break;
        }

        INITIALIZE_FLOTSAM_DIR;

        config_init();

        struct dependencies* deps = config_get_dependencies();

        if (strcmp(argv[i], "build") == 0) {
            char* build_cmd = config_get_build();
            build_cmd = realloc(build_cmd, 14);
            char* home_dir = getenv("HOME");

            for (int i = 0; i < deps->count; i++) {
                build_cmd = realloc(build_cmd,
                                    28 + strlen(deps->dependencies[i].name) + strlen(deps->dependencies[i].vers) +
                                      strlen(home_dir));
                strcat(build_cmd, " CFLAGS+='-I");
                strcat(build_cmd, home_dir);
                strcat(build_cmd, "/.flotsam/");
                strcat(build_cmd, deps->dependencies[i].name);
                strcat(build_cmd, "@");
                strcat(build_cmd, deps->dependencies[i].vers);
                strcat(build_cmd, "'");
                strcat(build_cmd, " LDFLAGS+='-L");
                strcat(build_cmd, home_dir);
                strcat(build_cmd, "/.flotsam/");
                strcat(build_cmd, deps->dependencies[i].name);
                strcat(build_cmd, "@");
                strcat(build_cmd, deps->dependencies[i].vers);
                strcat(build_cmd, "'");
                strcat(build_cmd, " LDFLAGS+='-l");
                strip_chars(deps->dependencies[i].name, LIB_PREFIX);
                strip_chars(deps->dependencies[i].name, ".h");
                strcat(build_cmd, "'");
                char* lib_name = strrchr(deps->dependencies[i].name, '/') + 1;
                strcat(build_cmd, lib_name);
            }

            if (system(build_cmd) != 0) {
                return 1;
            }
            free(build_cmd);
            break;
        }
        if (strcmp(argv[i], "install") == 0) {
            char* test_cmd = config_get_build();
            test_cmd = realloc(test_cmd, 9);
            strcat(test_cmd, " install");
            if (system(test_cmd) != 0) {
                return 1;
            }
            free(test_cmd);
            break;
        }
        if (strcmp(argv[i], "config") == 0) {
            if (config_print() != 0) {
                perror("error reading config");
            }
            break;
        }
        if (strcmp(argv[i], "deps") == 0) {
            if (config_print_dependencies() != 0) {
                perror("error reading config");
            }
            break;
        }
        if (strcmp(argv[i], "test") == 0) {
            char* test_cmd = config_get_build();
            test_cmd = realloc(test_cmd, 6);
            strcat(test_cmd, " test");
            if (system(test_cmd) != 0) {
                return 1;
            }
            free(test_cmd);
            break;
        }
        if (strcmp(argv[i], "update") == 0) {
            struct dependencies* deps = config_get_dependencies();
            for (int i = 0; i < deps->count; i++) {
                if (dependency_update(deps->dependencies[i].name, deps->dependencies[i].vers) != 0) {
                    return 1;
                }
            }
            break;
        }
        if (strcmp(argv[i], "clean") == 0) {
            char* test_cmd = config_get_build();
            test_cmd = realloc(test_cmd, 6);
            strcat(test_cmd, " clean");
            if (system(test_cmd) != 0) {
                return 1;
            }
            free(test_cmd);
            break;
        }

        git_libgit2_shutdown();
        config_free();

        return 0;
    }
}
