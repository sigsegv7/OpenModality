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

#ifndef _CORE_BPT_H_
#define _CORE_BPT_H_ 1

#include <sys/cdefs.h>
#include <sys/types.h>

/* Boot protocol signatures */
#define BPT_SIG_LIMINE "limine"

/*
 * Represents valid memory map entry
 * types
 */
typedef enum {
    MEM_USABLE,
    MEM_RESERVED,
    MEM_ACPI_RECLAIM,
    MEM_ACPI_NVS,
    MEM_BAD,
    MEM_BOOTLOADER,
    MEM_KERNEL,
    MEM_FRAMEBUFFER
} mem_type_t;

/*
 * Represents static variables provided by the
 * boot protocol currently in-use.
 *
 * @kernel_base: Virtual kernel load base
 */
struct bpt_vars {
    uintptr_t kernel_base;
};

/*
 * Represents a memory map entry
 *
 * @base: Base of memory area
 * @length: Length of memory area
 * @type: Memory area type
 */
struct bpt_mementry {
    uintptr_t base;
    size_t length;
    mem_type_t type;
};

/*
 * Represents boot protocol translation hooks that
 * interface with the underlying boot protocol.
 */
struct bpt_hooks {
    int(*get_vars)(struct bpt_vars *res);
    int(*get_mementry)(size_t index, struct bpt_mementry *res);
};

/*
 * Acquire static boot protocol variables
 *
 * @res: Result is written here
 *
 * Returns zero on success
 */
int bpt_get_vars(struct bpt_vars *res);

/*
 * Acquire a memory map entry by index
 *
 * Returns zero on success, non-zero values indicate end
 * of memory entry list or error.
 */
int bpt_get_mementry(size_t index, struct bpt_mementry *res);

/*
 * Initialize the boot protocol translation layer
 */
int bpt_init(void);

/*
 * Protocol specific init routines
 */
int bpt_init_limine(struct bpt_hooks *hooks);

/*
 * Shorthand to acquire the kernel load base, used in
 * macros for converting higher half to lower half and
 * vice versa.
 */
ALWAYS_INLINE static inline uintptr_t
bpt_kernel_base(void)
{
    struct bpt_vars vars;

    if (bpt_get_vars(&vars) != 0) {
        return 0;
    }

    return vars.kernel_base;
}

#endif  /* !_CORE_BPT_H_ */
