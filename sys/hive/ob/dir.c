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
#include <sys/types.h>
#include <sys/queue.h>
#include <sys/errno.h>
#include <os/pool.h>
#include <ob/dir.h>

int
ob_dir_new(const char *name, struct knode **res)
{
    struct knode_dir *dirp;
    struct knode *knp;
    int error;

    if (name == NULL || res == NULL) {
        return -EINVAL;
    }

    error = ob_knode_new(name, K_DIR, &knp);
    if (error != 0) {
        return error;
    }

    /* Allocate the data portion */
    knp->data = os_pool_allocate(sizeof(struct knode_dir));
    if (knp->data == NULL) {
        os_pool_free(knp);
        return -ENOMEM;
    }

    dirp = knp->data;
    TAILQ_INIT(&dirp->list);
    dirp->entry_count = 0;
    *res = knp;
    return 0;
}

int
ob_dir_append(struct knode *knp, struct knode *dir_kn)
{
    struct knode_dir *dir;
    int error;

    if (knp == NULL) {
        return -EINVAL;
    }

    /*
     * If the destination directory to append
     * the knode to is specified as NULL, attempt
     * to default at the root.
     */
    if (dir_kn == NULL) {
        error = ob_root_get("/", &dir_kn);
        if (error != 0)
            return error;
    }
    if (dir_kn == NULL) {
        return -ENOENT;
    }

    /* This must be a directory */
    if (dir_kn->type != K_DIR) {
        return -ENOTSUP;
    }

    if ((dir = dir_kn->data) == NULL) {
        return -EIO;
    }

    TAILQ_INSERT_TAIL(&dir->list, knp, dir_link);
    ++dir->entry_count;
    return 0;
}
