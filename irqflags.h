/* SPDX-License-Identifier: GPL-2.0 */
/*
 * linuxab/arch/x86/include/asm/irqflags.h
 * Local IRQ save/restore
 */

#ifndef _LINUXAB_X86_IRQFLAGS_H
#define _LINUXAB_X86_IRQFLAGS_H

#include <stdint.h>

typedef uint64_t irq_flags_t;

/* Read RFLAGS */
static inline irq_flags_t arch_local_save_flags(void)
{
    irq_flags_t flags;
    __asm__ volatile ("pushfq\n\t"
                      "popq %0"
                      : "=rm" (flags)
                      :: "memory");
    return flags;
}

/* Read RFLAGS and disable interrupts */
static inline irq_flags_t arch_local_irq_save(void)
{
    irq_flags_t flags;
    __asm__ volatile ("pushfq\n\t"
                      "cli\n\t"
                      "popq %0"
                      : "=rm" (flags)
                      :: "memory");
    return flags;
}

/* Restore RFLAGS */
static inline void arch_local_irq_restore(irq_flags_t flags)
{
    __asm__ volatile ("pushq %0\n\t"
                      "popfq"
                      :: "g" (flags)
                      : "memory", "cc");
}

/* Enable interrupts */
static inline void arch_local_irq_enable(void)
{
    __asm__ volatile ("sti" ::: "memory");
}

/* Disable interrupts */
static inline void arch_local_irq_disable(void)
{
    __asm__ volatile ("cli" ::: "memory");
}

/* Check if interrupts are enabled */
static inline bool arch_irqs_disabled_flags(irq_flags_t flags)
{
    return !(flags & (1 << 9)); /* IF bit */
}

static inline bool arch_irqs_disabled(void)
{
    return arch_irqs_disabled_flags(arch_local_save_flags());
}

/* Macros used by generic code */
#define raw_local_irq_save(flags)       \
    do { (flags) = arch_local_irq_save(); } while (0)

#define raw_local_irq_restore(flags)    \
    do { arch_local_irq_restore(flags); } while (0)

#define raw_local_irq_disable()         arch_local_irq_disable()
#define raw_local_irq_enable()          arch_local_irq_enable()

#define raw_local_save_flags(flags)     \
    do { (flags) = arch_local_save_flags(); } while (0)

#define raw_irqs_disabled_flags(flags)  arch_irqs_disabled_flags(flags)
#define raw_irqs_disabled()             arch_irqs_disabled()

#endif /* _LINUXAB_X86_IRQFLAGS_H */
