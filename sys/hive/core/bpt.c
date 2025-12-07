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

/*
 * Description: Boot protocol translation layer
 * Author: Ian Moffett
 */

#include <sys/types.h>
#include <core/bpt.h>
#include <lib/string.h>

#if defined(__BOOT_PROTO)
#define BOOT_PROTO __BOOT_PROTO
#else
#error "BOOT PROTOCOL NOT DEFINED"
#endif  /* __BOOT_PROTO */

static struct bpt_hooks hooks;

int
bpt_get_vars(struct bpt_vars *res)
{
    if (hooks.get_vars == NULL) {
        return -1;
    }

    return hooks.get_vars(res);
}

int
bpt_get_mementry(size_t index, struct bpt_mementry *res)
{
    if (hooks.get_mementry == NULL) {
        return -1;
    }

    return hooks.get_mementry(index, res);
}

int
bpt_init(void)
{
    /*
     * Initialize the hooks for the currently in-use
     * boot protocol
     */
    switch (*BOOT_PROTO) {
    case 'l':
        if (strcmp(BOOT_PROTO, BPT_SIG_LIMINE) == 0) {
            return bpt_init_limine(&hooks);
        }
        break;
    }

    return -1;
}
