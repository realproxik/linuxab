/* SPDX-License-Identifier: GPL-2.0 */
/*
 * linuxab/arch/x86/include/asm/tlbflush.h
 * x86_64 TLB management
 */

#ifndef _LINUXAB_X86_TLBFLUSH_H
#define _LINUXAB_X86_TLBFLUSH_H

#include "processor.h"
#include "page.h"

/* Flush single page */
static inline void __flush_tlb_one(unsigned long addr)
{
    __asm__ volatile ("invlpg (%0)" :: "r" (addr) : "memory");
}

/* Flush all user TLB entries */
static inline void __flush_tlb(void)
{
    __asm__ volatile ("mov %cr3, %rax\n\t"
                      "mov %rax, %cr3"
                      ::: "rax", "memory");
}

/* Flush all (kernel + user) */
static inline void __flush_tlb_all(void)
{
    __asm__ volatile ("mov %cr4, %rax\n\t"
                      "and $0xffffff7f, %eax\n\t"
                      "mov %rax, %cr4\n\t"
                      "or $0x80, %rax\n\t"
                      "mov %rax, %cr4"
                      ::: "rax", "memory");
}

/* Flush range */
static inline void flush_tlb_range(void *mm, unsigned long start,
                                    unsigned long end)
{
    /* Simplified: flush all */
    __flush_tlb();
}

static inline void flush_tlb_page(void *mm, unsigned long addr)
{
    __flush_tlb_one(addr);
}

static inline void flush_tlb_all(void)
{
    __flush_tlb_all();
}

/* PCID support */
#define PCID_KERNEL     0x000
#define PCID_USER       0x001
#define PCID_NOFLUSH    0x800

static inline void __flush_tlb_one_pcid(unsigned long addr, uint16_t pcid)
{
    uint64_t val = (addr & PAGE_MASK) | pcid;
    __asm__ volatile ("invpcid %0, %1" :: "m" (val), "r" ((uint64_t)0) : "memory");
}

static inline void invpcid_flush_all(uint16_t pcid)
{
    uint64_t desc[2] = { pcid, 0 };
    __asm__ volatile ("invpcid %0, %1" :: "m" (desc[0]), "r" ((uint64_t)2) : "memory");
}

#endif /* _LINUXAB_X86_TLBFLUSH_H */
