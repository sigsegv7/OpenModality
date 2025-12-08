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

#ifndef _MACHINE_MSR_H_
#define _MACHINE_MSR_H_ 1

#include <sys/cdefs.h>

#define IA32_APIC_BASE      0x0000001B
#define IA32_GS_BASE        0xC0000101
#define IA32_MTRR_CAP       0x000000FE
#define IA32_DEF_TYPE       0x000002FF
#define IA32_MTRR_PHYSBASE  0x00000200
#define IA32_MTRR_PHYSMASK  0x00000201
#define IA32_KERNEL_GS_BASE 0xC0000102
#define IA32_EFER           0xC0000080

ALWAYS_INLINE static inline void
md_wrmsr(uint32_t msr, uint64_t v)
{
    uint32_t lo, hi;

    lo = v & 0xFFFFFFFF;
    hi = (v >> 32) & 0xFFFFFFFF;

    ASMV(
        "wrmsr"
        :
        : "c" (msr), "a" (lo), "d" (hi)
        : "memory"
    );
}

ALWAYS_INLINE static inline uint64_t
md_rdmsr(uint32_t msr)
{
    uint32_t lo, hi;

    ASMV(
        "rdmsr"
        : "=a" (lo), "=d" (hi)
        : "c" (msr)
        : "memory"
    );

    return ((uint64_t)hi << 32) | lo;
}

#endif  /* !_MACHINE_MSR_H_ */
