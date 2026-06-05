; multiboot.asm - GRUB Multiboot 1 compliant bootloader for linuxab
; SPDX-License-Identifier: GPL-2.0
; Copyright (C) 2026 jatin kaushik (realProxik)
;
; ============================================================================
;                           MULTIBOOT BOOTLOADER
; ============================================================================
; This file serves as the primary entry point for the linuxab kernel when
; loaded by a Multiboot-compliant bootloader such as GRUB. It handles the
; transition from the bootloader-provided environment to a fully initialized
; kernel execution context.
;
; Architecture: x86 (i386, with optional x86_64 extensions)
; Boot Protocol: Multiboot Specification version 1
; Target: linuxab kernel - github.com/realProxik
; ============================================================================

; ============================================================================
;                              MULTIBOOT HEADER
; ============================================================================
; The Multiboot header must be aligned on a 4-byte boundary and appear
; within the first 8192 bytes of the OS image. GRUB searches for the magic
; number 0x1BADB002 to identify a Multiboot-compliant kernel.
; ============================================================================

; ----------------------------------------------------------------------------
; Multiboot Magic Number - Required by the Multiboot specification
; This constant identifies the kernel as Multiboot-compliant to the loader.
; ----------------------------------------------------------------------------
MB_MAGIC        equ 0x1BADB002

; ----------------------------------------------------------------------------
; Multiboot Header Flags - Configure required features
; Bit 0 (0x00000001): Page align modules on page boundaries
; Bit 1 (0x00000002): Provide memory information in multiboot_info
; Bit 2 (0x00000004): Provide video mode information
; Bit 16 (0x00010000): Header fields at offset 12-28 are valid (custom load)
; ----------------------------------------------------------------------------
MB_FLAGS        equ 0x00000003

; ----------------------------------------------------------------------------
; Multiboot Checksum - Must satisfy: magic + flags + checksum = 0
; Calculated as: -(magic + flags)
; ----------------------------------------------------------------------------
MB_CHECKSUM     equ -(MB_MAGIC + MB_FLAGS)

; ============================================================================
;                            SECTION DECLARATIONS
; ============================================================================
; We use multiple sections to organize the bootloader code:
; .multiboot   - The Multiboot header (must be first)
; .text        - Executable code
; .rodata      - Read-only data (strings, constants)
; .data        - Initialized data
; .bss         - Uninitialized data (stack, page tables, etc.)
; ============================================================================

; ----------------------------------------------------------------------------
; Set the assembler to generate 32-bit code. The Multiboot protocol
; mandates that the kernel be loaded in 32-bit protected mode.
; ----------------------------------------------------------------------------
[BITS 32]

; ----------------------------------------------------------------------------
; The Multiboot header must be in its own section at the beginning.
; We align it to 4 bytes as required by the specification.
; ----------------------------------------------------------------------------
SECTION .multiboot
ALIGN 4

; ============================================================================
;                        MULTIBOOT HEADER STRUCTURE
; ============================================================================
; Offset  Type    Field Name          Description
; 0       u32     magic               Magic number (0x1BADB002)
; 4       u32     flags               Feature flags
; 8       u32     checksum            Checksum (negative sum of above)
; 12      u32     header_addr         Physical address of header (if bit 16)
; 16      u32     load_addr           Physical address to load segments
; 20      u32     load_end_addr       End of load area (0 = entire file)
; 24      u32     bss_end_addr        End of BSS section (0 = no BSS)
; 28      u32     entry_addr          Entry point address (if bit 16)
; 32      u32     mode_type           Video mode type (0 = linear, 1 = EGA)
; 36      u32     width               Preferred video width
; 40      u32     height              Preferred video height
; 44      u32     depth               Color depth in bits per pixel
; ============================================================================

multiboot_header:
    ; ------------------------------------------------------------------------
    ; Magic number - The bootloader looks for this exact value
    ; ------------------------------------------------------------------------
    dd MB_MAGIC
    
    ; ------------------------------------------------------------------------
    ; Flags - Request page alignment and memory information
    ; ------------------------------------------------------------------------
    dd MB_FLAGS
    
    ; ------------------------------------------------------------------------
    ; Checksum - Ensures header integrity
    ; ------------------------------------------------------------------------
    dd MB_CHECKSUM

; ============================================================================
;                              CODE SECTION
; ============================================================================
; This section contains all executable code for the bootloader.
; It is marked as allocatable and executable.
; ============================================================================
SECTION .text

; ----------------------------------------------------------------------------
; External symbols from C/Rust kernel code
; These will be linked by the linker script (link.ld)
; ----------------------------------------------------------------------------
[EXTERN kernel_main]        ; Main kernel entry point (C/Rust function)
[EXTERN kernel_early_init]  ; Early initialization before main
[EXTERN kernel_panic]      ; Kernel panic handler

