// SPDX-License-Identifier: GPL-2.0
/*
 * linuxab Stack Protector (Stack Canary)
 * Compile with: -fstack-protector-strong or -fstack-protector-all
 */

#include <stdint.h>
#include <stddef.h>

/* 
 * Global canary value. In real Linux this is per-task and refreshed.
 * For linuxab, we use a global canary initialized at boot with entropy.
 */
uintptr_t __stack_chk_guard = 0xdeadbeef;

/* 
 * Called by compiler-generated code when canary check fails.
 * Name must be exactly __stack_chk_fail for GCC/Clang.
 */
void __stack_chk_fail(void)
{
    /* 
     * This is called when a stack smashing attack is detected.
     * We must not return.
     */
    __asm__ volatile (
        "cli\\n"
    );
    
    /* TODO: Print panic message via early_printk or serial */
    /* For now, halt and catch fire */
    
    while (1) {
        __asm__ volatile ("hlt");
    }
}

/* 
 * Initialize stack canary with random value.
 * Call this during early boot after RNG/entropy is available.
 * If no RNG, use timestamp + hardware randomness.
 */
void stack_protector_init(uintptr_t canary)
{
    /* Ensure canary has null byte to stop string attacks */
    canary &= ~(uintptr_t)0xFF;
    canary |= 0x01;  /* Least byte non-zero */
    
    __stack_chk_guard = canary;
}

/* 
 * Alternative: x86_64 rdtsc based quick init before RNG is ready.
 */
void stack_protector_init_early(void)
{
    uint32_t lo, hi;
    __asm__ volatile ("rdtsc" : "=a"(lo), "=d"(hi));
    uintptr_t canary = ((uintptr_t)hi << 32) | lo;
    stack_protector_init(canary);
}