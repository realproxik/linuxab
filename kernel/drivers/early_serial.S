* SPDX-License-Identifier: GPL-2.0 */
/*
 * linuxab/arch/x86/boot/early_serial.S
 * Early 8250 UART debug output for x86_64
 * Equivalent to ARM's 8250.S
 */

#include <linuxab/linkage.h>

/* Standard PC COM1 ports */
#define UART_BASE       0x3F8
#define UART_THR        (UART_BASE + 0x00)  /* Transmitter Holding */
#define UART_RBR        (UART_BASE + 0x00)  /* Receiver Buffer */
#define UART_IER        (UART_BASE + 0x01)  /* Interrupt Enable */
#define UART_IIR        (UART_BASE + 0x02)  /* Interrupt ID */
#define UART_FCR        (UART_BASE + 0x02)  /* FIFO Control */
#define UART_LCR        (UART_BASE + 0x03)  /* Line Control */
#define UART_MCR        (UART_BASE + 0x04)  /* Modem Control */
#define UART_LSR        (UART_BASE + 0x05)  /* Line Status */
#define UART_MSR        (UART_BASE + 0x06)  /* Modem Status */
#define UART_SCR        (UART_BASE + 0x07)  /* Scratch */

/* Line Status Register bits */
#define UART_LSR_DR     0x01    /* Data Ready */
#define UART_LSR_OE     0x02    /* Overrun Error */
#define UART_LSR_PE     0x04    /* Parity Error */
#define UART_LSR_FE     0x08    /* Framing Error */
#define UART_LSR_BI     0x10    /* Break Interrupt */
#define UART_LSR_THRE   0x20    /* Transmitter Holding Empty */
#define UART_LSR_TEMT   0x40    /* Transmitter Empty */
#define UART_LSR_ERR    0x80    /* Error */

/* Line Control Register */
#define UART_LCR_WLEN8  0x03    /* 8-bit */
#define UART_LCR_DLAB   0x80    /* Divisor Latch Access */

/* Modem Control Register */
#define UART_MCR_DTR    0x01
#define UART_MCR_RTS    0x02
#define UART_MCR_OUT1   0x04
#define UART_MCR_OUT2   0x08
#define UART_MCR_LOOP   0x10

/* Divisor for 115200 baud (assuming 1.8432MHz clock) */
#define UART_DIV_115200 1

    .text
    .code32

/*
 * early_serial_init - Initialize COM1 to 115200 8N1
 * Called in real mode or early 32-bit protected mode
 */
ENTRY(early_serial_init)
    pushl   %eax
    pushl   %edx

    /* Disable interrupts */
    movb    $0x00, %al
    movw    $UART_IER, %dx
    outb    %al, %dx

    /* Enable DLAB */
    movb    $(UART_LCR_DLAB | UART_LCR_WLEN8), %al
    movw    $UART_LCR, %dx
    outb    %al, %dx

    /* Set divisor to 1 (115200 baud) */
    movb    $UART_DIV_115200, %al
    movw    $UART_THR, %dx      /* DLL */
    outb    %al, %dx
    xorb    %al, %al
    incw    %dx                 /* DLM */
    outb    %al, %dx

    /* Clear DLAB, set 8N1 */
    movb    $UART_LCR_WLEN8, %al
    movw    $UART_LCR, %dx
    outb    %al, %dx

    /* Enable FIFO, clear them, 14-byte threshold */
    movb    $0xC7, %al
    movw    $UART_FCR, %dx
    outb    %al, %dx

    /* Set DTR, RTS, OUT2 */
    movb    $(UART_MCR_DTR | UART_MCR_RTS | UART_MCR_OUT2), %al
    movw    $UART_MCR, %dx
    outb    %al, %dx

    popl    %edx
    popl    %eax
    ret

/*
 * early_serial_putc - Output one character to COM1
 * %al = character to output
 * Clobbers: %dx
 */
ENTRY(early_serial_putc)
    pushl   %edx
    movw    $UART_LSR, %dx

1:  /* Wait for THRE */
    inb     %dx, %al
    testb   $UART_LSR_THRE, %al
    jz      1b

    /* Output character */
    movb    8(%esp), %al        /* Get char from stack (after push) */
    movw    $UART_THR, %dx
    outb    %al, %dx

    popl    %edx
    ret

/*
 * early_serial_puts - Output NUL-terminated string
 * %esi = string pointer (32-bit)
 */
ENTRY(early_serial_puts)
    pushl   %eax
    pushl   %edx
    pushl   %esi

1:
    lodsb
    testb   %al, %al
    jz      2f

    /* Wait for THRE */
    movw    $UART_LSR, %dx
3:
    inb     %dx, %al
    testb   $UART_LSR_THRE, %al
    jz      3b

    /* Send char */
    movb    (%esi-1), %al
    movw    $UART_THR, %dx
    outb    %al, %dx
    jmp     1b

2:
    popl    %esi
    popl    %edx
    popl    %eax
    ret

/*
 * early_serial_puthex - Output 32-bit hex value
 * %eax = value
 */
ENTRY(early_serial_puthex)
    pushl   %eax
    pushl   %ecx
    pushl    %edx
    movl    $8, %ecx

1:
    roll    $4, %eax
    movl    %eax, %edx
    andl    $0x0F, %edx
    cmpb    $10, %dl
    jb      2f
    addb    $('a' - 10), %dl
    jmp     3f
2:
    addb    $'0', %dl
3:
    pushl   %eax
    pushl   %edx
    call    early_serial_putc
    addl    $4, %esp
    popl    %eax
    loop    1b

    popl    %edx
    popl    %ecx
    popl    %eax
    ret

    .code64

/*
 * early_serial_putc64 - 64-bit mode version
 * %rdi = character
 */
ENTRY(early_serial_putc64)
    pushq   %rdx
    movw    $UART_LSR, %dx

1:
    inb     %dx, %al
    testb   $UART_LSR_THRE, %al
    jz      1b

    movb    %dil, %al
    movw    $UART_THR, %dx
    outb    %al, %dx

    popq    %rdx
    ret

/*
 * early_serial_puts64 - 64-bit mode string output
 * %rdi = string pointer
 */
ENTRY(early_serial_puts64)
    pushq   %rax
    pushq   %rdx
    pushq   %rsi
    movq    %rdi, %rsi

1:
    lodsb
    testb   %al, %al
    jz      2f

    movw    $UART_LSR, %dx
3:
    inb     %dx, %al
    testb   $UART_LSR_THRE, %al
    jz      3b

    movb    (%rsi-1), %al
    movw    $UART_THR, %dx
    outb    %al, %dx
    jmp     1b

2:
    popq    %rsi
    popq    %rdx
    popq    %rax
    ret

ENDPROC(early_serial_init)
ENDPROC(early_serial_putc)
ENDPROC(early_serial_puts)
ENDPROC(early_serial_puthex)
ENDPROC(early_serial_putc64)
ENDPROC(early_serial_puts64)
