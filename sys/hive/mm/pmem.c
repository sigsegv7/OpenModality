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

#include <sys/limits.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/units.h>
#include <lib/string.h>
#include <lib/stdbool.h>
#include <mm/memvar.h>
#include <core/spinlock.h>
#include <core/bpt.h>
#include <core/trace.h>

#define dtrace(fmt, ...) printf("pmem: " fmt, ##__VA_ARGS__)

/* Various stats */
static uintptr_t usable_top = 0;
static size_t mem_usable = 0;

/* Bitmap */
static size_t bitmap_size = 0;
static uint8_t *bitmap = NULL;
static size_t last_bit = 0;
static spinlock_t bitmap_lock = 0;

/*
 * Display size values in a pretty format
 */
static inline void
pmem_print_size(const char *name, size_t size)
{
    if (size >= UNIT_GIB) {
        dtrace("%s: %d GiB\n", name, size / UNIT_GIB);
    } else if (size >= UNIT_MIB) {
        dtrace("%s: %d MiB\n", name, size / UNIT_MIB);
    } else {
        dtrace("%s: %d bytes\n", name, size);
    }
}

/*
 * Mark a range of memory as allocated or free
 *
 * 1: ALLOCATED
 * 0: FREE
 */
static void
bitmap_set_range(uintptr_t start, uintptr_t end, bool alloc)
{
    /* Clamp range to page boundary */
    start = ALIGN_UP(start, PAGESIZE);
    end = ALIGN_UP(end, PAGESIZE);

    for (uintptr_t p = start; p < end; p += PAGESIZE) {
        if (alloc) {
            SETBIT(bitmap, p / PAGESIZE);
        } else {
            CLRBIT(bitmap, p / PAGESIZE);
        }
    }
}

/*
 * Fill the bitmap based on the system memory
 * map
 */
static void
pmem_fill_bitmap(void)
{
    struct bpt_mementry entry;
    uintptr_t start, end;

    memset(bitmap, 0xFF, bitmap_size);
    for (size_t i = 0;; ++i) {
        if (bpt_get_mementry(i, &entry) != 0) {
            break;
        }

        if (entry.type == MEM_USABLE) {
            start = entry.base;
            end = start + entry.length;
            bitmap_set_range(start, end, false);
        }
    }
}

/*
 * Locate an area big enough in physical memory to
 * hold the bitmap
 */
static void
pmem_alloc_bitmap(void)
{
    struct bpt_mementry entry;

    for (size_t i = 0;; ++i) {
        if (bpt_get_mementry(i, &entry) != 0) {
            break;
        }

        /* Drop unusable entries */
        if (entry.type != MEM_USABLE) {
            continue;
        }

        /* Drop entries that are too small */
        if (entry.length < bitmap_size) {
            continue;
        }

        bitmap = PHYS_TO_VIRT(entry.base);
        break;
    }

    pmem_fill_bitmap();
}

/*
 * Probe physical memory and gather memory statistics.
 */
static void
pmem_probe(void)
{
    struct bpt_mementry entry;

    for (size_t i = 0;; ++i) {
        if (bpt_get_mementry(i, &entry) != 0) {
            break;
        }

        /* Drop unusable entries */
        if (entry.type != MEM_USABLE) {
            continue;
        }

        mem_usable += entry.length;
        if ((entry.base + entry.length) > usable_top) {
            usable_top = entry.base + entry.length;
        }
    }

    /* Compute the length of the bitmap */
    bitmap_size = mem_usable / PAGESIZE;
    bitmap_size = ALIGN_UP(bitmap_size, CHAR_BIT) / CHAR_BIT;

    /* Print stats */
    pmem_print_size("usable", mem_usable);
    pmem_print_size("bitmap", bitmap_size);
}

/*
 * Allocate one or more physical memory frames
 */
static uintptr_t
pmem_alloc(size_t count)
{
    ssize_t start_idx = -1;
    size_t frames_found = 0;
    uintptr_t phys = 0;
    size_t max_bit;

    max_bit = usable_top / PAGESIZE;
    for (size_t i = last_bit; i < max_bit; ++i) {
        if (TESTBIT(bitmap, i)) {
            frames_found = 0;
            start_idx = -1;
            continue;
        }

        /* Keep track of region start */
        if (start_idx < 0) {
            start_idx = i;
        }

        /* Did we find the requested count? */
        if ((frames_found++) >= count) {
            phys = start_idx * PAGESIZE;
            break;
        }
    }

    if (phys != 0) {
        for (size_t i = start_idx; i < start_idx + count; ++i) {
            SETBIT(bitmap, i);
        }
    }

    return phys;
}

uintptr_t
mm_pmem_alloc(size_t count)
{
    uintptr_t phys;

    spinlock_acquire(&bitmap_lock, true);
    phys = pmem_alloc(count);
    if (phys == 0) {
        last_bit = 0;
        phys = pmem_alloc(count);
    }

    spinlock_release(&bitmap_lock);
    return phys;
}

void
mm_pmem_free(uintptr_t base, size_t count)
{
    uintptr_t range_end;

    spinlock_acquire(&bitmap_lock, true);
    range_end = base + (count * PAGESIZE);
    bitmap_set_range(base, range_end, false);
    spinlock_release(&bitmap_lock);
}

void
mm_pmem_init(void)
{
    dtrace("probing physical memory...\n");
    pmem_probe();

    dtrace("allocating bitmap...\n");
    pmem_alloc_bitmap();
}
