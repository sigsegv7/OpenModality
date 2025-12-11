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

#ifndef _MU_TIMER_H_
#define _MU_TIMER_H_ 1

#define TIMER_NAMELEN 32

/* Forward declaration */
struct timer;

/*
 * Represents the various kinds of timer
 * types that may be found on the system.
 *
 * @TIMER_LOCAL:   Processor local timer (e.g., x86 TSC)
 * @TIMER_GENERIC: Generic system timer
 * @TIMER_SCHED:   Scheduler timer
 */
typedef enum {
    TIMER_LOCAL,
    TIMER_GENERIC,
    TIMER_SCHED
} timer_type_t;

/*
 * Represents the collection of hooks that may
 * be invoked on a specific timer.
 */
struct timer_hooks {
    size_t(*get_count)(struct timer *timer);
};

/*
 * Represents a kind of timer attached
 * to the machine.
 *
 * @name:  Name of this timer
 * @hooks: Hooks that may be invoked on a specific timer
 * @type:  The type of this timer
 */
struct timer {
    char name[TIMER_NAMELEN];
    struct timer_hooks hooks;
    timer_type_t type;
};

/*
 * Initialize machine specific timer subsystems one
 * after the other.
 */
void mu_timer_init(void);

#endif  /* !_MU_TIMER_H_ */
