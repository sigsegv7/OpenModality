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

#include <sys/cdefs.h>
#include <core/bpt.h>
#include <core/trace.h>
#include <core/panic.h>
#include <core/timer.h>
#include <core/initrd.h>
#include <core/elfload.h>
#include <acpi/acpi.h>
#include <os/pool.h>
#include <ob/dir.h>
#include <mu/cpu.h>
#include <mu/pmap.h>
#include <mm/pmem.h>
#include <mm/memvar.h>

#define RTS_PATH "/sbin/rts"

static struct pcr bsp;

void kmain(void);

NORETURN static void
start_rts(void)
{
    uintptr_t stack;
    struct loaded_elf elf;
    struct mu_vas vas;
    void *data;
    int error;

    if (mu_pmap_readvas(&vas) != 0) {
        panic("hive: unable to read VAS for loading\n");
    }

    if ((stack = mm_pmem_alloc(1)) == 0) {
        panic("hive: unable to allocate user stack\n");
    }

    error = mu_pmap_map(
        &vas,
        stack,
        stack,
        PROT_READ | PROT_WRITE | PROT_USER,
        PAGESIZE_4K
    );

    if (error != 0) {
        panic("hive: unable to map stack\n");
    }

    if ((data = initrd_lookup(RTS_PATH)) == NULL) {
        panic("hive: unable to lookup \"%s\"\n", RTS_PATH);
    }

    if ((elf_load_raw(&vas, data, &elf)) != 0) {
        panic("hive: unable to load \"%s\"\n", RTS_PATH);
    }

    stack += PAGESIZE - 1;
    mu_proc_uvector(elf.entrypoint, stack);
}

void
kmain(void)
{
    /* Initialize boot protocol translation */
    if (bpt_init() != 0) {
        return;
    }

    printf("hive: engaging pmem...\n");
    mm_pmem_init();

    printf("hive: engaging root pool...\n");
    os_pool_init();

    printf("hive: engaging pmap layer...\n");
    mu_pmap_init();

    printf("hive: configuring bsp...\n");
    mu_cpu_conf(&bsp);

    printf("hive: engaging object store...\n");
    ob_store_init();

    printf("hive: engaging timers...\n");
    timer_init();

    printf("hive: engaging acpi...\n");
    acpi_init();

    printf("hive: engaging initrd...\n");
    initrd_init();

    printf("hive: bringing up rts...\n");
    start_rts();
}
