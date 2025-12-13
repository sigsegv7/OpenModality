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
#include <sys/cdefs.h>
#include <sys/param.h>
#include <core/bpt.h>
#include <core/panic.h>
#include <lib/string.h>

#define INITRD_PATH "/boot/initrd.mr"
#define MEMAR_MAGIC "LORD"
#define MEMAR_MAGIC_LEN 4
#define FILE_NAME_MAX 99

static struct bpt_module initrd;

/*
 * Represents a header that sits on top of each file
 * in the archive
 *
 * @hdr_size: Length of the header
 * @name: Filename
 *
 * This format is as follows:
 *
 * < FILE HEADER   >
 * < FILE CONTENTS >
 * < PADDING >
 * ...
 */
struct PACKED file_hdr {
    char magic[MEMAR_MAGIC_LEN];
    size_t hdr_size;
    size_t file_size;
    uint8_t fname_size;
    char name[FILE_NAME_MAX];
};

void *
initrd_lookup(const char *path)
{
    struct file_hdr *hdr;
    off_t next_off;
    size_t fname_size;
    char *p, *p_end;

    if (path == NULL) {
        return NULL;
    }

    if ((p = initrd.address) == NULL) {
        return NULL;
    }

    p_end = PTR_OFFSET(p, initrd.length);
    while (p < p_end) {
        hdr = (struct file_hdr *)p;
        if (memcmp(hdr->magic, MEMAR_MAGIC, MEMAR_MAGIC_LEN) != 0) {
            return NULL;
        }

        if (memcmp(hdr->name, path, hdr->fname_size) == 0) {
            return PTR_OFFSET(hdr, hdr->hdr_size);
        }

        p = PTR_OFFSET(hdr, hdr->file_size + hdr->hdr_size);
    }

    return NULL;
}

void
initrd_init(void)
{
    int error;

    error = bpt_get_module(INITRD_PATH, &initrd);
    if (error != 0) {
        panic("initrd: could not find \"%s\"\n", INITRD_PATH);
    }
}
