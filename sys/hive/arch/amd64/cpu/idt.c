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
#include <sys/cdefs.h>
#include <md/idt.h>
#include <md/gdt.h>

static struct idt_gate idt[256];
static struct idtr idtr = {
    .limit = sizeof(idt) - 1,
    .offset = (uintptr_t)&idt[0]
};

void
md_idt_set(uint8_t vector, uint8_t type, uintptr_t isr, uint8_t ist)
{
    struct idt_gate *gate;

    gate = &idt[vector];
    gate->offset_low = isr & 0xFFFF;
    gate->offset_mid = (isr >> 16) & 0xFFFF;
    gate->offset_high = (isr >> 32) & 0xFFFFFFFF;
    gate->target_cs = GDT_KCODE;
    gate->ist = ist;
    gate->zero = 0;
    gate->zero1 = 0;
    gate->reserved = 0;
    gate->p = 1;
    gate->dpl = (type == IDT_USER_GATE) ? 3 : 0;
    gate->type = type;
}

void
md_idt_load(void)
{
    ASMV(
        "lidt %0"
        :
        : "m" (idtr)
        : "memory"
    );
}
