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

#include <os/pool.h>
#include <core/panic.h>
#include <core/spinlock.h>
#include <mm/memvar.h>
#include <mm/tlsf.h>
#include <mm/pmem.h>

#define POOLSIZE_BYTES 0x2000000  /* 4 MiB */
#define POOLSIZE_PAGES (POOLSIZE_BYTES / PAGESIZE)

static bool is_init = false;
static void *pool_virt_base;
static uintptr_t pool_pma = 0;
static spinlock_t lock;
tlsf_t tlsf_ctx;

void
os_pool_free(void *pool)
{
    spinlock_acquire(&lock, true);
    tlsf_free(tlsf_ctx, pool);
    spinlock_release(&lock);
}

void *
os_pool_allocate(size_t length)
{
    void *tmp;

    spinlock_acquire(&lock, true);
    tmp = tlsf_malloc(tlsf_ctx, length);
    spinlock_release(&lock);
    return tmp;
}

void
os_pool_init(void)
{
    if (is_init) {
        return;
    }

    pool_pma = mm_pmem_alloc(POOLSIZE_PAGES);
    if (pool_pma == 0) {
        panic("pool: could not initialize root pool\n");
    }

    pool_virt_base = PHYS_TO_VIRT(pool_pma);
    tlsf_ctx = tlsf_create_with_pool(pool_virt_base, POOLSIZE_BYTES);
    is_init = true;
}