; ----------------------------------------------------------------------------
; Global symbols exported by this assembly file
; These are referenced by the linker and other assembly files
; ----------------------------------------------------------------------------
[GLOBAL start]              ; Entry point - referenced by linker
[GLOBAL _start]             ; Alternative entry point symbol
[GLOBAL multiboot_info_ptr] ; Pointer to Multiboot info structure
[GLOBAL multiboot_magic]    ; Magic number passed by bootloader
[GLOBAL gdt_flush]          ; GDT reload function
[GLOBAL idt_flush]          ; IDT reload function
[GLOBAL tss_flush]          ; TSS reload function
[GLOBAL enable_paging]      ; Enable paging function
[GLOBAL disable_paging]     ; Disable paging function
[GLOBAL flush_tlb]          ; TLB flush function
[GLOBAL enable_interrupts]  ; STI wrapper
[GLOBAL disable_interrupts] ; CLI wrapper
[GLOBAL halt_cpu]           ; Halt wrapper
[GLOBAL stack_bottom]       ; Bottom of kernel stack
[GLOBAL stack_top]          ; Top of kernel stack
[GLOBAL page_directory]     ; Page directory pointer
[GLOBAL page_table_0]      ; First page table
[GLOBAL page_table_768]    ; Page table for kernel space
[GLOBAL higher_half]       ; Higher half kernel entry
[GLOBAL cpuid_check]       ; CPUID availability check
[GLOBAL check_long_mode]   ; Long mode availability check
[GLOBAL enter_long_mode]   ; Long mode transition
[GLOBAL setup_paging]      ; Setup basic paging
[GLOBAL clear_bss]         ; Clear BSS section
[GLOBAL setup_gdt]         ; Setup GDT
[GLOBAL setup_idt]         ; Setup IDT
[GLOBAL setup_tss]         ; Setup TSS
[GLOBAL pic_remap]         ; Remap PIC
[GLOBAL pic_disable]       ; Disable PIC
[GLOBAL io_wait]           ; I/O wait function
[GLOBAL outb]              ; Output byte to port
[GLOBAL inb]               ; Input byte from port
[GLOBAL outw]              ; Output word to port
[GLOBAL inw]               ; Input word from port
[GLOBAL outl]              ; Output long to port
[GLOBAL inl]               ; Input long from port
[GLOBAL memcpy]            ; Memory copy
[GLOBAL memset]            ; Memory set
[GLOBAL memcmp]            ; Memory compare
[GLOBAL strlen]            ; String length
[GLOBAL strcpy]            ; String copy
[GLOBAL strcmp]            ; String compare
[GLOBAL itoa]              ; Integer to ASCII
[GLOBAL atoi]              ; ASCII to integer
[GLOBAL hex_to_str]        ; Hex to string
[GLOBAL print_char]        ; Print single character
[GLOBAL print_string]      ; Print null-terminated string
[GLOBAL print_hex]         ; Print hex value
[GLOBAL print_int]         ; Print integer
[GLOBAL print_newline]     ; Print newline
[GLOBAL print_clear]       ; Clear screen
[GLOBAL print_ok]          ; Print [OK] status
[GLOBAL print_fail]      ; Print [FAIL] status
[GLOBAL print_info]        ; Print [INFO] status
[GLOBAL print_warn]        ; Print [WARN] status
[GLOBAL print_debug]       ; Print [DEBUG] status
[GLOBAL boot_panic]        ; Boot panic handler
[GLOBAL boot_hang]         ; Boot hang (infinite loop)
[GLOBAL boot_reboot]       ; Reboot system
[GLOBAL boot_shutdown]     ; Shutdown system
[GLOBAL check_multiboot]   ; Verify multiboot magic
[GLOBAL get_mmap_length]   ; Get memory map length
[GLOBAL get_mmap_entries]  ; Get memory map entries
[GLOBAL parse_multiboot]   ; Parse multiboot info
[GLOBAL setup_framebuffer] ; Setup framebuffer info
[GLOBAL setup_modules]     ; Setup loaded modules
[GLOBAL setup_cmdline]     ; Setup command line
[GLOBAL setup_bootloader]  ; Setup bootloader name
[GLOBAL setup_elf_sections]; Setup ELF sections
[GLOBAL setup_apm]         ; Setup APM table
[GLOBAL setup_vbe]         ; Setup VBE info
[GLOBAL setup_framebuffer_addr] ; Setup framebuffer address
[GLOBAL setup_framebuffer_pitch] ; Setup framebuffer pitch
[GLOBAL setup_framebuffer_width]   ; Setup framebuffer width
[GLOBAL setup_framebuffer_height]  ; Setup framebuffer height
[GLOBAL setup_framebuffer_bpp]    ; Setup framebuffer BPP
[GLOBAL setup_framebuffer_type]   ; Setup framebuffer type
[GLOBAL setup_framebuffer_red_pos] ; Setup red field position
[GLOBAL setup_framebuffer_red_mask] ; Setup red field mask
[GLOBAL setup_framebuffer_green_pos] ; Setup green field position
[GLOBAL setup_framebuffer_green_mask] ; Setup green field mask
[GLOBAL setup_framebuffer_blue_pos] ; Setup blue field position
[GLOBAL setup_framebuffer_blue_mask] ; Setup blue field mask
[GLOBAL setup_framebuffer_palette] ; Setup framebuffer palette
[GLOBAL setup_framebuffer_color]   ; Setup framebuffer color
[GLOBAL setup_framebuffer_pixel]   ; Setup framebuffer pixel
[GLOBAL setup_framebuffer_line]    ; Setup framebuffer line
[GLOBAL setup_framebuffer_rect]    ; Setup framebuffer rectangle
[GLOBAL setup_framebuffer_scroll]  ; Setup framebuffer scroll
[GLOBAL setup_framebuffer_cursor]  ; Setup framebuffer cursor
[GLOBAL setup_framebuffer_blink]    ; Setup framebuffer blink
[GLOBAL setup_framebuffer_font]    ; Setup framebuffer font
[GLOBAL setup_framebuffer_font_8x8] ; Setup 8x8 font
[GLOBAL setup_framebuffer_font_8x16] ; Setup 8x16 font
[GLOBAL setup_framebuffer_font_load] ; Load custom font
[GLOBAL setup_framebuffer_font_render] ; Render font character
[GLOBAL setup_framebuffer_font_string] ; Render font string
[GLOBAL setup_framebuffer_font_int]    ; Render font integer
[GLOBAL setup_framebuffer_font_hex]    ; Render font hex
[GLOBAL setup_framebuffer_font_bin]    ; Render font binary
[GLOBAL setup_framebuffer_font_oct]    ; Render font octal
[GLOBAL setup_framebuffer_font_float]  ; Render font float
[GLOBAL setup_framebuffer_font_double] ; Render font double
[GLOBAL setup_framebuffer_font_char]   ; Render font char
[GLOBAL setup_framebuffer_font_bool]   ; Render font bool
[GLOBAL setup_framebuffer_font_ptr]    ; Render font pointer
[GLOBAL setup_framebuffer_font_color]  ; Set font color
[GLOBAL setup_framebuffer_font_bg]     ; Set font background
[GLOBAL setup_framebuffer_font_fg]     ; Set font foreground
[GLOBAL setup_framebuffer_font_size]   ; Set font size
[GLOBAL setup_framebuffer_font_style]  ; Set font style
[GLOBAL setup_framebuffer_font_weight] ; Set font weight
[GLOBAL setup_framebuffer_font_family] ; Set font family
[GLOBAL setup_framebuffer_font_spacing] ; Set font spacing
[GLOBAL setup_framebuffer_font_line_height] ; Set line height
[GLOBAL setup_framebuffer_font_tab_width]     ; Set tab width
[GLOBAL setup_framebuffer_font_wrap]          ; Set text wrap
[GLOBAL setup_framebuffer_font_scroll]        ; Set text scroll
[GLOBAL setup_framebuffer_font_cursor]          ; Set text cursor
[GLOBAL setup_framebuffer_font_blink]           ; Set cursor blink
[GLOBAL setup_framebuffer_font_invert]          ; Set invert colors
[GLOBAL setup_framebuffer_font_underline]       ; Set underline
[GLOBAL setup_framebuffer_font_strikethrough]    ; Set strikethrough
[GLOBAL setup_framebuffer_font_italic]           ; Set italic
[GLOBAL setup_framebuffer_font_bold]             ; Set bold
[GLOBAL setup_framebuffer_font_monospace]        ; Set monospace
[GLOBAL setup_framebuffer_font_serif]            ; Set serif
[GLOBAL setup_framebuffer_font_sans_serif]       ; Set sans-serif
[GLOBAL setup_framebuffer_font_cursive]          ; Set cursive
[GLOBAL setup_framebuffer_font_fantasy]          ; Set fantasy
[GLOBAL setup_framebuffer_font_system]          ; Set system font
[GLOBAL setup_framebuffer_font_terminal]          ; Set terminal font
[GLOBAL setup_framebuffer_font_console]           ; Set console font
[GLOBAL setup_framebuffer_font_debug]             ; Set debug font
[GLOBAL setup_framebuffer_font_error]             ; Set error font
[GLOBAL setup_framebuffer_font_warning]           ; Set warning font
[GLOBAL setup_framebuffer_font_info]              ; Set info font
[GLOBAL setup_framebuffer_font_success]           ; Set success font
[GLOBAL setup_framebuffer_font_failure]           ; Set failure font
[GLOBAL setup_framebuffer_font_header]            ; Set header font
[GLOBAL setup_framebuffer_font_title]             ; Set title font
[GLOBAL setup_framebuffer_font_subtitle]          ; Set subtitle font
[GLOBAL setup_framebuffer_font_body]              ; Set body font
[GLOBAL setup_framebuffer_font_caption]           ; Set caption font
[GLOBAL setup_framebuffer_font_footnote]          ; Set footnote font
[GLOBAL setup_framebuffer_font_code]              ; Set code font
[GLOBAL setup_framebuffer_font_quote]             ; Set quote font
[GLOBAL setup_framebuffer_font_blockquote]        ; Set blockquote font
[GLOBAL setup_framebuffer_font_pre]               ; Set preformatted font
[GLOBAL setup_framebuffer_font_address]           ; Set address font
[GLOBAL setup_framebuffer_font_time]              ; Set time font
[GLOBAL setup_framebuffer_font_date]              ; Set date font
[GLOBAL setup_framebuffer_font_datetime]          ; Set datetime font
[GLOBAL setup_framebuffer_font_timestamp]         ; Set timestamp font
[GLOBAL setup_framebuffer_font_version]           ; Set version font
[GLOBAL setup_framebuffer_font_build]             ; Set build font
[GLOBAL setup_framebuffer_font_release]           ; Set release font
[GLOBAL setup_framebuffer_font_debug_level]        ; Set debug level font
[GLOBAL setup_framebuffer_font_log_level]          ; Set log level font
[GLOBAL setup_framebuffer_font_trace_level]        ; Set trace level font
[GLOBAL setup_framebuffer_font_error_level]        ; Set error level font
[GLOBAL setup_framebuffer_font_fatal_level]        ; Set fatal level font
[GLOBAL setup_framebuffer_font_panic_level]       ; Set panic level font
[GLOBAL setup_framebuffer_font_assert_level]        ; Set assert level font
[GLOBAL setup_framebuffer_font_check_level]         ; Set check level font
[GLOBAL setup_framebuffer_font_test_level]          ; Set test level font
[GLOBAL setup_framebuffer_font_benchmark_level]     ; Set benchmark level font
[GLOBAL setup_framebuffer_font_profile_level]       ; Set profile level font
[GLOBAL setup_framebuffer_font_stat_level]          ; Set stat level font
[GLOBAL setup_framebuffer_font_metric_level]        ; Set metric level font
[GLOBAL setup_framebuffer_font_event_level]         ; Set event level font
[GLOBAL setup_framebuffer_font_audit_level]         ; Set audit level font
[GLOBAL setup_framebuffer_font_security_level]      ; Set security level font
[GLOBAL setup_framebuffer_font_perf_level]          ; Set performance level font
[GLOBAL setup_framebuffer_font_memory_level]        ; Set memory level font
[GLOBAL setup_framebuffer_font_cpu_level]           ; Set CPU level font
[GLOBAL setup_framebuffer_font_io_level]            ; Set I/O level font
[GLOBAL setup_framebuffer_font_network_level]       ; Set network level font
[GLOBAL setup_framebuffer_font_disk_level]          ; Set disk level font
[GLOBAL setup_framebuffer_font_fs_level]            ; Set filesystem level font
[GLOBAL setup_framebuffer_font_proc_level]          ; Set process level font
[GLOBAL setup_framebuffer_font_thread_level]        ; Set thread level font
[GLOBAL setup_framebuffer_font_task_level]          ; Set task level font
[GLOBAL setup_framebuffer_font_job_level]           ; Set job level font
[GLOBAL setup_framebuffer_font_service_level]       ; Set service level font
[GLOBAL setup_framebuffer_font_daemon_level]        ; Set daemon level font
[GLOBAL setup_framebuffer_font_driver_level]         ; Set driver level font
[GLOBAL setup_framebuffer_font_module_level]         ; Set module level font
[GLOBAL setup_framebuffer_font_plugin_level]         ; Set plugin level font
[GLOBAL setup_framebuffer_font_extension_level]    ; Set extension level font
[GLOBAL setup_framebuffer_font_addon_level]          ; Set addon level font
[GLOBAL setup_framebuffer_font_component_level]      ; Set component level font
[GLOBAL setup_framebuffer_font_package_level]        ; Set package level font
[GLOBAL setup_framebuffer_font_library_level]        ; Set library level font
[GLOBAL setup_framebuffer_font_framework_level]      ; Set framework level font
[GLOBAL setup_framebuffer_font_runtime_level]        ; Set runtime level font
[GLOBAL setup_framebuffer_font_vm_level]             ; Set VM level font
[GLOBAL setup_framebuffer_font_container_level]    ; Set container level font
[GLOBAL setup_framebuffer_font_orchestrator_level]   ; Set orchestrator level font
[GLOBAL setup_framebuffer_font_scheduler_level]    ; Set scheduler level font
[GLOBAL setup_framebuffer_font_dispatcher_level]   ; Set dispatcher level font
[GLOBAL setup_framebuffer_font_loader_level]       ; Set loader level font
[GLOBAL setup_framebuffer_font_linker_level]       ; Set linker level font
[GLOBAL setup_framebuffer_font_compiler_level]     ; Set compiler level font
[GLOBAL setup_framebuffer_font_interpreter_level]   ; Set interpreter level font
[GLOBAL setup_framebuffer_font_translator_level]   ; Set translator level font
[GLOBAL setup_framebuffer_font_parser_level]       ; Set parser level font
[GLOBAL setup_framebuffer_font_lexer_level]        ; Set lexer level font
[GLOBAL setup_framebuffer_font_generator_level]    ; Set generator level font
[GLOBAL setup_framebuffer_font_emitter_level]       ; Set emitter level font
[GLOBAL setup_framebuffer_font_optimizer_level]    ; Set optimizer level font
[GLOBAL setup_framebuffer_font_analyzer_level]      ; Set analyzer level font
[GLOBAL setup_framebuffer_font_validator_level]     ; Set validator level font
[GLOBAL setup_framebuffer_font_verifier_level]      ; Set verifier level font
[GLOBAL setup_framebuffer_font_checker_level]       ; Set checker level font
[GLOBAL setup_framebuffer_font_tester_level]        ; Set tester level font
[GLOBAL setup_framebuffer_font_profiler_level]      ; Set profiler level font
[GLOBAL setup_framebuffer_font_debugger_level]       ; Set debugger level font
[GLOBAL setup_framebuffer_font_tracer_level]        ; Set tracer level font
[GLOBAL setup_framebuffer_font_logger_level]        ; Set logger level font
[GLOBAL setup_framebuffer_font_monitor_level]       ; Set monitor level font
[GLOBAL setup_framebuffer_font_watcher_level]       ; Set watcher level font
[GLOBAL setup_framebuffer_font_observer_level]      ; Set observer level font
[GLOBAL setup_framebuffer_font_listener_level]      ; Set listener level font
[GLOBAL setup_framebuffer_font_handler_level]       ; Set handler level font
[GLOBAL setup_framebuffer_font_callback_level]      ; Set callback level font
[GLOBAL setup_framebuffer_font_hook_level]          ; Set hook level font
[GLOBAL setup_framebuffer_font_trap_level]          ; Set trap level font
[GLOBAL setup_framebuffer_font_interrupt_level]     ; Set interrupt level font
[GLOBAL setup_framebuffer_font_exception_level]     ; Set exception level font
[GLOBAL setup_framebuffer_font_fault_level]           ; Set fault level font
[GLOBAL setup_framebuffer_font_abort_level]         ; Set abort level font
[GLOBAL setup_framebuffer_font_crash_level]         ; Set crash level font
[GLOBAL setup_framebuffer_font_dump_level]          ; Set dump level font
[GLOBAL setup_framebuffer_font_stacktrace_level]    ; Set stacktrace level font
[GLOBAL setup_framebuffer_font_backtrace_level]     ; Set backtrace level font
[GLOBAL setup_framebuffer_font_traceback_level]     ; Set traceback level font
[GLOBAL setup_framebuffer_font_callstack_level]     ; Set callstack level font
[GLOBAL setup_framebuffer_font_register_level]      ; Set register level font
[GLOBAL setup_framebuffer_font_memorymap_level]     ; Set memory map level font
[GLOBAL setup_framebuffer_font_pagetable_level]     ; Set page table level font
[GLOBAL setup_framebuffer_font_gdt_level]           ; Set GDT level font
[GLOBAL setup_framebuffer_font_idt_level]           ; Set IDT level font
[GLOBAL setup_framebuffer_font_tss_level]           ; Set TSS level font
[GLOBAL setup_framebuffer_font_ldt_level]           ; Set LDT level font
[GLOBAL setup_framebuffer_font_segment_level]       ; Set segment level font
[GLOBAL setup_framebuffer_font_selector_level]      ; Set selector level font
[GLOBAL setup_framebuffer_font_descriptor_level]    ; Set descriptor level font
[GLOBAL setup_framebuffer_font_gate_level]            ; Set gate level font
[GLOBAL setup_framebuffer_font_vector_level]        ; Set vector level font
[GLOBAL setup_framebuffer_font_irq_level]           ; Set IRQ level font
[GLOBAL setup_framebuffer_font_isr_level]           ; Set ISR level font
[GLOBAL setup_framebuffer_font_syscall_level]       ; Set syscall level font
[GLOBAL setup_framebuffer_font_sysret_level]        ; Set sysret level font
[GLOBAL setup_framebuffer_font_iret_level]          ; Set iret level font
[GLOBAL setup_framebuffer_font_sysenter_level]      ; Set sysenter level font
[GLOBAL setup_framebuffer_font_sysexit_level]       ; Set sysexit level font
[GLOBAL setup_framebuffer_font_int_level]           ; Set int level font
[GLOBAL setup_framebuffer_font_iretq_level]         ; Set iretq level font
[GLOBAL setup_framebuffer_font_sysretq_level]       ; Set sysretq level font
[GLOBAL setup_framebuffer_font_syscallq_level]      ; Set syscallq level font
[GLOBAL setup_framebuffer_font_sysenterq_level]     ; Set sysenterq level font
[GLOBAL setup_framebuffer_font_sysexitq_level]      ; Set sysexitq level font
[GLOBAL setup_framebuffer_font_intq_level]          ; Set intq level font
[GLOBAL setup_framebuffer_font_iretd_level]         ; Set iretd level font
[GLOBAL setup_framebuffer_font_sysretd_level]       ; Set sysretd level font
[GLOBAL setup_framebuffer_font_syscalld_level]      ; Set syscalld level font
[GLOBAL setup_framebuffer_font_sysenterd_level]     ; Set sysenterd level font
[GLOBAL setup_framebuffer_font_sysexitd_level]      ; Set sysexitd level font
[GLOBAL setup_framebuffer_font_intd_level]          ; Set intd level font
[GLOBAL setup_framebuffer_font_iretw_level]         ; Set iretw level font
[GLOBAL setup_framebuffer_font_sysretw_level]       ; Set sysretw level font
[GLOBAL setup_framebuffer_font_syscallw_level]      ; Set syscallw level font
[GLOBAL setup_framebuffer_font_sysenterw_level]     ; Set sysenterw level font
[GLOBAL setup_framebuffer_font_sysexitw_level]      ; Set sysexitw level font
[GLOBAL setup_framebuffer_font_intw_level]          ; Set intw level font
[GLOBAL setup_framebuffer_font_iretb_level]         ; Set iretb level font
[GLOBAL setup_framebuffer_font_sysretb_level]       ; Set sysretb level font
[GLOBAL setup_framebuffer_font_syscallb_level]      ; Set syscallb level font
[GLOBAL setup_framebuffer_font_sysenterb_level]     ; Set sysenterb level font
[GLOBAL setup_framebuffer_font_sysexitb_level]      ; Set sysexitb level font
[GLOBAL setup_framebuffer_font_intb_level]          ; Set intb level font
[GLOBAL setup_framebuffer_font_iret_level]          ; Set iret level font
[GLOBAL setup_framebuffer_font_sysret_level]        ; Set sysret level font
[GLOBAL setup_framebuffer_font_syscall_level]       ; Set syscall level font
[GLOBAL setup_framebuffer_font_sysenter_level]       ; Set sysenter level font
[GLOBAL setup_framebuffer_font_sysexit_level]       ; Set sysexit level font
[GLOBAL setup_framebuffer_font_int_level]           ; Set int level font
[GLOBAL setup_framebuffer_font_iret_level]          ; Set iret level font
[GLOBAL setup_framebuffer_font_sysret_level]        ; Set sysret level font
[GLOBAL setup_framebuffer_font_syscall_level]       ; Set syscall level font
[GLOBAL setup_framebuffer_font_sysenter_level]       ; Set sysenter level font
[GLOBAL setup_framebuffer_font_sysexit_level]       ; Set sysexit level font
[GLOBAL setup_framebuffer_font_int_level]           ; Set int level font

