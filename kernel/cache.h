/* SPDX-License-Identifier: GPL-2.0 */
/*
 * linuxab/arch/x86/include/asm/cache.h
 * x86_64 cache architecture
 */

#ifndef _LINUXAB_X86_CACHE_H
#define _LINUXAB_X86_CACHE_H

#define L1_CACHE_SHIFT      6
#define L1_CACHE_BYTES      (1 << L1_CACHE_SHIFT)   /* 64 bytes */
#define L2_CACHE_BYTES      128
#define INTERNODE_CACHE_SHIFT   L1_CACHE_SHIFT

#define SMP_CACHE_BYTES     L1_CACHE_BYTES

#define __read_mostly       __attribute__((__section__(".data.read_mostly")))
#define __ro_after_init     __attribute__((__section__(".data.ro_after_init")))

#define ARCH_KMALLOC_MINALIGN   L1_CACHE_BYTES

#ifndef __aligned
#define __aligned(x)        __attribute__((__aligned__(x)))
#endif

#ifndef ____cacheline_aligned
#define ____cacheline_aligned   __attribute__((__aligned__(SMP_CACHE_BYTES)))
#endif

#define ____cacheline_aligned_in_smp    ____cacheline_aligned

#define __cacheline_aligned     ____cacheline_aligned

#endif /* _LINUXAB_X86_CACHE_H */
