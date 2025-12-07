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

#ifndef _SYS_TYPES_H_
#define _SYS_TYPES_H_ 1

/* Unsigned types */
typedef unsigned char       __uint8_t;
typedef unsigned short      __uint16_t;
typedef unsigned int        __uint32_t;
typedef unsigned long long  __uint64_t;
typedef __uint64_t          __size_t;
typedef __size_t            __uintptr_t;
typedef __size_t            __off_t;

/* Signed types */
typedef char            __int8_t;
typedef short           __int16_t;
typedef int             __int32_t;
typedef long long       __int64_t;
typedef __int64_t       __ssize_t;
typedef __ssize_t       __ptrdiff_t;

/* ID types */
typedef __size_t  id_t;
typedef id_t      pid_t;

#if defined(_HIVE) || defined(_OM_SOURCE)
#define NULL (void *)0

typedef __uint8_t   uint8_t;
typedef __uint16_t  uint16_t;
typedef __uint32_t  uint32_t;
typedef __uint64_t  uint64_t;
typedef __size_t    size_t;
typedef __uintptr_t uintptr_t;
typedef __off_t     off_t;

typedef __int8_t    int8_t;
typedef __int16_t   int16_t;
typedef __int32_t   int32_t;
typedef __int64_t   int64_t;
typedef __ssize_t   ssize_t;
typedef __ptrdiff_t ptrdiff_t;
#endif  /* _HIVE || _OM_SOURCE */

#endif  /* !_SYS_TYPES_H_ */