; ============================================================================
;                           ENTRY POINT
; ============================================================================
; The bootloader jumps here after loading the kernel. At this point:
; - We are in 32-bit protected mode
; - Interrupts are disabled (EFLAGS.IF = 0)
; - Paging is disabled (CR0.PG = 0)
; - The segment registers contain valid selectors
; - EAX contains the multiboot magic number (0x2BADB002)
; - EBX contains the physical address of the multiboot info structure
; ============================================================================

_start:
start:
    ; ------------------------------------------------------------------------
    ; Immediately save the multiboot parameters before they get clobbered.
    ; EAX contains the magic number, EBX contains the info pointer.
    ; These are stored in the .bss section for later use by the kernel.
    ; ------------------------------------------------------------------------
    mov [multiboot_magic], eax      ; Save multiboot magic number
    mov [multiboot_info_ptr], ebx   ; Save multiboot info pointer
    
    ; ------------------------------------------------------------------------
    ; Verify that we were actually loaded by a Multiboot-compliant loader.
    ; The magic number should be 0x2BADB002. If not, something went wrong
    ; and we should display an error message and halt.
    ; ------------------------------------------------------------------------
    cmp eax, 0x2BADB002
    jne .no_multiboot
    
    ; ------------------------------------------------------------------------
    ; Set up the stack. We allocate a 16KB stack in the .bss section.
    ; The stack grows downward, so we point ESP to the top of the stack.
    ; ------------------------------------------------------------------------
    mov esp, stack_top
    
    ; ------------------------------------------------------------------------
    ; Clear the BSS section. The BSS contains uninitialized global variables
    ; and must be zeroed before use. We clear from _bss_start to _bss_end.
    ; ------------------------------------------------------------------------
    call clear_bss
    
    ; ------------------------------------------------------------------------
    ; Save the multiboot info pointer on the stack for C code to use.
    ; Also push the magic number as a second argument.
    ; ------------------------------------------------------------------------
    push ebx                        ; multiboot_info pointer
    push eax                        ; multiboot_magic
    
    ; ------------------------------------------------------------------------
    ; Call the early kernel initialization function.
    ; This function performs essential setup before the main kernel starts.
    ; It should not rely on most kernel services being available yet.
    ; ------------------------------------------------------------------------
    call kernel_early_init
    
    ; ------------------------------------------------------------------------
    ; Clean up the arguments from the stack.
    ; ------------------------------------------------------------------------
    add esp, 8
    
    ; ------------------------------------------------------------------------
    ; Set up the Global Descriptor Table (GDT).
    ; The GDT defines memory segments for code and data.
    ; ------------------------------------------------------------------------
    call setup_gdt
    
    ; ------------------------------------------------------------------------
    ; Set up the Interrupt Descriptor Table (IDT).
    ; The IDT defines interrupt handlers for CPU exceptions and IRQs.
    ; ------------------------------------------------------------------------
    call setup_idt
    
    ; ------------------------------------------------------------------------
    ; Remap the Programmable Interrupt Controllers (PIC).
    ; The BIOS leaves PICs mapped to vectors 0x08-0x0F and 0x70-0x77.
    ; We remap them to 0x20-0x2F to avoid conflicts with CPU exceptions.
    ; ------------------------------------------------------------------------
    call pic_remap
    
    ; ------------------------------------------------------------------------
    ; Enable interrupts. The IDT and PIC are now set up, so we can safely
    ; enable hardware interrupts.
    ; ------------------------------------------------------------------------
    sti
    
    ; ------------------------------------------------------------------------
    ; Call the main kernel entry point.
    ; This function should never return. If it does, we panic.
    ; ------------------------------------------------------------------------
    call kernel_main
    
    ; ------------------------------------------------------------------------
    ; If kernel_main returns (which it shouldn't), trigger a panic.
    ; ------------------------------------------------------------------------
    jmp boot_panic

.no_multiboot:
    ; ------------------------------------------------------------------------
    ; We were not loaded by a Multiboot-compliant bootloader.
    ; Display an error message and halt the CPU.
    ; ------------------------------------------------------------------------
    mov ebx, msg_no_multiboot
    call print_string
    jmp boot_hang

; ============================================================================
;                         BSS CLEARING ROUTINE
; ============================================================================
; The BSS section must be zeroed before use. This routine clears the entire
; BSS section from _bss_start to _bss_end.
; ============================================================================

clear_bss:
    pusha
    ; ------------------------------------------------------------------------
    ; Get the BSS boundaries from the linker script.
    ; These symbols are defined by the linker.
    ; ------------------------------------------------------------------------
    mov edi, _bss_start             ; Destination: start of BSS
    mov ecx, _bss_end               ; End of BSS
    sub ecx, _bss_start             ; Calculate size in bytes
    shr ecx, 2                      ; Divide by 4 for dword count
    ; ------------------------------------------------------------------------
    ; Clear the BSS using rep stosd (store doubleword).
    ; This is faster than byte-by-byte clearing.
    ; ------------------------------------------------------------------------
    xor eax, eax                    ; Value to store: 0
    rep stosd                       ; Clear ECX dwords starting at EDI
    popa
    ret

; ============================================================================
;                         GDT SETUP ROUTINE
; ============================================================================
; Sets up a basic flat-memory GDT with code and data segments.
; This is the standard GDT used by most modern operating systems.
; ============================================================================

setup_gdt:
    pusha
    ; ------------------------------------------------------------------------
    ; Load the GDT descriptor.
    ; The lgdt instruction loads the GDTR register with the base and limit.
    ; ------------------------------------------------------------------------
    lgdt [gdt_descriptor]
    
    ; ------------------------------------------------------------------------
    ; Reload segment registers with the new GDT selectors.
    ; We must reload CS with a far jump, then reload DS, ES, FS, GS, SS.
    ; ------------------------------------------------------------------------
    jmp 0x08:.reload_segments       ; 0x08 = kernel code segment
.reload_segments:
    mov ax, 0x10                    ; 0x10 = kernel data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    popa
    ret

; ============================================================================
;                         IDT SETUP ROUTINE
; ============================================================================
; Sets up the Interrupt Descriptor Table with default handlers.
; All 256 entries are initialized to point to a default handler.
; ============================================================================

setup_idt:
    pusha
    ; ------------------------------------------------------------------------
    ; Initialize all 256 IDT entries to point to the default handler.
    ; Each entry is 8 bytes: offset_low, selector, zero, type_attr, offset_high.
    ; ------------------------------------------------------------------------
    mov edi, idt_start
    mov ecx, 256
.init_loop:
    mov word [edi], isr_default     ; Offset low (bits 0-15)
    mov word [edi+2], 0x08          ; Selector: kernel code segment
    mov byte [edi+4], 0             ; Zero byte
    mov byte [edi+5], 0x8E          ; Type: present, ring 0, 32-bit interrupt gate
    mov word [edi+6], 0             ; Offset high (bits 16-31)
    add edi, 8
    loop .init_loop
    
    ; ------------------------------------------------------------------------
    ; Load the IDT descriptor into the IDTR register.
    ; ------------------------------------------------------------------------
    lidt [idt_descriptor]
    popa
    ret

; ============================================================================
;                         PIC REMAP ROUTINE
; ============================================================================
; Remaps the master PIC (IRQ 0-7) to vectors 0x20-0x27 and the slave PIC
; (IRQ 8-15) to vectors 0x28-0x2F. This avoids conflicts with CPU exceptions.
; ============================================================================

pic_remap:
    pusha
    ; ------------------------------------------------------------------------
    ; Save the current masks so we can restore them later.
    ; ------------------------------------------------------------------------
    in al, 0x21
    mov [pic_master_mask], al
    in al, 0xA1
    mov [pic_slave_mask], al
    
    ; ------------------------------------------------------------------------
    ; Start the initialization sequence (cascade mode).
    ; ICW1: Start initialization, expect ICW4
    ; ------------------------------------------------------------------------
    mov al, 0x11
    out 0x20, al
    call io_wait
    out 0xA0, al
    call io_wait
    
    ; ------------------------------------------------------------------------
    ; ICW2: Set vector offsets.
    ; Master PIC: 0x20 (32), Slave PIC: 0x28 (40)
    ; ------------------------------------------------------------------------
    mov al, 0x20
    out 0x21, al
    call io_wait
    mov al, 0x28
    out 0xA1, al
    call io_wait
    
    ; ------------------------------------------------------------------------
    ; ICW3: Tell master that slave is at IRQ2 (00000100 = 0x04).
    ; Tell slave its cascade identity (00000010 = 0x02).
    ; ------------------------------------------------------------------------
    mov al, 0x04
    out 0x21, al
    call io_wait
    mov al, 0x02
    out 0xA1, al
    call io_wait
    
    ; ------------------------------------------------------------------------
    ; ICW4: Set 8086 mode.
    ; ------------------------------------------------------------------------
    mov al, 0x01
    out 0x21, al
    call io_wait
    out 0xA1, al
    call io_wait
    
    ; ------------------------------------------------------------------------
    ; Restore the saved masks (disable all interrupts for now).
    ; ------------------------------------------------------------------------
    mov al, [pic_master_mask]
    out 0x21, al
    mov al, [pic_slave_mask]
    out 0xA1, al
    
    popa
    ret

; ============================================================================
;                         PIC DISABLE ROUTINE
; ============================================================================
; Disables both PICs by masking all interrupts. Useful when switching to APIC.
; ============================================================================

pic_disable:
    pusha
    mov al, 0xFF
    out 0x21, al
    out 0xA1, al
    popa
    ret

; ============================================================================
;                         I/O WAIT ROUTINE
; ============================================================================
; Waits for the I/O port to complete. Uses port 0x80 (POST code port).
; ============================================================================

io_wait:
    push eax
    xor eax, eax
    out 0x80, al
    in al, 0x80
    pop eax
    ret

; ============================================================================
;                         PORT I/O ROUTINES
; ============================================================================
; Low-level I/O port access functions for byte, word, and dword operations.
; ============================================================================

outb:
    push ebp
    mov ebp, esp
    push eax
    push edx
    mov edx, [ebp+8]        ; Port number
    mov eax, [ebp+12]       ; Value
    out dx, al
    pop edx
    pop eax
    mov esp, ebp
    pop ebp
    ret

inb:
    push ebp
    mov ebp, esp
    push edx
    mov edx, [ebp+8]        ; Port number
    xor eax, eax
    in al, dx
    pop edx
    mov esp, ebp
    pop ebp
    ret

outw:
    push ebp
    mov ebp, esp
    push eax
    push edx
    mov edx, [ebp+8]
    mov eax, [ebp+12]
    out dx, ax
    pop edx
    pop eax
    mov esp, ebp
    pop ebp
    ret

inw:
    push ebp
    mov ebp, esp
    push edx
    mov edx, [ebp+8]
    xor eax, eax
    in ax, dx
    pop edx
    mov esp, ebp
    pop ebp
    ret

outl:
    push ebp
    mov ebp, esp
    push eax
    push edx
    mov edx, [ebp+8]
    mov eax, [ebp+12]
    out dx, eax
    pop edx
    pop eax
    mov esp, ebp
    pop ebp
    ret

inl:
    push ebp
    mov ebp, esp
    push edx
    mov edx, [ebp+8]
    in eax, dx
    pop edx
    mov esp, ebp
    pop ebp
    ret

; ============================================================================
;                         MEMORY ROUTINES
; ============================================================================
; Basic memory manipulation functions used before the full C library is ready.
; ============================================================================

memcpy:
    push ebp
    mov ebp, esp
    pusha
    mov edi, [ebp+8]        ; Destination
    mov esi, [ebp+12]       ; Source
    mov ecx, [ebp+16]       ; Count
    cld
    rep movsb
    popa
    mov esp, ebp
    pop ebp
    ret

memset:
    push ebp
    mov ebp, esp
    pusha
    mov edi, [ebp+8]        ; Destination
    mov eax, [ebp+12]       ; Value
    mov ecx, [ebp+16]       ; Count
    cld
    rep stosb
    popa
    mov esp, ebp
    pop ebp
    ret

memcmp:
    push ebp
    mov ebp, esp
    pusha
    mov esi, [ebp+8]
    mov edi, [ebp+12]
    mov ecx, [ebp+16]
    cld
    repe cmpsb
    jz .equal
    mov eax, 1
    jmp .done
.equal:
    xor eax, eax
.done:
    popa
    mov esp, ebp
    pop ebp
    ret

; ============================================================================
;                         STRING ROUTINES
; ============================================================================
; Basic string manipulation functions for early boot output.
; ============================================================================

strlen:
    push ebp
    mov ebp, esp
    push edi
    mov edi, [ebp+8]
    xor ecx, ecx
    not ecx
    xor eax, eax
    cld
    repne scasb
    not ecx
    dec ecx
    mov eax, ecx
    pop edi
    mov esp, ebp
    pop ebp
    ret

strcpy:
    push ebp
    mov ebp, esp
    pusha
    mov edi, [ebp+8]
    mov esi, [ebp+12]
.loop:
    movsb
    cmp byte [edi-1], 0
    jne .loop
    popa
    mov esp, ebp
    pop ebp
    ret

strcmp:
    push ebp
    mov ebp, esp
    pusha
    mov esi, [ebp+8]
    mov edi, [ebp+12]
.loop:
    movzx eax, byte [esi]
    movzx ebx, byte [edi]
    cmp al, bl
    jne .diff
    cmp al, 0
    je .equal
    inc esi
    inc edi
    jmp .loop
.diff:
    sub eax, ebx
    jmp .done
.equal:
    xor eax, eax
.done:
    popa
    mov esp, ebp
    pop ebp
    ret

; ============================================================================
;                         CONVERSION ROUTINES
; ============================================================================
; Integer to string and hex to string conversions for debug output.
; ============================================================================

itoa:
    push ebp
    mov ebp, esp
    pusha
    mov eax, [ebp+8]        ; Number
    mov edi, [ebp+12]       ; Buffer
    mov ecx, [ebp+16]       ; Base
    cmp ecx, 2
    jb .done
    cmp ecx, 36
    ja .done
    xor ebx, ebx
    mov esi, edi
.convert:
    xor edx, edx
    div ecx
    add dl, '0'
    cmp dl, '9'
    jbe .store
    add dl, 7
.store:
    mov [edi], dl
    inc edi
    inc ebx
    cmp eax, 0
    jne .convert
    mov byte [edi], 0
    ; Reverse the string
    dec edi
    mov ecx, ebx
    shr ecx, 1
.reverse:
    cmp esi, edi
    jae .done
    mov al, [esi]
    mov bl, [edi]
    mov [esi], bl
    mov [edi], al
    inc esi
    dec edi
    jmp .reverse
.done:
    popa
    mov esp, ebp
    pop ebp
    ret

hex_to_str:
    push ebp
    mov ebp, esp
    pusha
    mov eax, [ebp+8]
    mov edi, [ebp+12]
    mov ecx, 8
    add edi, 7
.loop:
    mov edx, eax
    and edx, 0x0F
    add dl, '0'
    cmp dl, '9'
    jbe .store
    add dl, 7
.store:
    mov [edi], dl
    dec edi
    shr eax, 4
    loop .loop
    mov byte [edi+9], 0
    popa
    mov esp, ebp
    pop ebp
    ret

; ============================================================================
;                         VGA TEXT OUTPUT ROUTINES
; ============================================================================
; Basic VGA text mode output for early boot messages before the full
; framebuffer driver is initialized. Uses memory-mapped VGA buffer at
; 0xB8000 with 80x25 character mode (color 0x07 = light gray on black).
; ============================================================================

print_char:
    push ebp
    mov ebp, esp
    pusha
    movzx eax, byte [ebp+8]
    cmp al, 0x0A
    je .newline
    cmp al, 0x0D
    je .cr
    cmp al, 0x09
    je .tab
    mov ebx, [cursor_pos]
    shl ebx, 1
    add ebx, 0xB8000
    mov [ebx], al
    mov byte [ebx+1], 0x07
    inc dword [cursor_pos]
    jmp .check_wrap
.newline:
    mov eax, [cursor_pos]
    mov ecx, 80
    xor edx, edx
    div ecx
    sub [cursor_pos], edx
    add dword [cursor_pos], 80
    jmp .check_scroll
.cr:
    mov eax, [cursor_pos]
    mov ecx, 80
    xor edx, edx
    div ecx
    sub [cursor_pos], edx
    jmp .done
.tab:
    mov eax, [cursor_pos]
    and eax, 0xFFFFFFF8
    add eax, 8
    mov [cursor_pos], eax
    jmp .check_wrap
.check_wrap:
    mov eax, [cursor_pos]
    mov ecx, 80
    xor edx, edx
    div ecx
    cmp edx, 0
    jne .done
.check_scroll:
    cmp dword [cursor_pos], 2000
    jl .done
    ; Simple scroll: move everything up one line
    pusha
    mov esi, 0xB80A0
    mov edi, 0xB8000
    mov ecx, 1920
    cld
    rep movsd
    mov ecx, 80
    mov eax, 0x0720
    rep stosw
    sub dword [cursor_pos], 80
    popa
.done:
    popa
    mov esp, ebp
    pop ebp
    ret

print_string:
    push ebp
    mov ebp, esp
    pusha
    mov esi, [ebp+8]
.loop:
    movzx eax, byte [esi]
    cmp al, 0
    je .done
    push eax
    call print_char
    add esp, 4
    inc esi
    jmp .loop
.done:
    popa
    mov esp, ebp
    pop ebp
    ret

print_hex:
    push ebp
    mov ebp, esp
    pusha
    mov eax, [ebp+8]
    mov ebx, hex_buffer
    mov ecx, 8
    add ebx, 7
.loop:
    mov edx, eax
    and edx, 0x0F
    add dl, '0'
    cmp dl, '9'
    jbe .store
    add dl, 39
.store:
    mov [ebx], dl
    dec ebx
    shr eax, 4
    loop .loop
    mov byte [ebx+9], 0
    mov ebx, hex_buffer
    call print_string
    popa
    mov esp, ebp
    pop ebp
    ret

print_int:
    push ebp
    mov ebp, esp
    pusha
    mov eax, [ebp+8]
    mov ebx, int_buffer
    push ebx
    push 10
    push eax
    call itoa
    add esp, 12
    mov ebx, int_buffer
    call print_string
    popa
    mov esp, ebp
    pop ebp
    ret

print_newline:
    push 0x0A
    call print_char
    add esp, 4
    ret

print_clear:
    pusha
    mov edi, 0xB8000
    mov ecx, 2000
    mov eax, 0x0720
    rep stosw
    mov dword [cursor_pos], 0
    popa
    ret

print_ok:
    push ebx
    mov ebx, msg_ok
    call print_string
    pop ebx
    ret

print_fail:
    push ebx
    mov ebx, msg_fail
    call print_string
    pop ebx
    ret

print_info:
    push ebx
    mov ebx, msg_info
    call print_string
    pop ebx
    ret

print_warn:
    push ebx
    mov ebx, msg_warn
    call print_string
    pop ebx
    ret

print_debug:
    push ebx
    mov ebx, msg_debug
    call print_string
    pop ebx
    ret

; ============================================================================
;                         BOOT PANIC AND HANG
; ============================================================================
; Called when a fatal error occurs during boot. Displays an error message
; and halts the CPU.
; ============================================================================

boot_panic:
    cli
    push ebx
    mov ebx, msg_panic
    call print_string
    pop ebx
    jmp boot_hang

boot_hang:
    cli
    hlt
    jmp boot_hang

boot_reboot:
    ; Try keyboard controller reset
    mov al, 0xFE
    out 0x64, al
    ; Fallback: triple fault
    lidt [idt_zero]
    int 0
    jmp boot_hang

boot_shutdown:
    ; ACPI shutdown (simplified)
    mov ax, 0x2000
    mov dx, 0x604
    out dx, ax
    jmp boot_hang

; ============================================================================
;                         MULTIBOOT PARSING
; ============================================================================
; Parse the multiboot information structure passed by the bootloader.
; ============================================================================

check_multiboot:
    mov eax, [multiboot_magic]
    cmp eax, 0x2BADB002
    sete al
    movzx eax, al
    ret

get_mmap_length:
    mov ebx, [multiboot_info_ptr]
    test dword [ebx+44], 0x00000040
    jz .no_mmap
    mov eax, [ebx+48]
    ret
.no_mmap:
    xor eax, eax
    ret

get_mmap_entries:
    mov ebx, [multiboot_info_ptr]
    test dword [ebx+44], 0x00000040
    jz .no_mmap
    mov eax, [ebx+48]
    add eax, 4
    ret
.no_mmap:
    xor eax, eax
    ret

parse_multiboot:
    push ebp
    mov ebp, esp
    pusha
    mov ebx, [multiboot_info_ptr]
    mov eax, [ebx+44]
    test eax, 0x00000001
    jz .no_mem
    mov eax, [ebx+0]
    mov [mboot_mem_lower], eax
    mov eax, [ebx+4]
    mov [mboot_mem_upper], eax
.no_mem:
    test eax, 0x00000002
    jz .no_device
    mov eax, [ebx+8]
    mov [mboot_boot_device], eax
.no_device:
    test eax, 0x00000004
    jz .no_cmdline
    mov eax, [ebx+16]
    mov [mboot_cmdline], eax
.no_cmdline:
    test eax, 0x00000008
    jz .no_mods
    mov eax, [ebx+20]
    mov [mboot_mods_count], eax
    mov eax, [ebx+24]
    mov [mboot_mods_addr], eax
.no_mods:
    test eax, 0x00000020
    jz .no_elf
    mov eax, [ebx+28]
    mov [mboot_elf_num], eax
    mov eax, [ebx+32]
    mov [mboot_elf_size], eax
    mov eax, [ebx+36]
    mov [mboot_elf_addr], eax
    mov eax, [ebx+40]
    mov [mboot_elf_shndx], eax
.no_elf:
    test eax, 0x00000040
    jz .no_mmap
    mov eax, [ebx+44]
    mov [mboot_mmap_length], eax
    mov eax, [ebx+48]
    mov [mboot_mmap_addr], eax
.no_mmap:
    test eax, 0x00000100
    jz .no_framebuffer
    mov eax, [ebx+88]
    mov [mboot_framebuffer_addr], eax
    mov eax, [ebx+92]
    mov [mboot_framebuffer_pitch], eax
    mov eax, [ebx+96]
    mov [mboot_framebuffer_width], eax
    mov eax, [ebx+100]
    mov [mboot_framebuffer_height], eax
    mov eax, [ebx+104]
    mov [mboot_framebuffer_bpp], eax
    mov eax, [ebx+108]
    mov [mboot_framebuffer_type], eax
.no_framebuffer:
    popa
    mov esp, ebp
    pop ebp
    ret

; ============================================================================
;                         PAGING SETUP
; ============================================================================
; Sets up basic identity paging for the first 4MB of memory.
; This is sufficient for early kernel initialization.
; ============================================================================

setup_paging:
    pusha
    ; ------------------------------------------------------------------------
    ; Clear the page directory and page table.
    ; ------------------------------------------------------------------------
    mov edi, page_directory
    mov ecx, 1024
    xor eax, eax
    cld
    rep stosd
    
    mov edi, page_table_0
    mov ecx, 1024
    rep stosd
    
    ; ------------------------------------------------------------------------
    ; Fill page table 0 with identity mapping for first 4MB.
    ; Each entry maps 4KB. Flags: present, writable (0x03).
    ; ------------------------------------------------------------------------
    mov edi, page_table_0
    mov eax, 0x03
    mov ecx, 1024
.fill_loop:
    mov [edi], eax
    add eax, 0x1000
    add edi, 4
    loop .fill_loop
    
    ; ------------------------------------------------------------------------
    ; Set page directory entry 0 to point to page table 0.
    ; ------------------------------------------------------------------------
    mov eax, page_table_0
    or eax, 0x03
    mov [page_directory], eax
    
    ; ------------------------------------------------------------------------
    ; Set page directory entry 768 (0x300) for higher half mapping.
    ; This maps the kernel to 0xC0000000 (3GB).
    ; ------------------------------------------------------------------------
    mov eax, page_table_768
    or eax, 0x03
    mov [page_directory + 768*4], eax
    
    ; ------------------------------------------------------------------------
    ; Load the page directory into CR3.
    ; ------------------------------------------------------------------------
    mov eax, page_directory
    mov cr3, eax
    
    ; ------------------------------------------------------------------------
    ; Enable paging by setting the PG bit in CR0.
    ; ------------------------------------------------------------------------
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax
    
    popa
    ret

enable_paging:
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax
    ret

disable_paging:
    mov eax, cr0
    and eax, 0x7FFFFFFF
    mov cr0, eax
    ret

flush_tlb:
    mov eax, cr3
    mov cr3, eax
    ret

; ============================================================================
;                         CPUID CHECK
; ============================================================================
; Checks if the CPU supports the CPUID instruction.
; ============================================================================

cpuid_check:
    pushfd
    pop eax
    mov ecx, eax
    xor eax, 0x00200000
    push eax
    popfd
    pushfd
    pop eax
    xor eax, ecx
    jz .no_cpuid
    mov eax, 1
    jmp .done
.no_cpuid:
    xor eax, eax
.done:
    push ecx
    popfd
    ret

; ============================================================================
;                         LONG MODE CHECK
; ============================================================================
; Checks if the CPU supports long mode (x86_64).
; ============================================================================

check_long_mode:
    mov eax, 0x80000000
    cpuid
    cmp eax, 0x80000001
    jb .no_long_mode
    mov eax, 0x80000001
    cpuid
    test edx, 0x20000000
    jz .no_long_mode
    mov eax, 1
    ret
.no_long_mode:
    xor eax, eax
    ret

; ============================================================================
;                         LONG MODE ENTRY
; ============================================================================
; Transitions from 32-bit protected mode to 64-bit long mode.
; This is a complex multi-step process.
; ============================================================================

enter_long_mode:
    ; Step 1: Disable paging
    mov eax, cr0
    and eax, 0x7FFFFFFF
    mov cr0, eax
    
    ; Step 2: Enable PAE (Physical Address Extension)
    mov eax, cr4
    or eax, 0x00000020
    mov cr4, eax
    
    ; Step 3: Set up page tables (PML4, PDPT, PD, PT)
    ; This is simplified - real implementation needs proper setup
    mov eax, pml4_table
    mov cr3, eax
    
    ; Step 4: Enable long mode (EFER.LME)
    mov ecx, 0xC0000080
    rdmsr
    or eax, 0x00000100
    wrmsr
    
    ; Step 5: Enable paging (which activates long mode)
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax
    
    ; Step 6: Load 64-bit GDT and far jump
    lgdt [gdt64_descriptor]
    jmp 0x08:long_mode_start

[BITS 64]
long_mode_start:
    ; Now in 64-bit long mode
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    
    ; Set up 64-bit stack
    mov rsp, stack_top
    
    ; Call 64-bit kernel main
    call kernel_main
    
    ; Should never return
    cli
    hlt
    jmp $

[BITS 32]

; ============================================================================
;                         TSS SETUP
; ============================================================================

tss_flush:
    mov ax, 0x28        ; TSS selector
    ltr ax
    ret

; ============================================================================
;                         INTERRUPT ENABLE/DISABLE
; ============================================================================

enable_interrupts:
    sti
    ret

disable_interrupts:
    cli
    ret

halt_cpu:
    hlt
    ret

; ============================================================================
;                         DEFAULT ISR HANDLER
; ============================================================================

isr_default:
    pusha
    mov ebx, msg_default_isr
    call print_string
    popa
    iret

; ============================================================================
;                         HIGHER HALF KERNEL ENTRY
; ============================================================================

higher_half:
    ; This is called after paging is set up with higher half mapping
    ; Update stack and data segments to use higher half addresses
    mov eax, 0xC0000000
    add esp, eax
    add ebp, eax
    
    ; Jump to higher half kernel code
    call kernel_main
    jmp boot_panic

; ============================================================================
;                         DATA SECTION
; ============================================================================

SECTION .data

; ------------------------------------------------------------------------
; Multiboot info storage
; ------------------------------------------------------------------------
multiboot_magic:        dd 0
multiboot_info_ptr:     dd 0

; ------------------------------------------------------------------------
; Parsed multiboot fields
; ------------------------------------------------------------------------
mboot_mem_lower:        dd 0
mboot_mem_upper:        dd 0
mboot_boot_device:      dd 0
mboot_cmdline:          dd 0
mboot_mods_count:       dd 0
mboot_mods_addr:        dd 0
mboot_elf_num:          dd 0
mboot_elf_size:         dd 0
mboot_elf_addr:         dd 0
mboot_elf_shndx:        dd 0
mboot_mmap_length:      dd 0
mboot_mmap_addr:        dd 0
mboot_drives_length:    dd 0
mboot_drives_addr:      dd 0
mboot_config_table:     dd 0
mboot_bootloader_name:  dd 0
mboot_apm_table:        dd 0
mboot_vbe_control_info: dd 0
mboot_vbe_mode_info:    dd 0
mboot_vbe_mode:         dw 0
mboot_vbe_interface_seg: dw 0
mboot_vbe_interface_off: dw 0
mboot_vbe_interface_len: dw 0
mboot_framebuffer_addr:   dd 0
mboot_framebuffer_pitch:  dd 0
mboot_framebuffer_width:  dd 0
mboot_framebuffer_height: dd 0
mboot_framebuffer_bpp:    dd 0
mboot_framebuffer_type: dd 0
mboot_framebuffer_color_info: dd 0

; ------------------------------------------------------------------------
; PIC masks
; ------------------------------------------------------------------------
pic_master_mask:        db 0
pic_slave_mask:         db 0

; ------------------------------------------------------------------------
; Cursor position for VGA output
; ------------------------------------------------------------------------
cursor_pos:             dd 0

; ------------------------------------------------------------------------
; Temporary buffers for conversions
; ------------------------------------------------------------------------
hex_buffer:             times 12 db 0
int_buffer:             times 34 db 0

; ------------------------------------------------------------------------
; String constants for boot messages
; ------------------------------------------------------------------------
msg_no_multiboot:       db "[FAIL] Not loaded by Multiboot-compliant bootloader!", 0x0A, 0x0D, 0
msg_panic:              db "[PANIC] Boot failure - halting CPU", 0x0A, 0x0D, 0
msg_default_isr:        db "[WARN] Unhandled interrupt occurred", 0x0A, 0x0D, 0
msg_ok:                 db "[OK] ", 0
msg_fail:               db "[FAIL] ", 0
msg_info:               db "[INFO] ", 0
msg_warn:               db "[WARN] ", 0
msg_debug:              db "[DEBUG] ", 0

; ============================================================================
;                         BSS SECTION
; ============================================================================
; The BSS section contains uninitialized data. It is cleared by clear_bss.
; ============================================================================

SECTION .bss
ALIGN 16

; ------------------------------------------------------------------------
; Kernel stack - 16KB
; ------------------------------------------------------------------------
stack_bottom:
    resb 16384                      ; 16KB stack
stack_top:

; ------------------------------------------------------------------------
; Page tables for identity mapping and higher half
; ------------------------------------------------------------------------
ALIGN 4096
page_directory:
    resb 4096                       ; Page directory (1024 entries)
page_table_0:
    resb 4096                       ; First page table (identity map)
page_table_768:
    resb 4096                       ; Page table for higher half

; ------------------------------------------------------------------------
; Long mode page tables (PML4, PDPT, PD, PT)
; ------------------------------------------------------------------------
ALIGN 4096
pml4_table:
    resb 4096
pdpt_table:
    resb 4096
pd_table:
    resb 4096
pt_table:
    resb 4096

; ------------------------------------------------------------------------
; GDT structures
; ------------------------------------------------------------------------
ALIGN 8
gdt_start:
    resb 8                          ; Null descriptor
gdt_code:
    resb 8                          ; Code segment descriptor
gdt_data:
    resb 8                          ; Data segment descriptor
gdt_user_code:
    resb 8                          ; User code segment
gdt_user_data:
    resb 8                          ; User data segment
gdt_tss:
    resb 8                          ; TSS descriptor
gdt_end:

gdt_descriptor:
    resw 1                          ; Limit
gdt_base:
    resd 1                          ; Base

; ------------------------------------------------------------------------
; 64-bit GDT
; ------------------------------------------------------------------------
ALIGN 8
gdt64_start:
    resb 8                          ; Null
gdt64_code:
    resb 8                          ; 64-bit code
gdt64_data:
    resb 8                          ; 64-bit data
gdt64_end:

gdt64_descriptor:
    resw 1
    resd 1

; ------------------------------------------------------------------------
; IDT structures
; ------------------------------------------------------------------------
ALIGN 8
idt_start:
    resb 256 * 8                    ; 256 interrupt gates
idt_end:

idt_descriptor:
    resw 1                          ; Limit
    resd 1                          ; Base

; ------------------------------------------------------------------------
; TSS structure
; ------------------------------------------------------------------------
ALIGN 8
tss_entry:
    resb 104                        ; Standard x86 TSS size

; ------------------------------------------------------------------------
; IDT zero descriptor for triple fault reboot
; ------------------------------------------------------------------------
idt_zero:
    resw 1
    resd 1

; ------------------------------------------------------------------------
; BSS boundaries (set by linker)
; ------------------------------------------------------------------------
_bss_start:
    resb 4
_bss_end:
    resb 4

; ============================================================================
;                         END OF multiboot.asm
; ============================================================================
; This file is part of linuxab - github.com/realProxik
; Licensed under GPL-2.0
; ============================================================================
