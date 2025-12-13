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
#include <core/bpt.h>
#include <boot/limine.h>
#include <lib/string.h>

/* Memory map */
static struct limine_memmap_response *memmap_resp;
static volatile struct limine_memmap_request memmap_req = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};

/* Higher half direct map */
static struct limine_hhdm_response *hhdm_resp;
static volatile struct limine_hhdm_request hhdm_req = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0
};

/* ACPI RSDP */
static struct limine_rsdp_response *rsdp_resp;
static volatile struct limine_rsdp_request rsdp_req = {
    .id = LIMINE_RSDP_REQUEST,
    .revision = 0
};

/* Module request */
static struct limine_module_response *mod_resp;
static volatile struct limine_module_request mod_req = {
    .id = LIMINE_MODULE_REQUEST,
    .revision = 0
};

static int
limine_get_module(const char *name, struct bpt_module *res)
{
    struct limine_file *file;

    if (name == NULL || res == NULL) {
        return -EINVAL;
    }

    for (size_t i = 0; i < mod_resp->module_count; ++i) {
        file = mod_resp->modules[i];
        if (strcmp(file->path, name) == 0) {
            res->address = file->address;
            res->length = file->size;
            return 0;
        }
    }

    return -ENOENT;
}

/*
 * Acquire static / unchanging variables from the
 * bootloader
 */
static int
limine_get_vars(struct bpt_vars *vars)
{
    if (vars == NULL) {
        return -1;
    }

    vars->kernel_base = hhdm_resp->offset;
    vars->rsdp_base = rsdp_resp->address;
    return 0;
}

static int
limine_get_mementry(size_t index, struct bpt_mementry *res)
{
    struct limine_memmap_entry *entry;

    if (res == NULL || index >= memmap_resp->entry_count) {
        return -1;
    }

    entry = memmap_resp->entries[index];
    res->base = entry->base;
    res->length = entry->length;
    res->type = entry->type;    /* 1:1 */
    return 0;
}

int
bpt_init_limine(struct bpt_hooks *hooks)
{
    if (hooks == NULL) {
        return -1;
    }

    /* Load responses */
    hhdm_resp = hhdm_req.response;
    memmap_resp = memmap_req.response;
    rsdp_resp = rsdp_req.response;
    mod_resp = mod_req.response;

    /* Set hooks */
    hooks->get_vars = limine_get_vars;
    hooks->get_mementry = limine_get_mementry;
    hooks->get_module = limine_get_module;
    return 0;
}
