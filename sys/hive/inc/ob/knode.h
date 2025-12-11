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

#ifndef _OB_KNODE_H_
#define _OB_KNODE_H_ 1

#include <sys/queue.h>
#include <sys/types.h>

/* Maximum length of knode names */
#define KNODE_NAME_LEN 32

/*
 * Represents valid kernel node types
 *
 * @K_NONE:     No assigned type
 * @K_CLKDEV:   Clock device node
 */
typedef enum {
    K_NONE,
    K_DIR,
    K_CLKDEV,
} ktype_t;

/*
 * A kernel node is an abstract representation
 * of any system object. It is not bound to any
 * single type and each is backed by its own respective
 * subsystems.
 *
 * @name: Name of kernel node
 * @type: Type of kernel node
 * @data: Opaque reference to backing data
 * @ref:  Reference counter
 * @dir_link: Directory queue link
 */
struct knode {
    char name[KNODE_NAME_LEN];
    ktype_t type;
    void *data;
    int ref;
    TAILQ_ENTRY(knode) dir_link;
};

/*
 * Initialize a new knode
 *
 * @name: Name of knode to initialize
 * @type: Type of knode
 * @res: Pointer of newly allocated knode written here
 *
 * Returns zero on success
 */
int ob_knode_new(const char *name, ktype_t type, struct knode **res);

/*
 * Resolve a knode by path
 *
 * @path: Path to resolve
 * @flags: Optional flags
 * @res: Result pointer is written here
 *
 * Returns zero on success
 */
int ob_knode_resolve(const char *path, int flags, struct knode **res);

#endif  /* !_OB_KNODE_H_ */
