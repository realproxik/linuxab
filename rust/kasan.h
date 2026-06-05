// SPDX-License-Identifier: GPL-2.0
/*
 * linuxab KASAN (Kernel Address Sanitizer)
 * Shadow memory based detection for out-of-bounds and use-after-free
 */

#ifndef _LINUXAB_KASAN_H
#define _LINUXAB_KASAN_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define KASAN_SHADOW_SCALE_SHIFT 3
#define KASAN_SHADOW_SCALE_SIZE  (1ULL << KASAN_SHADOW_SCALE_SHIFT)
#define KASAN_SHADOW_MASK        (KASAN_SHADOW_SCALE_SIZE - 1)

/* Shadow values */
#define KASAN_FREE_PAGE          0xFF  /* freed page */
#define KASAN_PAGE_REDZONE       0xFE  /* page redzone */
#define KASAN_SLAB_REDZONE       0xFD  /* slab redzone */
#define KASAN_SLAB_FREE          0xFC  /* freed slab object */
#define KASAN_GLOBAL_REDZONE     0xFB  /* global redzone */
#define KASAN_STACK_LEFT         0xF1  /* stack left redzone */
#define KASAN_STACK_MID          0xF2  /* stack mid redzone */
#define KASAN_STACK_RIGHT        0xF3  /* stack right redzone */
#define KASAN_STACK_FRAME        0xF4  /* stack after scope */
#define KASAN_STACK_KMALLOC      0xF5  /* kmalloc redzone */
#define KASAN_KMALLOC_REDZONE    0xF6  /* kmalloc redzone */

/* Shadow memory base - adjust to your memory map */
extern uint8_t *kasan_shadow_base;
extern bool kasan_enabled;

void kasan_init(void *shadow_base, size_t shadow_size);
void kasan_poison_shadow(const void *addr, size_t size, uint8_t value);
void kasan_unpoison_shadow(const void *addr, size_t size);
void kasan_alloc_pages(void *addr, size_t pages);
void kasan_free_pages(void *addr, size_t pages);

/* Hooks for allocator */
void kasan_kmalloc(void *addr, size_t size, size_t alloc_size);
void kasan_kfree(void *addr, size_t size);
void kasan_alloca(void *addr, size_t size);

/* Report */
void kasan_report(unsigned long addr, size_t size, bool write, unsigned long ip);

/* Builtins called by compiler instrumentation */
void __asan_loadN(void *addr, size_t size);
void __asan_storeN(void *addr, size_t size);
void __asan_load1(void *addr);  void __asan_store1(void *addr);
void __asan_load2(void *addr);  void __asan_store2(void *addr);
void __asan_load4(void *addr);  void __asan_store4(void *addr);
void __asan_load8(void *addr);  void __asan_store8(void *addr);
void __asan_load16(void *addr); void __asan_store16(void *addr);
void __asan_handle_no_return(void);

#endif /* _LINUXAB_KASAN_H */
