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

#include <sys/types.h>
#include <core/panic.h>
#include <lib/stdbool.h>
#include <ob/knode.h>
#include <ob/dir.h>

static bool is_init = false;
static struct knode *root_dir;

int
ob_root_foreach(ktype_t type, int(*cb)(struct knode *kn))
{
    struct knode_dir *dir;
    struct knode *knp;
    int retval = 0;

    if (root_dir == NULL || cb == NULL) {
        return -1;
    }

    dir = KNODE_DIR(root_dir);
    TAILQ_FOREACH(knp, &dir->list, dir_link) {
        if (knp->type != type) {
            continue;
        }

        if ((retval = cb(knp)) >= 0) {
            break;
        }
    }

    return retval;
}

void
ob_store_init(void)
{
    if (is_init) {
        return;
    }

    is_init = true;
    if (ob_dir_new("/", &root_dir) != 0) {
        panic("ob: could not allocate root directory\n");
    }
}
