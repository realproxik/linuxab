/* SPDX-License-Identifier: GPL-2.0 */
/*
 * linuxab/arch/x86/include/asm/string.h
 * x86_64 optimized string operations
 */

#ifndef _LINUXAB_X86_STRING_H
#define _LINUXAB_X86_STRING_H

#include <stddef.h>
#include <stdint.h>

static inline void *memcpy(void *dest, const void *src, size_t n)
{
    __asm__ volatile ("rep movsb"
                      : "+D" (dest), "+S" (src), "+c" (n)
                      :: "memory");
    return dest;
}

static inline void *memset(void *s, int c, size_t n)
{
    __asm__ volatile ("rep stosb"
                      : "+D" (s), "+c" (n)
                      : "a" (c)
                      : "memory");
    return s;
}

static inline int memcmp(const void *s1, const void *s2, size_t n)
{
    uint8_t res;
    __asm__ volatile ("repe cmpsb\n\t"
                      "setne %0"
                      : "=qm" (res), "+D" (s1), "+S" (s2), "+c" (n)
                      :: "memory");
    return res;
}

static inline void *memmove(void *dest, const void *src, size_t n)
{
    if (dest < src)
        return memcpy(dest, src, n);
    
    __asm__ volatile ("std\n\t"
                      "rep movsb\n\t"
                      "cld"
                      : "+D" ((char *)dest + n - 1),
                        "+S" ((char *)src + n - 1),
                        "+c" (n)
                      :: "memory");
    return dest;
}

static inline size_t strlen(const char *s)
{
    size_t len;
    __asm__ volatile ("repne scasb\n\t"
                      "not %0\n\t"
                      "dec %0"
                      : "=c" (len)
                      : "D" (s), "a" (0), "0" (~0UL)
                      : "memory");
    return len;
}

static inline int strcmp(const char *s1, const char *s2)
{
    int res;
    __asm__ volatile ("1: lodsb\n\t"
                      "scasb\n\t"
                      "jne 2f\n\t"
                      "testb %%al, %%al\n\t"
                      "jne 1b\n\t"
                      "xorl %%eax, %%eax\n\t"
                      "jmp 3f\n"
                      "2: sbbl %%eax, %%eax\n\t"
                      "orb $1, %%al\n"
                      "3:"
                      : "=a" (res), "+D" (s1), "+S" (s2)
                      :: "memory");
    return res;
}

static inline char *strcpy(char *dest, const char *src)
{
    char *tmp = dest;
    while ((*dest++ = *src++) != '\0');
    return tmp;
}

static inline char *strncpy(char *dest, const char *src, size_t n)
{
    size_t i;
    for (i = 0; i < n && src[i] != '\0'; i++)
        dest[i] = src[i];
    for (; i < n; i++)
        dest[i] = '\0';
    return dest;
}

static inline int strncmp(const char *s1, const char *s2, size_t n)
{
    while (n && *s1 && (*s1 == *s2)) {
        s1++; s2++; n--;
    }
    if (n == 0) return 0;
    return *(unsigned char *)s1 - *(unsigned char *)s2;
}

#endif /* _LINUXAB_X86_STRING_H */
