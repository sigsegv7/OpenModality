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

#include <sys/param.h>
#include <sys/units.h>
#include <mu/pmap.h>
#include <mm/pmem.h>
#include <mm/memvar.h>
#include <lib/stdbool.h>
#include <lib/string.h>

/*
 * Page-Table Entry (PTE) flags
 *
 * See Intel SDM Vol 3A, Section 4.5, Table 4-19
 */
#define PTE_ADDR_MASK   0x000FFFFFFFFFF000
#define PTE_P           BIT(0)        /* Present */
#define PTE_RW          BIT(1)        /* Writable */
#define PTE_US          BIT(2)        /* User r/w allowed */
#define PTE_PWT         BIT(3)        /* Page-level write-through */
#define PTE_PCD         BIT(4)        /* Page-level cache disable */
#define PTE_ACC         BIT(5)        /* Accessed */
#define PTE_DIRTY       BIT(6)        /* Dirty (written-to page) */
#define PTE_PS          BIT(7)        /* Page size */
#define PTE_GLOBAL      BIT(8)        /* Global / sticky map */
#define PTE_NX          BIT(63)       /* Execute-disable */

#define CR4_LA57 BIT(12)  /* 5-level paging */

/*
 * Represents various paging structure
 * levels for translation
 */
typedef enum {
    PMAP_PML1,
    PMAP_PML2,
    PMAP_PML3,
    PMAP_PML4,
    PMAP_PML5
} pmap_level_t;

/*
 * Used to convert page size definitions to
 * their respective alignment boundaries.
 */
static size_t mem_pstab[] = {
    [PAGESIZE_4K] = 4096,
    [PAGESIZE_2M] = UNIT_MIB * 2,
    [PAGESIZE_1G] = UNIT_GIB
};

/*
 * Returns true if the given pagesize is valid.
 *
 * TODO: Support huge pages
 */
static inline bool
is_ps_valid(pagesize_t size)
{
    switch (size) {
    case PAGESIZE_4K:
        return true;
    }

    return false;
}

/*
 * Acquire the index of the top-level that
 * the CR3 register references
 */
static inline pmap_level_t
pmap_toplevel(void)
{
    uint64_t cr4;

    ASMV(
        "mov %%cr4, %0"
        : "=r" (cr4)
        :
        : "memory"
    );

    return ISSET(cr4, CR4_LA57) ? PMAP_PML5 : PMAP_PML4;
}

static inline void
pmap_invlpg(uintptr_t vma)
{
    ASMV(
        "invlpg (%0)"
        :
        : "r" (vma)
        : "memory"
    );
}

int
mu_pmap_readvas(struct mu_vas *res)
{
    if (res == NULL) {
        return -1;
    }

    ASMV(
        "mov %%cr3, %0"
        : "=r" (res->cr3)
        :
        : "memory"
    );

    return 0;
}

/*
 * Convert system protection flags into machine
 * page table bits
 */
static inline size_t
prot_to_pte(uint16_t prot)
{
    size_t pte_flags = PTE_P | PTE_NX;

    if (ISSET(prot, PROT_WRITE))
        pte_flags |= PTE_RW;
    if (ISSET(prot, PROT_EXEC))
        pte_flags &= ~PTE_NX;
    if (ISSET(prot, PROT_USER))
        pte_flags |= PTE_US;

    return pte_flags;
}

/*
 * Extract a paging structure level index from a
 * virtual memory address for translation.
 */
static size_t
vma_level_index(uintptr_t vma, pmap_level_t level)
{
    switch (level) {
    case PMAP_PML5: return (vma >> 47) & 0x3FF;
    case PMAP_PML4: return (vma >> 39) & 0x1FF;
    case PMAP_PML3: return (vma >> 30) & 0x1FF;
    case PMAP_PML2: return (vma >> 21) & 0x1FF;
    case PMAP_PML1: return (vma >> 12) & 0x1FF;
    }

    return (size_t)-1;
}

/*
 * Acquire the base of a paging structure within a specific
 * virtual address space by performing iterative descent style
 * traversal for translation. This will use bits as indices
 * derived from the virtual memory address per level.
 */
static uint64_t *
vma_level_base(struct mu_vas *vas, uintptr_t vma, pmap_level_t lvl, bool alloc)
{
    pmap_level_t cur_lvl = pmap_toplevel();
    uintptr_t *cur_base, phys;
    size_t index;

    if (vas == NULL) {
        return NULL;
    }

    /* Acquire the top-level structure */
    phys = vas->cr3 & PTE_ADDR_MASK;
    cur_base = PHYS_TO_VIRT(phys);

    while (cur_lvl > lvl) {
        index = vma_level_index(vma, cur_lvl);

        /* Is this entry present? */
        if (ISSET(cur_base[index], PTE_P)) {
            phys = cur_base[index] & PTE_ADDR_MASK;
            cur_base = PHYS_TO_VIRT(phys);
            --cur_lvl;
            continue;
        }

        /*
         * If we are not to allocate any entries, return a value
         * of NULL to indicate failure.
         */
        if (!alloc) {
            return NULL;
        }

        phys = mm_pmem_alloc(1);
        if (phys == 0) {
            return NULL;
        }

        memset(PHYS_TO_VIRT(phys), 0, PAGESIZE);
        cur_base[index] = phys | (PTE_P | PTE_RW | PTE_US);

        cur_base = PHYS_TO_VIRT(phys);
        --cur_lvl;
    }

    return cur_base;
}

int
mu_pmap_map(struct mu_vas *vas, uintptr_t vma, uintptr_t pma, int prot,
    pagesize_t ps)
{
    uintptr_t *pgtbl;
    size_t index, pte_flags;

    if (vas == NULL || !is_ps_valid(ps)) {
        return -1;
    }

    vma = ALIGN_DOWN(vma, mem_pstab[ps]);
    pgtbl = vma_level_base(vas, vma, PMAP_PML1, true);
    if (pgtbl == NULL) {
        return -1;
    }

    pte_flags = prot_to_pte(prot);
    index = vma_level_index(vma, PMAP_PML1);
    pgtbl[index] = pma | pte_flags;
    pmap_invlpg(vma);
    return 0;
}
