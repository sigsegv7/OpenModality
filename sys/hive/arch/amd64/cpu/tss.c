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
#include <sys/cdefs.h>
#include <md/tss.h>
#include <md/gdt.h>
#include <lib/string.h>
#include <os/pool.h>

int
md_tss_init(struct tss_entry *tss, struct tss_desc *desc)
{
    uintptr_t base;

    if (desc == NULL) {
        return -EINVAL;
    }

    /* Allocate a new TSS if we can */
    if (tss == NULL) {
        tss = os_pool_allocate(sizeof(*tss));
        if (tss == NULL) {
            return -1;
        }
    }

    memset(tss, 0, sizeof(*tss));
    base = (uintptr_t)tss;

    desc->base_low = base & 0xFFFF;
    desc->base_mid = (base >> 16) & 0xFF;
    desc->base_mid1 = (base >> 24) & 0xFF;
    desc->base_high = (base >> 32) & 0xFFFFFFFF;

    desc->avl = 0;          /* Unused */
    desc->dpl = 0;          /* Kernel only */
    desc->p   = 1;          /* Present */
    desc->type = 0x9;       /* TSS */

    ASMV(
        "str %%ax\n\t"
        "mov %0, %%ax\n\t"
        "ltr %%ax"
        :
        : "i" (GDT_TSS | 3)
        : "memory", "ax"
    );

    return 0;
}
