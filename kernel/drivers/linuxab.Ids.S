/* SPDX-License-Identifier: GPL-2.0 */
/*
 * linuxab/arch/x86/kernel/linuxab.lds.S
 * Linker script for x86_64 linuxab kernel
 */

OUTPUT_FORMAT("elf64-x86-64", "elf64-x86-64", "elf64-x86-64")
OUTPUT_ARCH(i386:x86-64)

ENTRY(linuxab_start)

SECTIONS
{
    /* Kernel load address at 1MB + 2MB offset for higher half mapping */
    . = 0xFFFFFFFF80000000 + 0x100000;

    _text = .;
    .text : AT(ADDR(.text) - 0xFFFFFFFF80000000)
    {
        *(.text.linuxab_start)
        *(.text .text.*)
        *(.fixup)
        *(.gnu.warning)
    }
    _etext = .;

    . = ALIGN(4096);
    _rodata = .;
    .rodata : AT(ADDR(.rodata) - 0xFFFFFFFF80000000)
    {
        *(.rodata .rodata.*)
        *(.pci_fixup)
    }
    
    /* Exception table for kernel */
    __ex_table : AT(ADDR(__ex_table) - 0xFFFFFFFF80000000)
    {
        __start___ex_table = .;
        *(__ex_table)
        __stop___ex_table = .;
    }
    
    /* Kernel symbol table (if CONFIG_KALLSYMS) */
    __ksymtab : AT(ADDR(__ksymtab) - 0xFFFFFFFF80000000)
    {
        __start___ksymtab = .;
        *(__ksymtab)
        __stop___ksymtab = .;
    }
    
    __ksymtab_gpl : AT(ADDR(__ksymtab_gpl) - 0xFFFFFFFF80000000)
    {
        __start___ksymtab_gpl = .;
        *(__ksymtab_gpl)
        __stop___ksymtab_gpl = .;
    }
    
    _erodata = .;

    . = ALIGN(4096);
    _data = .;
    .data : AT(ADDR(.data) - 0xFFFFFFFF80000000)
    {
        *(.data .data.*)
        *(.init.data)
        
        /* Constructor/destructor tables */
        __ctors_start = .;
        *(.ctors)
        *(.init_array)
        __ctors_end = .;
    }
    
    . = ALIGN(4096);
    _bss = .;
    .bss : AT(ADDR(.bss) - 0xFFFFFFFF80000000)
    {
        *(.bss .bss.*)
        *(COMMON)
    }
    _ebss = .;
    
    /* Per-CPU data section (simplified) */
    . = ALIGN(4096);
    __per_cpu_start = .;
    .data.percpu : AT(ADDR(.data.percpu) - 0xFFFFFFFF80000000)
    {
        *(.data.percpu)
        *(.data.percpu.*)
    }
    __per_cpu_end = .;
    
    _edata = .;

    /* Init sections - can be freed after boot */
    . = ALIGN(4096);
    _sinittext = .;
    .init.text : AT(ADDR(.init.text) - 0xFFFFFFFF80000000)
    {
        *(.init.text)
    }
    _einittext = .;
    
    .init.data : AT(ADDR(.init.data) - 0xFFFFFFFF80000000)
    {
        *(.init.data)
    }
    
    . = ALIGN(4096);
    __initramfs_start = .;
    .init.ramfs : AT(ADDR(.init.ramfs) - 0xFFFFFFFF80000000)
    {
        *(.init.ramfs)
    }
    __initramfs_end = .;

    /* Discard unused sections */
    /DISCARD/ :
    {
        *(.comment)
        *(.note)
        *(.note.gnu.build-id)
        *(.eh_frame)
        *(.gnu.hash)
    }

    /* Kernel image end */
    _end = .;

    /* Provide symbols for assembly */
    _text_phys = _text - 0xFFFFFFFF80000000;
    _data_phys = _data - 0xFFFFFFFF80000000;
}
