// SPDX-License-Identifier: GPL-2.0

/* 
 * kernel vga Screen 
 * written by
 *         jatin kaushik    2026
 */
![no_std]
#![no_main]
#![feature(abi_x86_interrupt)]
#![feature(asm_const)]
#![feature(naked_functions)]
#![allow(dead_code)]
#![allow(unused_imports)]

/* kernel modules */
pub mod acpi;
pub mod alloc;
pub mod bits;
pub mod bitmap;
pub mod block;
pub mod auxiliary;
pub mod bug;
pub mod cpu;
pub mod clk;
pub mod configfs;
pub mod cpufreq;
pub mod cpumask;
pub mod cred;
pub mod device;
pub mod devres;
pub mod dma;
pub mod driver;
pub mod error;
pub mod faux;
pub mod firmware;
pub mod fmt;
pub mod fs;
pub mod gpu;
pub mod i2c;
pub mod impl_flags;
pub mod io;
pub mod init;
pub mod interop;
pub mod iov;
pub mod irq;
pub mod jump_label;
pub mod kunit;
pub mod lib;
pub mod list;
pub mod net;
pub mod num;
pub mod of;
pub mod opp;
pub mod page;
pub mod pci;
pub mod pid;
pub mod platform;
pub mod prelude;
pub mod processor;
pub mod print;
pub mod ptr;
pub mod pwm;
pub mod rbtree;
pub mod regulator;
pub mod safety;
pub mod sizes;
pub mod soc;
pub mod trace_point;
pub mod xarray;

// Subsystems
pub mod vga;
pub mod gdt;
pub mod idt;
pub mod interrupts;
pub mod memory;
pub mod heap;
pub mod terminal;
pub mod desktop;
pub mod bios;

use core::panic::PanicInfo;

/// Kernel entry point - called from assembly bootloader
#[no_mangle]
pub extern "C" fn kernel_main() -> ! {
    // Initialize VGA first for output
    vga::clear_screen();
    vga::set_color(vga::Color::Green, vga::Color::Black);
    
    print_banner();
    
    // Check if F12 was pressed - enter BIOS setup
    if bios::check_f12_boot() {
        vga::print("\\n[F12] Detected - Entering BIOS Setup...\\n");
        bios::enter_setup();
    }
    
    vga::print("\\n[*] Initializing MyKernel subsystems...\\n\\n");
    
    // Initialize all kernel subsystems
    init_subsystems();
    
    vga::print("\\n[+] All systems initialized!\\n");
    vga::print("[+] Starting terminal...\\n\\n");
    
    // Start interactive terminal with bash-like shell
    terminal::run_shell();
    
    // Should never reach here
    loop {
        core::arch::asm!("hlt");
    }
}

