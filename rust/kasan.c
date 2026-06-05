
// SPDX-License-Identifier: GPL-2.0
/*
 * linuxab KASAN implementation
 */

#include "kasan.h"
#include "printk.h"

uint8_t *kasan_shadow_base = NULL;
bool kasan_enabled = false;

static inline uint8_t *kasan_mem_to_shadow(const void *addr)
{
    return kasan_shadow_base + (((uintptr_t)addr) >> KASAN_SHADOW_SCALE_SHIFT);
}

static inline bool kasan_addr_valid(const void *addr)
{
    /* TODO: Check against actual memory map */
    return (addr != NULL);
}

void kasan_init(void *shadow_base, size_t shadow_size)
{
    kasan_shadow_base = (uint8_t *)shadow_base;
    /* Initialize all shadow memory to 0 (accessible) */
    for (size_t i = 0; i < shadow_size; i++) {
        kasan_shadow_base[i] = 0;
    }
    kasan_enabled = true;
    printk(KERN_INFO "KASAN initialized, shadow: %p, size: %zu\\n", shadow_base, shadow_size);
}

void kasan_poison_shadow(const void *addr, size_t size, uint8_t value)
{
    if (!kasan_enabled || !addr) return;
    
    uint8_t *shadow = kasan_mem_to_shadow(addr);
    size_t shadow_size = size >> KASAN_SHADOW_SCALE_SHIFT;
    
    /* Handle partial first byte */
    uintptr_t aligned = ((uintptr_t)addr) & KASAN_SHADOW_MASK;
    if (aligned) {
        *shadow = value;
        shadow++;
        if (size < (KASAN_SHADOW_SCALE_SIZE - aligned))
            return;
        size -= (KASAN_SHADOW_SCALE_SIZE - aligned);
    }
    
    for (size_t i = 0; i < (size >> KASAN_SHADOW_SCALE_SHIFT); i++) {
        shadow[i] = value;
    }
    
    /* Handle partial last byte */
    if (size & KASAN_SHADOW_MASK) {
        shadow[size >> KASAN_SHADOW_SCALE_SHIFT] = value;
    }
}

void kasan_unpoison_shadow(const void *addr, size_t size)
{
    if (!kasan_enabled || !addr) return;
    
    uint8_t *shadow = kasan_mem_to_shadow(addr);
    size_t shadow_size = size >> KASAN_SHADOW_SCALE_SHIFT;
    
    uintptr_t aligned = ((uintptr_t)addr) & KASAN_SHADOW_MASK;
    if (aligned) {
        *shadow = 0;
        shadow++;
        if (size < (KASAN_SHADOW_SCALE_SIZE - aligned))
            return;
        size -= (KASAN_SHADOW_SCALE_SIZE - aligned);
    }
    
    for (size_t i = 0; i < (size >> KASAN_SHADOW_SCALE_SHIFT); i++) {
        shadow[i] = 0;
    }
    
    if (size & KASAN_SHADOW_MASK) {
        shadow[size >> KASAN_SHADOW_SCALE_SHIFT] = 0;
    }
}

void kasan_kmalloc(void *addr, size_t size, size_t alloc_size)
{
    if (!kasan_enabled) return;
    
    /* Poison redzone after allocated object */
    kasan_unpoison_shadow(addr, size);
    kasan_poison_shadow((char *)addr + size, alloc_size - size, KASAN_KMALLOC_REDZONE);
}

void kasan_kfree(void *addr, size_t size)
{
    if (!kasan_enabled) return;
    kasan_poison_shadow(addr, size, KASAN_SLAB_FREE);
}

void kasan_alloca(void *addr, size_t size)
{
    if (!kasan_enabled) return;
    kasan_unpoison_shadow(addr, size);
    /* Poison redzones around alloca */
    kasan_poison_shadow((char *)addr - KASAN_SHADOW_SCALE_SIZE, KASAN_SHADOW_SCALE_SIZE, KASAN_STACK_LEFT);
    kasan_poison_shadow((char *)addr + size, KASAN_SHADOW_SCALE_SIZE, KASAN_STACK_RIGHT);
}

void kasan_report(unsigned long addr, size_t size, bool write, unsigned long ip)
{
    printk(KERN_EMERG "=================================================================\\n");
    printk(KERN_EMERG "BUG: KASAN: %s in %p\\n", write ? "write-access" : "read-access", (void *)ip);
    printk(KERN_EMERG "Address: %p, Size: %zu\\n", (void *)addr, size);
    printk(KERN_EMERG "=================================================================\\n");
    
    /* TODO: Print stack trace, halt or panic */
    while (1) __asm__ volatile ("cli; hlt");
}

static inline void kasan_check(const void *addr, size_t size, bool write)
{
    if (!kasan_enabled) return;
    if (!kasan_addr_valid(addr)) return;
    
    uint8_t *shadow = kasan_mem_to_shadow(addr);
    uint8_t shadow_val = *shadow;
    
    if (shadow_val == 0) return;  /* Fully accessible */
    
    /* Check partial access */
    uintptr_t last_byte = ((uintptr_t)addr + size - 1) & KASAN_SHADOW_MASK;
    if (last_byte >= shadow_val) {
        kasan_report((unsigned long)addr, size, write, (unsigned long)__builtin_return_address(0));
    }
}

/* Compiler instrumentation entry points */
void __asan_load1(void *addr)  { kasan_check(addr, 1, false); }
void __asan_store1(void *addr) { kasan_check(addr, 1, true); }
void __asan_load2(void *addr)  { kasan_check(addr, 2, false); }
void __asan_store2(void *addr) { kasan_check(addr, 2, true); }
void __asan_load4(void *addr)  { kasan_check(addr, 4, false); }
void __asan_store4(void *addr) { kasan_check(addr, 4, true); }
void __asan_load8(void *addr)  { kasan_check(addr, 8, false); }
void __asan_store8(void *addr) { kasan_check(addr, 8, true); }
void __asan_load16(void *addr) { kasan_check(addr, 16, false); }
void __asan_store16(void *addr){ kasan_check(addr, 16, true); }

void __asan_loadN(void *addr, size_t size)  { kasan_check(addr, size, false); }
void __asan_storeN(void *addr, size_t size) { kasan_check(addr, size, true); }
void __asan_handle_no_return(void) { /* TODO: poison stack */ }