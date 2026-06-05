// SPDX-License-Identifier: GPL-2.0
/*
 * linuxab CFI (Control Flow Integrity)
 * Forward-edge protection for indirect calls
 */

#ifndef _LINUXAB_CFI_H
#define _LINUXAB_CFI_H

#include <stdint.h>
#include <stdbool.h>

/* CFI type IDs - each function signature gets a hash/id */
#define CFI_TYPE_ID(type)  _cfi_type_id_##type

/* 
 * For simplicity in linuxab, we use a shadow table approach.
 * Each valid function start address is registered in a bitmap/table.
 */

#define CFI_BITMAP_BITS  (256 * 1024 * 1024ULL)  /* Cover 256MB code region */
#define CFI_BITMAP_SIZE  (CFI_BITMAP_BITS / 8)

extern uint8_t *cfi_bitmap;
extern bool cfi_enabled;

void cfi_init(void *code_start, size_t code_size);
void cfi_register_function(void *fn);
void cfi_unregister_function(void *fn);
bool cfi_validate_call(void *target);

/* 
 * In production Linux, CFI uses -fsanitize=cfi with LLVM.
 * For GCC/GNU toolchain, we can use a simpler shadow bitmap.
 */

/* Called from compiler-generated checks */
void __cfi_check(uint64_t id, void *target, void *site);

#endif /* _LINUXAB_CFI_H */