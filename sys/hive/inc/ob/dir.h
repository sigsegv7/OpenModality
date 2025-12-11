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

#ifndef _OB_DIR_H_
#define _OB_DIR_H_ 1

#include <sys/types.h>
#include <ob/knode.h>

#define KNODE_DIR(KNODE_P) ((KNODE_P)->data)

/*
 * Represents a knode directory
 *
 * @list: List of kernel nodes
 * @entry_count: Number of entries in this directory
 */
struct knode_dir {
    TAILQ_HEAD(, knode) list;
    size_t entry_count;
};

/*
 * Create a new knode directory
 *
 * @name: Name of directory to create
 * @res: Result pointer is written here
 *
 * Returns zero on success
 */
int ob_dir_new(const char *name, struct knode **res);

/*
 * Append a knode to a directory knode
 *
 * @knp: Knode to append
 * @dir_kn: Target directory knode
 *
 * Returns zero on success
 */
int ob_dir_append(struct knode *knp, struct knode *dir_kn);

/*
 * Obtain a knode by name from the root knode directory
 *
 * @name: Name to lookup
 * @res: Result is written here
 *
 * Returns zero on success
 */
int ob_root_get(const char *name, struct knode **res);

/*
 * Lookup a node within the root directory
 *
 * @type: Knode type to lookup
 * @cb: Callback invoked every time 'type' is encountered
 *
 * XXX: 'cb' returns a less than zero value to continue the iteration
 *      and a >= 0 value to terminate it
 */
int ob_root_foreach(ktype_t type, int(*cb)(struct knode *kn));

/*
 * Initialize the knode object store
 */
void ob_store_init(void);

#endif  /* !_OB_DIR_H_ */
