/*
 * Copyright (c) 2024, Texas Instruments Incorporated - http://www.ti.com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ======== gcc_stubs.c ========
 *  This file provides reference implementations of potential low-level system calls
 *  that the GCC newlib runtime may invoke.  The implementations in this file may be used
 *  by examples and reference code, but should be considered non-production-quality as
 *  they intentionally set errno to failure codes and do nothing.  Each GCC_based
 *  application should review their specific needs for these services and implement them
 *  to meet their needs.  Consult GCC documentation for more details.
 */

#include <stddef.h>
#include <stdint.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

__attribute__((weak)) int _close(int fildes)
{
    errno = ENOSYS;
    return -1;
}
__attribute__((weak)) int _fstat(int fildes, struct stat *st)
{
    errno = ENOSYS;
    return -1;
}
__attribute__((weak)) int _getpid(void)
{
    errno = ENOSYS;
    return -1;
}
__attribute__((weak)) int _isatty(int file)
{
    errno = ENOSYS;
    return 0;
}
__attribute__((weak)) int _kill(int pid, int sig)
{
    errno = ENOSYS;
    return -1;
}
__attribute__((weak)) int _lseek(int file, int ptr, int dir)
{
    errno = ENOSYS;
    return -1;
}
__attribute__((weak)) int _read(int file, char *ptr, int len)
{
    errno = ENOSYS;
    return -1;
}
__attribute__((weak)) int _write(int file, char *ptr, int len)
{
    errno = ENOSYS;
    return -1;
}
__attribute__((weak)) void _exit(int rc)
{
    /* Convince GCC that this function never returns.  */
    while (1) {}
}