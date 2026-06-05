/* SPDX-License-Identifier: GPL-2.0 */
/*
 * linuxab/arch/x86/kernel/debug.S
 * Assembly debug helpers for x86_64
 */

#include <linuxab/linkage.h>
#include <linuxab/asm-offsets.h>

    .text
    .code64

/*
 * dump_regs - Dump all GPRs to serial/VGA
 * void dump_regs(struct pt_regs *regs)
 */
ENTRY(dump_regs)
    pushq   %rbp
    movq    %rsp, %rbp
    
    /* Save callee-saved regs */
    pushq   %rbx
    pushq   %r12
    pushq   %r13
    pushq   %r14
    pushq   %r15
    
    movq    %rdi, %rbx          /* regs pointer */
    
    /* Print RAX */
    movq    PT_RAX(%rbx), %rsi
    movq    $fmt_rax, %rdi
    call    printk
    
    /* Print RBX */
    movq    PT_RBX(%rbx), %rsi
    movq    $fmt_rbx, %rdi
    call    printk
    
    /* Print RCX */
    movq    PT_RCX(%rbx), %rsi
    movq    $fmt_rcx, %rdi
    call    printk
    
    /* Print RDX */
    movq    PT_RDX(%rbx), %rsi
    movq    $fmt_rdx, %rdi
    call    printk
    
    /* Print RSI */
    movq    PT_RSI(%rbx), %rsi
    movq    $fmt_rsi, %rdi
    call    printk
    
    /* Print RDI */
    movq    PT_RDI(%rbx), %rsi
    movq    $fmt_rdi, %rdi
    call    printk
    
    /* Print RBP */
    movq    PT_RBP(%rbx), %rsi
    movq    $fmt_rbp, %rdi
    call    printk
    
    /* Print RSP */
    movq    PT_RSP(%rbx), %rsi
    movq    $fmt_rsp, %rdi
    call    printk
    
    /* Print R8-R15 */
    movq    PT_R8(%rbx), %rsi
    movq    $fmt_r8, %rdi
    call    printk
    
    movq    PT_R9(%rbx), %rsi
    movq    $fmt_r9, %rdi
    call    printk
    
    movq    PT_R10(%rbx), %rsi
    movq    $fmt_r10, %rdi
    call    printk
    
    movq    PT_R11(%rbx), %rsi
    movq    $fmt_r11, %rdi
    call    printk
    
    movq    PT_R12(%rbx), %rsi
    movq    $fmt_r12, %rdi
    call    printk
    
    movq    PT_R13(%rbx), %rsi
    movq    $fmt_r13, %rdi
    call    printk
    
    movq    PT_R14(%rbx), %rsi
    movq    $fmt_r14, %rdi
    call    printk
    
    movq    PT_R15(%rbx), %rsi
    movq    $fmt_r15, %rdi
    call    printk
    
    /* Print RIP */
    movq    PT_RIP(%rbx), %rsi
    movq    $fmt_rip, %rdi
    call    printk
    
    /* Print RFLAGS */
    movq    PT_RFLAGS(%rbx), %rsi
    movq    $fmt_rflags, %rdi
    call    printk
    
    /* Print CS/SS */
    movq    PT_CS(%rbx), %rsi
    movq    $fmt_cs, %rdi
    call    printk
    
    movq    PT_SS(%rbx), %rsi
    movq    $fmt_ss, %rdi
    call    printk
    
    /* Restore */
    popq    %r15
    popq    %r14
    popq    %r13
    popq    %r12
    popq    %rbx
    popq    %rbp
    ret

    .section .rodata
fmt_rax:    .asciz "RAX: %016llx\n"
fmt_rbx:    .asciz "RBX: %016llx\n"
fmt_rcx:    .asciz "RCX: %016llx\n"
fmt_rdx:    .asciz "RDX: %016llx\n"
fmt_rsi:    .asciz "RSI: %016llx\n"
fmt_rdi:    .asciz "RDI: %016llx\n"
fmt_rbp:    .asciz "RBP: %016llx\n"
fmt_rsp:    .asciz "RSP: %016llx\n"
fmt_r8:     .asciz "R8:  %016llx\n"
fmt_r9:     .asciz "R9:  %016llx\n"
fmt_r10:    .asciz "R10: %016llx\n"
fmt_r11:    .asciz "R11: %016llx\n"
fmt_r12:    .asciz "R12: %016llx\n"
fmt_r13:    .asciz "R13: %016llx\n"
fmt_r14:    .asciz "R14: %016llx\n"
fmt_r15:    .asciz "R15: %016llx\n"
fmt_rip:    .asciz "RIP: %016llx\n"
fmt_rflags: .asciz "RFLAGS: %016llx\n"
fmt_cs:     .asciz "CS:  %04llx\n"
fmt_ss:     .asciz "SS:  %04llx\n"

    .text

/*
 * dump_stack - Simple stack trace using RBP chain
 * void dump_stack(void)
 */
ENTRY(dump_stack)
    pushq   %rbp
    movq    %rsp, %rbp
    
    movq    $fmt_stack_hdr, %rdi
    call    printk
    
    movq    %rbp, %rbx          /* Walk RBP chain */
    movq    $16, %r12           /* Max depth */
    
1:
    testq   %rbx, %rbx
    jz      2f
    
    movq    8(%rbx), %rsi       /* Return address */
    movq    %rbx, %rdx          /* Frame pointer */
    movq    $fmt_stack_entry, %rdi
    call    printk
    
    movq    (%rbx), %rbx        /* Next RBP */
    decq    %r12
    jnz     1b

2:
    popq    %rbp
    ret

    .section .rodata
fmt_stack_hdr:    .asciz "Call Trace:\n"
fmt_stack_entry:  .asciz " [<%016llx>] frame=%p\n"

    .text

/*
 * show_code - Show code around RIP
 * void show_code(unsigned long *rip)
 */
ENTRY(show_code)
    pushq   %rbp
    movq    %rsp, %rbp
    
    /* Print 16 bytes before and 16 bytes after RIP */
    subq    $16, %rdi
    movq    $fmt_code, %rsi
    /* TODO: Print hex bytes */
    
    popq    %rbp
    ret

    .section .rodata
fmt_code: .asciz "Code: "

    .text

/*
 * int3_handler - Breakpoint handler
 */
ENTRY(int3_handler)
    /* Build pt_regs on stack */
    pushq   %rax
    pushq   %rcx
    pushq   %rdx
    pushq   %rsi
    pushq   %rdi
    pushq   %r8
    pushq   %r9
    pushq   %r10
    pushq   %r11
    pushq   %rbx
    pushq   %rbp
    pushq   %r12
    pushq   %r13
    pushq   %r14
    pushq   %r15
    
    movq    %rsp, %rdi
    call    dump_regs
    
    popq    %r15
    popq    %r14
    popq    %r13
    popq    %r12
    popq    %rbp
    popq    %rbx
    popq    %r11
    popq    %r10
    popq    %r9
    popq    %r8
    popq    %rdi
    popq    %rsi
    popq    %rdx
    popq    %rcx
    popq    %rax
    
    iretq

ENDPROC(dump_regs)
ENDPROC(dump_stack)
ENDPROC(show_code)
ENDPROC(int3_handler)
