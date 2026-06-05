/* SPDX-License-Identifier: GPL-2.0 */
/*
 * linuxab/arch/x86/kernel/head_64.S
 * x86_64 kernel entry point
 * Sets up long mode, initial page tables, and jumps to C
 */

#include <linuxab/linkage.h>

/* Multiboot header */
#define MULTIBOOT_HEADER_MAGIC  0x1BADB002
#define MULTIBOOT_HEADER_FLAGS  0x00000003  /* PAGE_ALIGN + MEM_INFO */

    .section .mboot, "a"
    .align 4
mboot_header:
    .long MULTIBOOT_HEADER_MAGIC
    .long MULTIBOOT_HEADER_FLAGS
    .long -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)

    .text
    .code32

/*
 * linuxab_start - Entry point from bootloader (GRUB/multiboot)
 * EAX = MULTIBOOT_MAGIC (0x2BADB002)
 * EBX = Pointer to multiboot info structure
 */
ENTRY(linuxab_start)
    cli
    
    /* Save multiboot info */
    movl    %eax, %edi          /* Magic */
    movl    %ebx, %esi          /* Info pointer */
    
    /* Check multiboot magic */
    cmpl    $0x2BADB002, %eax
    je      1f
    
    /* Not multiboot - hang */
    movl    $0xB8000, %edx
    movw    $0x4F4E, (%edx)     /* 'N' red on white = No multiboot
    hlt
    jmp     .

1:
    /* Load GDT */
    lgdt    early_gdt_descr - __START_KERNEL_map
    
    /* Reload segments */
    ljmp    $0x08, $1f
1:
    movl    $0x10, %eax
    movw    %ax, %ds
    movw    %ax, %es
    movw    %ax, %fs
    movw    %ax, %gs
    movw    %ax, %ss
    
    /* Set up early stack */
    movl    $early_stack_top - __START_KERNEL_map, %esp
    
    /* Enable PAE */
    movl    %cr4, %eax
    orl     $0x20, %eax         /* CR4.PAE */
    movl    %eax, %cr4
    
    /* Load PML4 */
    movl    $early_pml4 - __START_KERNEL_map, %eax
    movl    %eax, %cr3
    
    /* Enable long mode (EFER.LME) */
    movl    $0xC0000080, %ecx   /* EFER MSR */
    rdmsr
    orl     $0x100, %eax        /* LME */
    wrmsr
    
    /* Enable paging + protected mode */
    movl    %cr0, %eax
    orl     $0x80000001, %eax   /* PG + PE */
    movl    %eax, %cr0
    
    /* Now in compatibility mode, ljmp to 64-bit code */
    ljmp    $0x08, $linuxab_startup_64 - __START_KERNEL_map

    .code64
ENTRY(linuxab_startup_64)
    /* We are now in long mode but still in low identity mapping */
    
    /* Set up proper 64-bit segments */
    movl    $0x10, %eax
    movw    %ax, %ds
    movw    %ax, %es
    movw    %ax, %fs
    movw    %ax, %gs
    movw    %ax, %ss
    
    /* Set up proper stack in higher half */
    movq    $early_stack_top, %rsp
    
    /* Save multiboot info in higher half registers */
    movl    %edi, %edi          /* Zero extend magic */
    movl    %esi, %esi          /* Zero extend info ptr */
    
    /* Jump to higher half mapped code */
    movabsq $linuxab_main, %rax
    jmp     *%rax

    .bss
    .align 4096
    .space  8192
early_stack_top:

    .data
    .align 4096

/* Early page tables - identity map first 1GB + higher half map */
early_pml4:
    .quad   early_pdpt_low - __START_KERNEL_map + 0x003   /* Present, RW */
    .fill   255, 8, 0
    .quad   early_pdpt_high - __START_KERNEL_map + 0x003  /* Higher half at 0xFFFF800000000000 */
    .fill   255, 8, 0

early_pdpt_low:
    .quad   early_pd_low - __START_KERNEL_map + 0x003
    .fill   511, 8, 0

early_pdpt_high:
    .quad   early_pd_high - __START_KERNEL_map + 0x003
    .fill   511, 8, 0

early_pd_low:
    /* 1GB huge page at 0x00000000 */
    .quad   0x0000000000000183  /* Present, RW, PS (1GB), Global */
    .fill   511, 8, 0

early_pd_high:
    /* 1GB huge page at 0xFFFFFFFF80000000 */
    .quad   0x0000000000000183  /* Same physical memory, higher half */
    .fill   511, 8, 0

/* Early GDT */
    .align 16
early_gdt:
    .quad   0x0000000000000000  /* NULL */
    .quad   0x00AF9A000000FFFF  /* 64-bit code */
    .quad   0x00CF92000000FFFF  /* 64/32-bit data */
    .quad   0x00AFFA000000FFFF  /* 32-bit code (compat) */
    .quad   0x00CFF2000000FFFF  /* 32-bit data */

early_gdt_descr:
    .word   early_gdt_descr - early_gdt - 1
    .quad   early_gdt - __START_KERNEL_map

    .set    __START_KERNEL_map, 0xFFFFFFFF80000000

END(linuxab_start)
END(linuxab_startup_64)
