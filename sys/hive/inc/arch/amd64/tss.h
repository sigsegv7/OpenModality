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

#ifndef _MACHINE_TSS_H_
#define _MACHINE_TSS_H_ 1

#include <sys/types.h>
#include <sys/cdefs.h>

#define TSS_TYPE  0x09

/*
 * 64-bit task state segment entry.
 *
 * XXX: See section 7.7 of the Intel SDM
 */
struct PACKED tss_entry {
    uint32_t reserved;
    uint32_t rsp0_low;
    uint32_t rsp0_high;
    uint32_t rsp1_low;
    uint32_t rsp1_high;
    uint32_t rsp2_low;
    uint32_t rsp2_high;
    uint32_t reserved1;
    uint32_t reserved2;
    uint32_t ist1_low;
    uint32_t ist1_high;
    uint32_t ist2_low;
    uint32_t ist2_high;
    uint32_t ist3_low;
    uint32_t ist3_high;
    uint32_t ist4_low;
    uint32_t ist4_high;
    uint32_t ist5_low;
    uint32_t ist5_high;
    uint32_t ist6_low;
    uint32_t ist6_high;
    uint32_t ist7_low;
    uint32_t ist7_high;
    uint32_t reserved3;
    uint32_t reserved4;
    uint16_t reserved5;
    uint16_t iomap;
};

struct PACKED tss_desc {
    uint16_t seglim_low;
    uint16_t base_low;
    uint8_t base_mid;
    uint8_t type        : 4;
    uint8_t zero        : 1;
    uint8_t dpl         : 2;
    uint8_t p           : 1;
    uint8_t seglim_high : 4;
    uint8_t avl         : 1;
    uint8_t zero1       : 2;
    uint8_t gran        : 1;
    uint8_t base_mid1;
    uint32_t base_high;
    uint8_t reserved;
    uint8_t zero2       : 5;
    uint32_t reserved1  : 19;
};

/*
 * Allocate a task state segment and fill in
 * its respective TSS descriptor structure
 *
 * @tss: TSS to use (NULL to auto-allocate)
 * @desc: Descriptor that references TSS
 *
 * Returns zero on success
 */
int md_tss_init(struct tss_entry *tss, struct tss_desc *desc);

#endif  /* !_MACHINE_TSS_H_ */
