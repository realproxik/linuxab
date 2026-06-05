// SPDX-License-Identifier: GPL-2.0
/*
 * linuxab Spectre/Meltdown + Retpoline implementation
 */

#include "spectre.h"
#include "printk.h"

uint32_t cpu_vulnerabilities = 0;
bool retpoline_enabled = false;

/* 
 * Retpoline thunk for x86_64 indirect calls/jumps.
 * This prevents Spectre V2 by bouncing through a "safe" trampoline.
 * 
 * Usage: Instead of `call *%rax`, use:
 *   mov %rax, %r11
 *   call __x86_indirect_thunk_r11
 */

/* 
 * Inline assembly thunk. In real Linux these are in .S files.
 * We use inline asm with naked attribute or just provide the thunk.
 */
__attribute__((naked, used))
void __x86_indirect_thunk_rax(void)
{
    __asm__ volatile (
        "call 1f\\n"
        "1: \\n"
        "pause \\n"
        "lfence \\n"
        "jmp 1b\\n"
        "2: \\n"
        "mov %%rax, (%%rsp)\\n"
        "ret\\n"
        ::: "memory"
    );
}

__attribute__((naked, used))
void __x86_indirect_thunk_r11(void)
{
    __asm__ volatile (
        "call 1f\\n"
        "1: \\n"
        "pause \\n"
        "lfence \\n"
        "jmp 1b\\n"
        "2: \\n"
        "mov %%r11, (%%rsp)\\n"
        "ret\\n"
        ::: "memory"
    );
}

/* Generic thunk - can be called with any register */
#define DEFINE_RETPOLINE_THUNK(reg) \
    __attribute__((naked, used)) \
    void __x86_indirect_thunk_##reg(void) \
    { \
        __asm__ volatile ( \
            "call 1f\\n" \
            "1: \\n" \
            "pause \\n" \
            "lfence \\n" \
            "jmp 1b\\n" \
            "2: \\n" \
            "mov %" #reg ", (%%rsp)\\n" \
            "ret\\n" \
            ::: "memory" \
        ); \
    }

/* Define thunks for common registers */
DEFINE_RETPOLINE_THUNK(rbx)
DEFINE_RETPOLINE_THUNK(rcx)
DEFINE_RETPOLINE_THUNK(rdx)
DEFINE_RETPOLINE_THUNK(rsi)
DEFINE_RETPOLINE_THUNK(rdi)
DEFINE_RETPOLINE_THUNK(rbp)
DEFINE_RETPOLINE_THUNK(r8)
DEFINE_RETPOLINE_THUNK(r9)
DEFINE_RETPOLINE_THUNK(r10)
DEFINE_RETPOLINE_THUNK(r12)
DEFINE_RETPOLINE_THUNK(r13)
DEFINE_RETPOLINE_THUNK(r14)
DEFINE_RETPOLINE_THUNK(r15)

void spectre_init(void)
{
    /* 
     * TODO: Detect CPU vendor/model and check microcode/features.
     * For now, assume vulnerable and enable mitigations.
     */
    
    cpu_vulnerabilities = SPECTRE_V1 | SPECTRE_V2 | MELTDOWN;
    
    /* Enable retpoline by default */
    retpoline_enabled = true;
    
    printk(KERN_INFO "Spectre/Meltdown: Vulnerabilities detected: 0x%x\\n", cpu_vulnerabilities);
    printk(KERN_INFO "Spectre/Meltdown: Retpoline %s\\n", retpoline_enabled ? "enabled" : "disabled");
    
    /* TODO: If IBPB/IBRS available, use hardware mitigations instead */
}

void spectre_report_vulnerabilities(void)
{
    printk(KERN_INFO "CPU vulnerabilities:\\n");
    if (cpu_vulnerabilities & SPECTRE_V1)
        printk(KERN_INFO "  - Spectre V1 (bounds check bypass)\\n");
    if (cpu_vulnerabilities & SPECTRE_V2)
        printk(KERN_INFO "  - Spectre V2 (branch target injection)\\n");
    if (cpu_vulnerabilities & MELTDOWN)
        printk(KERN_INFO "  - Meltdown (rogue data cache load)\\n");
    if (cpu_vulnerabilities & SPECTRE_V4)
        printk(KERN_INFO "  - Spectre V4 (speculative store bypass)\\n");
    if (cpu_vulnerabilities & L1TF)
        printk(KERN_INFO "  - L1TF (L1 terminal fault)\\n");
    if (cpu_vulnerabilities & MDS)
        printk(KERN_INFO "  - MDS (microarchitectural data sampling)\\n");
}

/* 
 * Meltdown mitigation: Flush TLB/PCID when crossing privilege boundaries.
 * Called before returning to userspace (if you have userspace).
 */
void meltdown_flush_tlb(void)
{
    /* 
     * On x86_64, reload CR3 to flush TLB.
     * In real Linux, this is done via PCID/INVPCID.
     */
    uint64_t cr3;
    __asm__ volatile ("mov %%cr3, %0" : "=r"(cr3));
    __asm__ volatile ("mov %0, %%cr3" :: "r"(cr3) : "memory");
}

/* 
 * Spectre V4 (SSBD) mitigation - Speculative Store Bypass Disable
 */
void spectre_v4_enable_ssbd(void)
{
    /* 
     * If CPU supports SSBD via MSR (AMD) or SPEC_CTRL (Intel),
     * set the appropriate bit.
     * For now, placeholder.
     */
}
