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

#include <dirent.h>
#include <git2.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#ifdef __linux__
#include <limits.h>
#else
#include <sys/syslimits.h>
#endif
#include <unistd.h>

#include "config.h"
#include "dependency.h"

#define DEP_CACHE_PATH "/.flotsam/"
#define PATH_SEPERATOR "/"
#define ULR_PREFIX_HTTPS "https://"
#define ULR_PREFIX_GIT "git@"
#define VERSION_SEPERATOR "@"
#define MAX_URL_LEN 2048
#define REFS_HEAD "refs/heads/"

#define GIT_ERROR_PRINT printf("error %d/%d: %s\n", res, e->klass, e->message)

#define DYLIB_EXT ".dylib"
#define SO_EXT ".so"
#define LIB_PATH "/usr/local/lib/"

/**
 * build_dependency_path returns the full path of the
 * dependency based on the dependency itself as well
 * as the version given. The returned string needs
 * to be freed by the caller.
 */
static char*
build_dependency_path(const char* dep, const char* ver)
{
    char* path = malloc(sizeof(char) * PATH_MAX + 1);
    memset(path, 0, PATH_MAX);
    strcat(path, getenv("HOME"));
    strcat(path, DEP_CACHE_PATH);
    strcat(path, dep);
    strcat(path, VERSION_SEPERATOR);
    strcat(path, ver);

    return path;
}

/**
 * clone clones the given dependency and checks out
 * the version provided.
 */
static int
clone(const char* dep, const char* ver)
{
    char url[MAX_URL_LEN];
    memset(&url, 0, MAX_URL_LEN);
    strcat(url, ULR_PREFIX_HTTPS);
    strcat(url, dep);

    char* path = build_dependency_path(dep, ver);

    // check if the directory already exists and if so, return;
    struct stat s = { 0 };
    if (stat(path, &s) == 0 && S_ISDIR(s.st_mode)) {
        return 0;
    }

    git_repository* repo = NULL;
    git_repository_init_options opts = GIT_REPOSITORY_INIT_OPTIONS_INIT;
    int res = git_clone(&repo, url, path, NULL);
    if (res != 0) {
        const git_error* e = giterr_last();
        GIT_ERROR_PRINT;
        return -1;
    }
    free(path);

    git_object* treeish = NULL;
    res = git_revparse_single(&treeish, repo, ver);
    if (res != 0) {
        const git_error* e = giterr_last();
        GIT_ERROR_PRINT;
        return -1;
    }

    git_checkout_options co_opts = GIT_CHECKOUT_OPTIONS_INIT;
    co_opts.checkout_strategy = GIT_CHECKOUT_FORCE;
    res = git_checkout_tree(repo, treeish, &co_opts);
    if (res != 0) {
        const git_error* e = giterr_last();
        GIT_ERROR_PRINT;
        return -1;
    }

    char vers_ref[300];
    memset(vers_ref, 0, 300);
    strcat(vers_ref, REFS_HEAD);
    strcat(vers_ref, ver);

    res = git_repository_set_head(repo, vers_ref);
    if (res != 0) {
        const git_error* e = giterr_last();
        GIT_ERROR_PRINT;
        return -1;
    }
    git_object_free(treeish);

    return res;
}

int
dependency_update(const char* dep, const char* ver)
{
    char* path = build_dependency_path(dep, ver);

    int res = clone(dep, ver);
    if (res != 0) {
        return -1;
    }
    chdir(path);

    char* build_cmd = config_get_build();
    build_cmd = realloc(build_cmd, 24);
    strcat(build_cmd, " > /dev/null 2>&1");
    if (system(build_cmd) != 0) {
        return 1;
    }

    DIR* dp;
    struct dirent* dirp;

    if ((dp = opendir(".")) == NULL) {
        perror("can't open .");
        return -1;
    }
    while ((dirp = readdir(dp)) != NULL) {
        if (strstr(dirp->d_name, DYLIB_EXT) != NULL || strstr(dirp->d_name, SO_EXT)) {
            char sl[PATH_MAX];
            memset(sl, 0, PATH_MAX);
            strcat(sl, path);
            strcat(sl, PATH_SEPERATOR);
            strcat(sl, dirp->d_name);

            char dl[PATH_MAX];
            memset(dl, 0, PATH_MAX);
            strcat(dl, LIB_PATH);
            strcat(dl, dirp->d_name);
            if (symlink(sl, dl) != 0) {
                perror(dirp->d_name);
                return -1;
            }
        }
    }
    free(path);
    closedir(dp);
    chdir(getenv("OLDPWD"));

    return 0;
}

int
dependency_update_all()
{
    return 0;
}