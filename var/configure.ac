#
# Copyright (c) 2025 Ian Marco Moffett and the OpenModality engineers
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice,
#    this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. Neither the name of the project nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#

AC_INIT([OpenModality], [0.0.1], [ian@osmora.org])
AC_PROG_CC

BOOT_PROTO="limine"
SDKROOT=$(pwd)/sdk/

CC=$(pwd)/cc/gcc/bin/x86_64-pc-osmora-gcc
LD=$(pwd)/cc/toolchain/build-binutils/bin/x86_64-pc-osmora-ld
ARCH="amd64"

MD_CFLAGS="-mno-sse -mno-sse2 -mno-sse3 -mno-mmx -mno-3dnow -mno-avx -mno-avx2 -mno-80387"
CFLAGS="-Wno-attributes -nostdlib -nostdinc -ffreestanding  -mcmodel=kernel -fno-stack-protector \\
        --std=gnu11 -fexceptions -D_HIVE $MD_CFLAGS"

AC_SUBST(SYS_CFLAGS, [$CFLAGS])
AC_SUBST(SDKROOT, [$SDKROOT])
AC_SUBST(BOOT_PROTO, [$BOOT_PROTO])
AC_SUBST(CC, [$CC])
AC_SUBST(LD, [$LD])
AC_SUBST(ARCH, [$ARCH])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
