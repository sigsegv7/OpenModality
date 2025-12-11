/*
 * Copyright (c) 2025 Ian Marco Moffett and the OpenModality engineers
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the project nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/errno.h>
#include <sys/queue.h>
#include <sys/cdefs.h>
#include <ob/knode.h>
#include <ob/dir.h>
#include <os/pool.h>
#include <lib/string.h>

static int
knode_subdir_find(struct knode *parent, const char *name, struct knode **res)
{
    struct knode *knp;
    struct knode_dir *pardir;
    int retval = -ENOENT;

    if (parent == NULL || name == NULL) {
        return -EINVAL;
    }

    if (res == NULL) {
        return -EINVAL;
    }

    /* Is this actually a directory? */
    if (parent->type != K_DIR) {
        return -ENOTDIR;
    }

    pardir = KNODE_DIR(parent);
    TAILQ_FOREACH(knp, &pardir->list, dir_link) {
        if (LIKELY(knp->name[0] != *name)) {
            continue;
        }

        if (strcmp(knp->name, name) == 0) {
            *res = knp;
            retval = 0;
            break;
        }
    }

    return retval;
}

int
ob_knode_new(const char *name, ktype_t type, struct knode **res)
{
    struct knode *knp;
    size_t name_len;

    if (name == NULL || res == NULL) {
        return -EINVAL;
    }

    /* Ensure there is no overflow */
    name_len = strlen(name);
    if (name_len >= KNODE_NAME_LEN - 1) {
        return -ENAMETOOLONG;
    }

    /* Allocate a new node */
    knp = os_pool_allocate(sizeof(*knp));
    if (knp == NULL) {
        return -ENOMEM;
    }

    memcpy(knp->name, name, name_len);
    knp->type = type;
    knp->ref = 1;
    *res = knp;
    return 0;
}

int
ob_knode_resolve(const char *path, int flags, struct knode **res)
{
    struct knode *knp = NULL;
    const char *p;
    char pathbuf[128];
    size_t pathbuf_idx = 0;
    int error;

    if (path == NULL || res == NULL) {
        return -EINVAL;
    }

    p = path;
    while (*p != '\0') {
        /* Skip leading slashes */
        while (*p == '/' && *p != '\0') {
            ++p;
        }

        /* Fill the component buffer */
        while (*p != '/' && *p != '\0') {
            if (pathbuf_idx >= sizeof(pathbuf) - 1) {
                return -ENAMETOOLONG;
            }

            pathbuf[pathbuf_idx++] = *p++;
        }

        pathbuf[pathbuf_idx] = '\0';
        pathbuf_idx = 0;

        /* Do we need to search the root? */
        if (knp == NULL) {
            if ((error = ob_root_get(pathbuf, &knp)) != 0)
                return error;

            continue;
        }

        /* Lookup the subdirectory now */
        error = knode_subdir_find(knp, pathbuf, &knp);
        if (error != 0) {
            return error;
        }
    }

    *res = knp;
    return 0;
}
