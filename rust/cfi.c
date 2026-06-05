// SPDX-License-Identifier: GPL-2.0
/*
 * linuxab CFI implementation
 */

#include "cfi.h"
#include "printk.h"

uint8_t *cfi_bitmap = NULL;
bool cfi_enabled = false;

static uintptr_t cfi_code_base = 0;

void cfi_init(void *code_start, size_t code_size)
{
    cfi_code_base = (uintptr_t)code_start;
    
    /* Allocate bitmap - in real kernel use memblock or bootmem */
    /* For linuxab, assume cfi_bitmap is pre-allocated by linker/boot */
    
    if (!cfi_bitmap) {
        printk(KERN_ERR "CFI: No bitmap allocated!\\n");
        return;
    }
    
    /* Clear bitmap */
    for (size_t i = 0; i < CFI_BITMAP_SIZE; i++) {
        cfi_bitmap[i] = 0;
    }
    
    cfi_enabled = true;
    printk(KERN_INFO "CFI initialized for code region %p - %p\\n",
           code_start, (char *)code_start + code_size);
}

void cfi_register_function(void *fn)
{
    if (!cfi_enabled || !fn) return;
    
    uintptr_t addr = (uintptr_t)fn;
    if (addr < cfi_code_base) return;
    
    uintptr_t offset = addr - cfi_code_base;
    if (offset >= CFI_BITMAP_BITS) return;
    
    size_t byte_idx = offset / 8;
    size_t bit_idx = offset % 8;
    
    cfi_bitmap[byte_idx] |= (1 << bit_idx);
}

void cfi_unregister_function(void *fn)
{
    if (!cfi_enabled || !fn) return;
    
    uintptr_t addr = (uintptr_t)fn;
    if (addr < cfi_code_base) return;
    
    uintptr_t offset = addr - cfi_code_base;
    if (offset >= CFI_BITMAP_BITS) return;
    
    size_t byte_idx = offset / 8;
    size_t bit_idx = offset % 8;
    
    cfi_bitmap[byte_idx] &= ~(1 << bit_idx);
}

bool cfi_validate_call(void *target)
{
    if (!cfi_enabled) return true;
    if (!target) return false;
    
    uintptr_t addr = (uintptr_t)target;
    if (addr < cfi_code_base) return false;
    
    uintptr_t offset = addr - cfi_code_base;
    if (offset >= CFI_BITMAP_BITS) return false;
    
    size_t byte_idx = offset / 8;
    size_t bit_idx = offset % 8;
    
    return (cfi_bitmap[byte_idx] & (1 << bit_idx)) != 0;
}

/* 
 * __cfi_check is called by compiler when using -fsanitize=cfi
 * For GCC, we might need custom instrumentation or use LLVM.
 * This is a simplified version.
 */
void __cfi_check(uint64_t id, void *target, void *site)
{
    (void)id;
    (void)site;
    
    if (!cfi_validate_call(target)) {
        printk(KERN_EMERG "CFI violation: invalid indirect call target %p from %p (type %llx)\\n",
               target, site, (unsigned long long)id);
        /* TODO: Print stack trace, panic */
        while (1) __asm__ volatile ("cli; hlt");
    }
}