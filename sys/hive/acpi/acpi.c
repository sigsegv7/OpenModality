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
#include <sys/errno.h>
#include <lib/string.h>
#include <core/bpt.h>
#include <core/trace.h>
#include <core/panic.h>
#include <acpi/acpi.h>
#include <acpi/tables.h>
#include <mm/memvar.h>

#define dtrace(fmt, ...) printf("acpi: " fmt, ##__VA_ARGS__)

static struct acpi_rsdp *rsdp;
static struct acpi_root_sdt *sdt;
static size_t sdt_entries = 0;

/*
 * Display the OEMID to the console
 */
static void
acpi_print_oemid(struct acpi_rsdp *rsdp)
{
    char oemid[OEMID_SIZE + 1];
    uint8_t rev;

    memcpy(oemid, rsdp->oemid, sizeof(oemid) - 1);
    oemid[OEMID_SIZE] = '\0';

    /* Some emulators use 0, bump if this happens */
    if ((rev = rsdp->revision) == 0) {
        ++rev;
    }

    dtrace("DETECTED ACPI %d.0", rev);
    dtrace("VENDOR: %s", oemid);
}

/*
 * Verify the checksum of an ACPI header and return
 * zero if valid
 */
static int
acpi_checksum(struct acpi_header *hdr)
{
    uint8_t csum = 0;

    for (int i = 0; i < hdr->length; ++i) {
        csum += ((char *)hdr)[i];
    }

    return (csum == 0) ? 0 : -1;
}

void *
acpi_query(const char *s)
{
    struct acpi_header *hdr;

    for (int i = 0; i < sdt_entries; ++i) {
        hdr = PHYS_TO_VIRT((uintptr_t)sdt->tables[i]);
        if (memcmp(hdr->signature, s, 4) == 0) {
            return acpi_checksum(hdr) == 0 ? (void *)hdr : NULL;
        }
    }

    return NULL;
}

int
acpi_read_madt(uint32_t type, int(*cb)(struct apic_header *, size_t), size_t arg)
{
    static struct acpi_madt *madt;
    struct apic_header *hdr;
    uint8_t *cur, *end;
    int retval;

    if (cb == NULL) {
        return -EINVAL;
    }

    if (madt == NULL) {
        madt = acpi_query("APIC");
    }

    if (madt == NULL) {
        panic("acpi madt not found");
    }

    cur = (uint8_t *)(madt + 1);
    end = (uint8_t *)madt + madt->hdr.length;

    while (cur < end) {
        hdr = (void *)cur;

        if (hdr->type == type) {
            retval = cb(hdr, arg);
        }

        if (retval >= 0) {
            return retval;
        }

        cur += hdr->length;
    }

    return -1;
}

static void
acpi_print_rsdp(void)
{
    char oemid[OEMID_SIZE + 1];

    /*
     * On some emulators the revision might be read
     * as zero. Bump it up by one if this is the case.
     */
    if (rsdp->revision == 0) {
        rsdp->revision = 1;
    }

    oemid[OEMID_SIZE] = '\0';
    memcpy(oemid, rsdp->oemid, OEMID_SIZE);
    dtrace("acpi %d.0 by %s\n", rsdp->revision, oemid);
}

void
acpi_init(void)
{
    struct bpt_vars vars;
    int error;

    error = bpt_get_vars(&vars);
    if (error != 0) {
        panic("acpi: could not get bpt vars\n");
    }

    if ((rsdp = vars.rsdp_base) == NULL) {
        panic("acpi: could not get rsdp\n");
    }

    acpi_print_rsdp();
    if (rsdp->revision >= 2) {
        sdt = PHYS_TO_VIRT(rsdp->xsdt_addr);
        sdt_entries = (sdt->hdr.length - sizeof(sdt->hdr)) / 8;
        dtrace("using xsdt as root sdt\n");
    } else {
        sdt = PHYS_TO_VIRT(rsdp->rsdt_addr);
        sdt_entries = (sdt->hdr.length - sizeof(sdt->hdr)) / 4;
        dtrace("using rsdt as root sdt\n");
    }
}
