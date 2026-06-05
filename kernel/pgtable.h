/* SPDX-License-Identifier: GPL-2.0 */
/*
 * linuxab/arch/x86/include/asm/pgtable.h
 * x86_64 page table manipulation
 */

#ifndef _LINUXAB_X86_PGTABLE_H
#define _LINUXAB_X86_PGTABLE_H

#include "page.h"

#define KERNEL_PGD_BOUNDARY     PTRS_PER_PGD
#define KERNEL_PGD_PTRS         (PTRS_PER_PGD)

#define pgd_none(pgd)           (!pgd_val(pgd))
#define pgd_present(pgd)        (pgd_val(pgd) & _PAGE_PRESENT)
#define pgd_bad(pgd)            ((pgd_val(pgd) & ~PHYSICAL_MASK) != 0)

#define pud_none(pud)           (!pud_val(pud))
#define pud_present(pud)        (pud_val(pud) & _PAGE_PRESENT)
#define pud_bad(pud)            ((pud_val(pud) & ~PHYSICAL_MASK) != 0)

#define pmd_none(pmd)           (!pmd_val(pmd))
#define pmd_present(pmd)        (pmd_val(pmd) & _PAGE_PRESENT)
#define pmd_bad(pmd)            ((pmd_val(pmd) & ~PHYSICAL_MASK) != 0)
#define pmd_large(pmd)          (pmd_val(pmd) & _PAGE_PSE)

#define pte_none(pte)           (!pte_val(pte))
#define pte_present(pte)        (pte_val(pte) & _PAGE_PRESENT)
#define pte_write(pte)          (pte_val(pte) & _PAGE_RW)
#define pte_dirty(pte)          (pte_val(pte) & _PAGE_DIRTY)
#define pte_young(pte)          (pte_val(pte) & _PAGE_ACCESSED)
#define pte_special(pte)        (pte_val(pte) & _PAGE_SPECIAL)
#define pte_exec(pte)           (!(pte_val(pte) & _PAGE_NX))

#define pte_pfn(pte)            ((pte_val(pte) & PHYSICAL_MASK) >> PAGE_SHIFT)
#define pfn_pte(pfn, prot)      __pte(((pfn) << PAGE_SHIFT) | pgprot_val(prot))
#define mk_pte(page, prot)      pfn_pte(virt_to_pfn(page), prot)

typedef uint64_t pgprot_t;

#define pgprot_val(x)           ((uint64_t)(x))
#define __pgprot(x)             ((pgprot_t)(x))

#define PAGE_KERNEL_PGPROT      __pgprot(_PAGE_PRESENT | _PAGE_RW | _PAGE_GLOBAL | _PAGE_ACCESSED | _PAGE_DIRTY)
#define PAGE_KERNEL_RO_PGPROT   __pgprot(_PAGE_PRESENT | _PAGE_GLOBAL | _PAGE_ACCESSED)
#define PAGE_KERNEL_EXEC_PGPROT __pgprot(_PAGE_PRESENT | _PAGE_RW | _PAGE_GLOBAL | _PAGE_ACCESSED | _PAGE_DIRTY)

/* Set page table entry */
static inline void native_set_pte(pte_t *ptep, pte_t pte)
{
    *ptep = pte;
}

static inline void native_set_pmd(pmd_t *pmdp, pmd_t pmd)
{
    *pmdp = pmd;
}

static inline void native_set_pud(pud_t *pudp, pud_t pud)
{
    *pudp = pud;
}

static inline void native_set_pgd(pgd_t *pgdp, pgd_t pgd)
{
    *pgdp = pgd;
}

/* Get page table entry */
static inline pte_t native_ptep_get(pte_t *ptep)
{
    return *ptep;
}

/* Page table entry modification */
static inline pte_t pte_mkwrite(pte_t pte)
{
    return __pte(pte_val(pte) | _PAGE_RW);
}

static inline pte_t pte_wrprotect(pte_t pte)
{
    return __pte(pte_val(pte) & ~_PAGE_RW);
}

static inline pte_t pte_mkdirty(pte_t pte)
{
    return __pte(pte_val(pte) | _PAGE_DIRTY);
}

static inline pte_t pte_mkclean(pte_t pte)
{
    return __pte(pte_val(pte) & ~_PAGE_DIRTY);
}

static inline pte_t pte_mkyoung(pte_t pte)
{
    return __pte(pte_val(pte) | _PAGE_ACCESSED);
}

static inline pte_t pte_mkold(pte_t pte)
{
    return __pte(pte_val(pte) & ~_PAGE_ACCESSED);
}

static inline pte_t pte_mkexec(pte_t pte)
{
    return __pte(pte_val(pte) & ~_PAGE_NX);
}

static inline pte_t pte_mknexec(pte_t pte)
{
    return __pte(pte_val(pte) | _PAGE_NX);
}

/* Global page table operations */
#define set_pte(ptep, pte)          native_set_pte(ptep, pte)
#define set_pmd(pmdp, pmd)          native_set_pmd(pmdp, pmd)
#define set_pud(pudp, pud)          native_set_pud(pudp, pud)
#define set_pgd(pgdp, pgd)          native_set_pgd(pgdp, pgd)

#define pte_page(pte)               pfn_to_virt(pte_pfn(pte))

#endif /* _LINUXAB_X86_PGTABLE_H */
