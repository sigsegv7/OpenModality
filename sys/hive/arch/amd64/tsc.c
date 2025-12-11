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
#include <mu/timer.h>
#include <core/panic.h>
#include <ob/dir.h>
#include <md/tsc.h>

static struct knode *tsc_knp = NULL;

/* Forward declarations */
static struct timer_hooks tsc_hooks;

/*
 * Get the current counter value from the
 * processor's TSC.
 */
static size_t
tsc_get_count(struct timer *timer)
{
    uint32_t lo, hi;

    if (timer == NULL) {
        return 0;
    }

    ASMV(
        "rdtsc"
        : "=d" (hi), "=a" (lo)
        :
        : "memory"
    );

    return ((uint64_t)hi << 32) | lo;
}

void
tsc_init(struct knode *clkdev_root)
{
    int error;

    if (clkdev_root == NULL) {
        panic("tsc: bad clkdev root\n");
    }

    error = ob_knode_new("tsc", K_CLKDEV, &tsc_knp);
    if (error != 0) {
        panic("tsc: could not register TSC\n");
    }

    tsc_knp->data = &tsc_hooks;
    error = ob_dir_append(tsc_knp, clkdev_root);
    if (error != 0) {
        panic("tsc: could not add TSC knode\n");
    }

}

static struct timer_hooks tsc_hooks = {
    .get_count = tsc_get_count
};
