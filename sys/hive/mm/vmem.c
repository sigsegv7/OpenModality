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
#include <sys/param.h>
#include <mm/vmem.h>
#include <mm/memvar.h>

int
vmem_map_region(struct mu_vas *vas, struct vmem_region *region, int prot)
{
    uintptr_t pbase, vbase;
    size_t length;
    int error;

    if (vas == NULL || region == NULL) {
        return -EINVAL;
    }

    /* Clamp parameters to page boundary */
    pbase = ALIGN_DOWN(region->pma, PAGESIZE);
    vbase = ALIGN_DOWN(region->vma, PAGESIZE);
    length = ALIGN_UP(region->length, PAGESIZE);

    for (off_t i = 0; i < length; i += PAGESIZE) {
        error = mu_pmap_map(
            vas,
            vbase + i,
            pbase + i,
            prot,
            PAGESIZE_4K
        );

        /* TODO: We'll need to clean up after ourselves here */
        if (error != 0) {
            return error;
        }
    }

    return 0;
}