fn print_banner() {
    vga::set_color(vga::Color::Cyan, vga::Color::Black);
    vga::print(r#"
    __  __       _  __        _  __
   |  \/  |     | |/ /       | |/ /
   | \\  / |_   _| ' / ___ _  | ' / ___ _   _ ___
   | |\\/| | | | |  < / _ \\ | |  < / _ \\ | | / __|
   | |  | | |_| | . \\  __/ | | . \\  __/ |_| \\__ \\
   |_|  |_|\\__, |_|\\_\\___|_| |_|\\_\\___|\\__, |___/
             __/ |                        __/ |
            |___/                        |___/

"#);
    vga::set_color(vga::Color::White, vga::Color::Black);
    vga::print("    linuxab \\n");

fn init_subsystems() {
    // sys init
    cpu::init();
    gdt::init();
    idt::init();
    interrupts::init():
    memory::init();
    heap::init();
    acpi::init();
    pci::init();
    block::init();
    fs::init();
    net::init();
    gpu::init();
    deskktop::init();
    safety::init();
}

/// Panic handler
#[panic_handler]
fn panic(info: &PanicInfo) -> ! {
    vga::set_color(vga::Color::Red, vga::Color::Black);
    vga::print("\\n\\n kernel err ch%_$=bash\\n");
    
    if let Some(location) = info.location() {
        vga::print("Location: ");
        vga::print(location.file());
        vga::print(":");
        // TODO: Print line number
        vga::print("\\n");
    }
    
    if let Some(message) = info.message() {
        vga::print("Message: ");
        // TODO: Format message
        vga::print("\\n");
    }
    
    vga::print("\\nSystem halted.\\n");
    
    loop {
        core::arch::asm!("cli; hlt");
    }
}
"""

with open(f"{base}/src/kernel/main.rs", "w") as f:
    f.write(main_rs)

# ============================================================
# 8. VGA Driver (Enhanced)
# ============================================================
vga_rs = """//! VGA Text Mode Driver
//! Supports: Colors, scrolling, cursor, basic graphics

use core::ptr;

const VGA_BUFFER: *mut u8 = 0xb8000 as *mut u8;
const VGA_WIDTH: usize = 80;
const VGA_HEIGHT: usize = 25;

#[derive(Clone, Copy, Debug, PartialEq)]
#[repr(u8)]
pub enum Color {
    Black = 0,
    Blue = 1,
    Green = 2,
    Cyan = 3,
    Red = 4,
    Magenta = 5,
    Brown = 6,
    LightGray = 7,
    DarkGray = 8,
    LightBlue = 9,
    LightGreen = 10,
    LightCyan = 11,
    LightRed = 12,
    Pink = 13,
    Yellow = 14,
    White = 15,
}

static mut VGA_COL: usize = 0;
static mut VGA_ROW: usize = 0;
static mut VGA_FG: Color = Color::White;
static mut VGA_BG: Color = Color::Black;

pub fn clear_screen() {
    unsafe {
        VGA_COL = 0;
        VGA_ROW = 0;
        let color = make_color(VGA_FG, VGA_BG);
        for i in 0..(VGA_WIDTH * VGA_HEIGHT) {
            let offset = i * 2;
            ptr::write_volatile(VGA_BUFFER.add(offset), b' ');
            ptr::write_volatile(VGA_BUFFER.add(offset + 1), color);
        }
    }
}

pub fn set_color(fg: Color, bg: Color) {
    unsafe {
        VGA_FG = fg;
        VGA_BG = bg;
    }
}

fn make_color(fg: Color, bg: Color) -> u8 {
    (bg as u8) << 4 | (fg as u8)
}

pub fn print(s: &str) {
    for byte in s.bytes() {
        putchar(byte);
    }
}

pub fn putchar(c: u8) {
    unsafe {
        match c {
            b'\\n' => newline(),
            b'\\r' => VGA_COL = 0,
            b'\\t' => {
                let spaces = 8 - (VGA_COL % 8);
                for _ in 0..spaces {
                    putchar(b' ');
                }
            }
            0x08 => { // Backspace
                if VGA_COL > 0 {
                    VGA_COL -= 1;
                    let offset = (VGA_ROW * VGA_WIDTH + VGA_COL) * 2;
                    ptr::write_volatile(VGA_BUFFER.add(offset), b' ');
                    ptr::write_volatile(VGA_BUFFER.add(offset + 1), make_color(VGA_FG, VGA_BG));
                }
            }
            _ => {
                if c >= 0x20 && c <= 0x7E {
                    let offset = (VGA_ROW * VGA_WIDTH + VGA_COL) * 2;
                    ptr::write_volatile(VGA_BUFFER.add(offset), c);
                    ptr::write_volatile(VGA_BUFFER.add(offset + 1), make_color(VGA_FG, VGA_BG));
                    VGA_COL += 1;
                    if VGA_COL >= VGA_WIDTH {
                        newline();
                    }
                }
            }
        }
    }
}

fn newline() {
    unsafe {
        VGA_COL = 0;
        VGA_ROW += 1;
        if VGA_ROW >= VGA_HEIGHT {
            scroll();
            VGA_ROW = VGA_HEIGHT - 1;
        }
    }
}

fn scroll() {
    unsafe {
        for row in 1..VGA_HEIGHT {
            for col in 0..VGA_WIDTH {
                let src = ((row * VGA_WIDTH + col) * 2) as isize;
                let dst = (((row - 1) * VGA_WIDTH + col) * 2) as isize;
                let char_byte = ptr::read_volatile(VGA_BUFFER.offset(src));
                let attr_byte = ptr::read_volatile(VGA_BUFFER.offset(src + 1));
                ptr::write_volatile(VGA_BUFFER.offset(dst), char_byte);
                ptr::write_volatile(VGA_BUFFER.offset(dst + 1), attr_byte);
            }
        }
        // Clear last line
        let color = make_color(VGA_FG, VGA_BG);
        for col in 0..VGA_WIDTH {
            let offset = ((VGA_HEIGHT - 1) * VGA_WIDTH + col) * 2;
            ptr::write_volatile(VGA_BUFFER.add(offset), b' ');
            ptr::write_volatile(VGA_BUFFER.add(offset + 1), color);
        }
    }
}

pub fn move_cursor(row: usize, col: usize) {
    unsafe {
        VGA_ROW = row.min(VGA_HEIGHT - 1);
        VGA_COL = col.min(VGA_WIDTH - 1);
    }
    // TODO: Update hardware cursor via VGA ports 0x3D4/0x3D5
}

pub fn get_cursor() -> (usize, usize) {
    unsafe { (VGA_ROW, VGA_COL) }
}

pub fn draw_box(x: usize, y: usize, w: usize, h: usize, color: Color) {
    // Simple box drawing for desktop
    set_color(color, VGA_BG);
    for row in y..(y + h).min(VGA_HEIGHT) {
        for col in x..(x + w).min(VGA_WIDTH) {
            let offset = (row * VGA_WIDTH + col) * 2;
            let c = if row == y || row == y + h - 1 {
                if col == x || col == x + w - 1 { b'+' }
                else { b'-' }
            } else if col == x || col == x + w - 1 {
                b'|'
            } else {
                b' '
            };
            unsafe {
                ptr::write_volatile(VGA_BUFFER.add(offset), c);
                ptr::write_volatile(VGA_BUFFER.add(offset + 1), make_color(color, VGA_BG));
            }
        }
    }
}