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

#include <sys/mman.h>
#include <sys/param.h>
#include <sys/errno.h>
#include <sys/elf.h>
#include <sys/types.h>
#include <mm/memvar.h>
#include <mm/vmem.h>
#include <mm/pmem.h>
#include <lib/stdbool.h>
#include <lib/string.h>
#include <core/elfload.h>

/*
 * Verify that an ELF file to be loaded is valid
 * and return true if so.
 */
static bool
elf_verify(const Elf64_Ehdr *eh)
{
    if (eh == NULL) {
        return false;
    }

    /* Must have the magic bytes to be a valid ELF */
    if (memcmp(&eh->e_ident[EI_MAG0], ELFMAG, SELFMAG) != 0) {
        return false;
    }

    /* This must be executable */
    if (eh->e_type != ET_EXEC) {
        return false;
    }

    /* Must be EV_CURRENT, drop it otherwise */
    if (eh->e_version != EV_CURRENT) {
        return false;
    }

    return true;
}

static int
elf_load(struct mu_vas *vas, Elf64_Ehdr *eh, struct loaded_elf *res)
{
    struct vmem_region region;
    Elf64_Phdr *phdr_base, *phdr;
    void *dest, *src;
    off_t misalign;
    uintptr_t pma_base;
    int error;
    int prot, retval = 0;

    if (vas == NULL || eh == NULL) {
        return -EINVAL;
    }

    if (res == NULL) {
        return -EINVAL;
    }

    phdr_base = PTR_OFFSET(eh, eh->e_phoff);
#define PHDR_INDEX(INDEX) \
    PTR_OFFSET(phdr_base, eh->e_phentsize * (INDEX))

    for (int i = 0; i < eh->e_phnum; ++i) {
        phdr = PHDR_INDEX(i);

        /* Drop non-loadable sections */
        if (phdr->p_type != PT_LOAD)
            continue;

        /* Drop empty segments */
        if (phdr->p_memsz == 0)
            continue;

        /* Set the appropriate protection flags */
        prot = PROT_READ | PROT_USER;
        if (ISSET(phdr->p_flags, PF_W))
            prot |= PROT_WRITE;
        if (ISSET(phdr->p_flags, PF_X))
            prot |= PROT_EXEC;

        /*
         * Next, we construct the region descriptor and allocate
         * the required memory before we begin mapping.
         */
        misalign = phdr->p_vaddr & (PAGESIZE - 1);
        region.length = ALIGN_UP(phdr->p_memsz + misalign, PAGESIZE);
        region.vma = phdr->p_vaddr;
        region.pma = mm_pmem_alloc(region.length / PAGESIZE);
        if (region.pma == 0) {
            retval = -ENOMEM;
            break;
        }

        /*
         * Now we map the full range of memory, this will be mapped
         * into the given virtual address space so we'll need to do
         * some HHDM magic to copy the data.
         */
        error = vmem_map_region(vas, &region, prot);
        if (error != 0) {
            retval = error;
            break;
        }

        /* Now copy the segment into memory */
        dest = PHYS_TO_VIRT(region.pma);
        src = PTR_OFFSET(eh, phdr->p_offset);
        memcpy(dest, src, phdr->p_memsz);
    }
#undef PHDR_INDEX
    return retval;
}

int
elf_load_raw(struct mu_vas *vas, void *image, struct loaded_elf *res)
{
    Elf64_Ehdr *eh;
    int error;

    if (vas == NULL || image == NULL) {
        return -EINVAL;
    }

    if (res == NULL) {
        return -EINVAL;
    }

    /* Ensure that the image is valid */
    eh = (Elf64_Ehdr *)image;
    if (!elf_verify(eh)) {
        return false;
    }

    error = elf_load(vas, eh, res);
    if (error != 0) {
        return error;
    }

    res->entrypoint = eh->e_entry;
    return 0;
}
