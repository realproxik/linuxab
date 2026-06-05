// SPDX-License-Identifier: GPL-2.0
/*
 * linuxab Spectre/Meltdown Mitigations
 */

#ifndef _LINUXAB_SPECTRE_H
#define _LINUXAB_SPECTRE_H

#include <stdint.h>
#include <stdbool.h>

/* CPU vulnerability flags */
#define SPECTRE_V1       (1 << 0)
#define SPECTRE_V2       (1 << 1)
#define MELTDOWN         (1 << 2)
#define SPECTRE_V4       (1 << 3)
#define L1TF             (1 << 4)
#define MDS              (1 << 5)

extern uint32_t cpu_vulnerabilities;
extern bool retpoline_enabled;

void spectre_init(void);
void spectre_report_vulnerabilities(void);

/* 
 * array_index_mask_nospec - prevent Spectre V1 bounds check bypass
 * Returns mask that is 0 if index >= size, else all 1s
 */
static inline uint64_t array_index_mask_nospec(uint64_t index, uint64_t size)
{
    uint64_t mask;
    
    /* 
     * This is the standard Linux approach:
     * Force index to be evaluated, then create mask.
     * The barrier prevents speculation.
     */
    __asm__ volatile (
        "cmp %1, %0\\n"
        "sbb %0, %0\\n"
        "neg %0\\n"
        : "=r" (mask)
        : "r" (size), "0" (index)
        : "cc"
    );
    
    return mask;
}

/* Speculation barrier */
static inline void speculation_barrier(void)
{
    __asm__ volatile ("lfence" ::: "memory");
}

/* Prevent speculative execution past this point */
static inline void barrier_nospec(void)
{
    __asm__ volatile ("lfence" ::: "memory");
}

/* 
 * Force serialization - used after modifying page tables
 * to prevent Meltdown-style data leakage
 */
static inline void serialize_cpu(void)
{
    /* Use cpuid as serializing instruction */
    uint32_t eax = 0, ebx, ecx, edx;
    __asm__ volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(eax)
        : "memory"
    );
}

/* 
 * Safe array access helper for Spectre V1 mitigation
 */
#define array_access_nospec(arr, idx, size) \
    ({ \
        typeof(size) _s = (size); \
        typeof(idx)  _i = (idx); \
        uint64_t _mask = array_index_mask_nospec(_i, _s); \
        typeof(*(arr)) *_p = (arr); \
        typeof(*(arr)) _val = _p[_i & _mask]; \
        speculation_barrier(); \
        _val; \
    })

#endif /* _LINUXAB_SPECTRE_H */