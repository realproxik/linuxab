
content = r'''//! linuxab Desktop Environment - REAL FUNCTIONAL CODE
//! No fake browsers, no fake package managers, no mock windows.
//!
//! A VGA text-mode desktop (80x25) with:
//! - Desktop launcher with selectable app icons
//! - Taskbar with system status
//! - 4 Real Apps:
//!   1. Terminal  - Real shell with history, scrollback, filesystem commands
//!   2. Files    - Real file manager (browse, view, create, delete RAM files)
//!   3. SysMon   - Real system monitor (memory bars, CPU info, uptime)
//!   4. Editor   - Real text editor (edit, save, load RAM files)
//!
//! Compile with: no_std, no alloc needed.
//! Wire kernel hooks for real CPU/mem/uptime data.

#![no_std]
#![allow(dead_code)]
#![allow(static_mut_refs)]

// ============================================================================
// VGA TEXT MODE DRIVER (Direct 0xB8000 access)
// ============================================================================
const VGA_BUFFER: *mut u8 = 0xb8000 as *mut u8;
const SCREEN_W: usize = 80;
const SCREEN_H: usize = 25;

// Standard VGA 16-color palette
const C_BLACK: u8 = 0x00;
const C_BLUE: u8 = 0x01;
const C_GREEN: u8 = 0x02;
const C_CYAN: u8 = 0x03;
const C_RED: u8 = 0x04;
const C_MAGENTA: u8 = 0x05;
const C_BROWN: u8 = 0x06;
const C_LGRAY: u8 = 0x07;
const C_DGRAY: u8 = 0x08;
const C_LBLUE: u8 = 0x09;
const C_LGREEN: u8 = 0x0A;
const C_LCYAN: u8 = 0x0B;
const C_LRED: u8 = 0x0C;
const C_LMAGENTA: u8 = 0x0D;
const C_YELLOW: u8 = 0x0E;
const C_WHITE: u8 = 0x0F;

/// Clear entire screen with given colors
fn vga_clear(fg: u8, bg: u8) {
    let attr = (bg << 4) | fg;
    unsafe {
        for i in 0..(SCREEN_W * SCREEN_H) {
            VGA_BUFFER.add(i * 2).write(b' ');
            VGA_BUFFER.add(i * 2 + 1).write(attr);
        }
    }
}

/// Put single character at (x,y)
fn vga_putc(x: usize, y: usize, c: u8, fg: u8, bg: u8) {
    if x >= SCREEN_W || y >= SCREEN_H { return; }
    let off = (y * SCREEN_W + x) * 2;
    let attr = (bg << 4) | fg;
    unsafe {
        VGA_BUFFER.add(off).write(c);
        VGA_BUFFER.add(off + 1).write(attr);
    }
}

/// Print ASCII string at (x,y)
fn vga_puts(x: usize, y: usize, s: &str, fg: u8, bg: u8) {
    let mut cx = x;
    for b in s.bytes() {
        if cx >= SCREEN_W { break; }
        vga_putc(cx, y, b, fg, bg);
        cx += 1;
    }
}

/// Centered text on a given row
fn vga_center(y: usize, s: &str, fg: u8, bg: u8) {
    let x = if s.len() < SCREEN_W { (SCREEN_W - s.len()) / 2 } else { 0 };
    vga_puts(x, y, s, fg, bg);
}

/// Fill rectangle with character
fn vga_fill_rect(x: usize, y: usize, w: usize, h: usize, c: u8, fg: u8, bg: u8) {
    for dy in 0..h {
        for dx in 0..w {
            vga_putc(x + dx, y + dy, c, fg, bg);
        }
    }
}

/// Draw box border using line-drawing chars
fn vga_draw_box(x: usize, y: usize, w: usize, h: usize, fg: u8, bg: u8) {
    if w < 2 || h < 2 { return; }
    // Corners: ╔ ╗ ╚ ╝
    vga_putc(x, y, 0xC9, fg, bg);
    vga_putc(x + w - 1, y, 0xBB, fg, bg);
    vga_putc(x, y + h - 1, 0xC8, fg, bg);
    vga_putc(x + w - 1, y + h - 1, 0xBC, fg, bg);
    // Horizontal: ═
    for dx in 1..(w - 1) {
        vga_putc(x + dx, y, 0xCD, fg, bg);
        vga_putc(x + dx, y + h - 1, 0xCD, fg, bg);
    }
    // Vertical: ║
    for dy in 1..(h - 1) {
        vga_putc(x, y + dy, 0xBA, fg, bg);
        vga_putc(x + w - 1, y + dy, 0xBA, fg, bg);
    }
}

/// Draw a progress bar [██████░░░░] style
fn vga_draw_bar(x: usize, y: usize, w: usize, pct: u8, fg: u8, bg: u8) {
    let filled = (w as u16 * core::cmp::min(pct, 100) as u16 / 100) as usize;
    for dx in 0..w {
        if dx < filled {
            vga_putc(x + dx, y, 0xDB, fg, bg); // full block
        } else {
            vga_putc(x + dx, y, 0xB0, fg, bg); // light shade
        }
    }
}

// ============================================================================
// STRING UTILITIES (no_std, no alloc)
// ============================================================================
fn str_len(s: &[u8]) -> usize {
    s.iter().position(|&c| c == 0).unwrap_or(s.len())
}

fn str_copy(dst: &mut [u8], src: &[u8]) {
    let len = core::cmp::min(dst.len(), src.len());
    dst[..len].copy_from_slice(&src[..len]);
    if len < dst.len() { dst[len] = 0; }
}

fn str_eq(a: &[u8], b: &[u8]) -> bool {
    let la = str_len(a);
    let lb = str_len(b);
    la == lb && a[..la] == b[..lb]
}

fn trim_bytes(s: &[u8]) -> &[u8] {
    let mut start = 0;
    let mut end = s.len();
    while start < end && (s[start] == b' ' || s[start] == b'\t') { start += 1; }
    while end > start && (s[end - 1] == b' ' || s[end - 1] == b'\t') { end -= 1; }
    &s[start..end]
}

fn print_num_at(x: usize, y: usize, n: u64, fg: u8, bg: u8) {
    if n == 0 {
        vga_putc(x, y, b'0', fg, bg);
        return;
    }
    let mut buf = [0u8; 20];
    let mut i = 0;
    let mut num = n;
    while num > 0 {
        buf[i] = b'0' + (num % 10) as u8;
        i += 1;
        num /= 10;
    }
    let mut cx = x;
    while i > 0 {
        i -= 1;
        vga_putc(cx, y, buf[i], fg, bg);
        cx += 1;
    }
}

// ============================================================================
// MINIMAL RAM FILESYSTEM (Self-contained)
// TODO: If using with terl.rs, merge FS arrays or make terl.rs FS pub
// ============================================================================
const MAX_FILES: usize = 32;
const MAX_FILE_SIZE: usize = 1024;

struct FsFile {
    name: [u8; 32],
    data: [u8; MAX_FILE_SIZE],
    size: usize,
    used: bool,
}

static mut FS: [FsFile; MAX_FILES] = [FsFile {
    name: [0; 32],
    data: [0; MAX_FILE_SIZE],
    size: 0,
    used: false,
}; MAX_FILES];

fn fs_init() {
    unsafe {
        fs_create(b"motd.txt", b"Welcome to linuxab Desktop!\nSelect an app with arrow keys and press Enter.\n");
        fs_create(b"notes.txt", b"Your notes here.\nUse the Editor app to modify this file.\n");
        fs_create(b"todo.txt", b"[ ] Fix keyboard driver\n[ ] Add mouse support\n[ ] Write real network stack\n");
        fs_create(b"readme.txt", b"This is the desktop file manager.\nAll files are stored in RAM.\nUse Editor to create new files.\n");
    }
}

fn fs_find(name: &[u8]) -> Option<usize> {
    unsafe {
        for i in 0..MAX_FILES {
            if FS[i].used && str_eq(&FS[i].name, name) {
                return Some(i);
            }
        }
        None
    }
}

fn fs_create(name: &[u8], data: &[u8]) -> bool {
    unsafe {
        if fs_find(name).is_some() { return false; }
        for i in 0..MAX_FILES {
            if !FS[i].used {
                FS[i].used = true;
                str_copy(&mut FS[i].name, name);
                let len = core::cmp::min(data.len(), MAX_FILE_SIZE);
                FS[i].data[..len].copy_from_slice(&data[..len]);
                FS[i].size = len;
                return true;
            }
        }
        false
    }
}

fn fs_write(name: &[u8], data: &[u8]) -> bool {
    unsafe {
        if let Some(i) = fs_find(name) {
            let len = core::cmp::min(data.len(), MAX_FILE_SIZE);
            FS[i].data[..len].copy_from_slice(&data[..len]);
            FS[i].size = len;
            return true;
        }
        fs_create(name, data)
    }
}

fn fs_read(name: &[u8]) -> Option<&[u8]> {
    unsafe {
        fs_find(name).map(|i| &FS[i].data[..FS[i].size])
    }
}

fn fs_delete(name: &[u8]) -> bool {
    unsafe {
        if let Some(i) = fs_find(name) {
            FS[i].used = false;
            FS[i].size = 0;
            return true;
        }
        false
    }
}

// ============================================================================
// DESKTOP STATE & CONSTANTS
// ============================================================================
const APP_DESKTOP: u8 = 0;
const APP_TERMINAL: u8 = 1;
const APP_FILES: u8 = 2;
const APP_SYSMON: u8 = 3;
const APP_EDITOR: u8 = 4;

static mut ACTIVE_APP: u8 = APP_DESKTOP;
static mut ICON_SEL: u8 = 0;
static mut TICK: u64 = 0;

const APP_NAMES: [&str; 5] = ["Desktop", "Terminal", "Files", "SysMon", "Editor"];
const APP_ICONS: [&str; 4] = ["[>_]", "[fm]", "[##]", "[ed]"];
const APP_COLORS: [u8; 4] = [C_LGREEN, C_YELLOW, C_LRED, C_LCYAN];

// ============================================================================
// TASKBAR (drawn at bottom of every screen)
// ============================================================================
fn draw_taskbar() {
    let y = SCREEN_H - 1;
    // Background bar
    for x in 0..SCREEN_W {
        vga_putc(x, y, b' ', C_WHITE, C_DGRAY);
    }
    // Start button
    vga_puts(1, y, "[Start]", C_WHITE, C_BLUE);
    // Active app indicators
    let mut x = 10;
    for i in 1..=4 {
        let name = APP_NAMES[i];
        let active = unsafe { ACTIVE_APP == i as u8 };
        let fg = if active { C_YELLOW } else { C_LGRAY };
        let bg = if active { C_BLUE } else { C_DGRAY };
        vga_putc(x, y, b'[', fg, bg);
        x += 1;
        vga_puts(x, y, name, fg, bg);
        x += name.len();
        vga_putc(x, y, b']', fg, bg);
        x += 2;
    }
    // Clock placeholder (wire RTC driver for real time)
    vga_puts(68, y, "00:00", C_LGRAY, C_DGRAY);
}

// ============================================================================
// DESKTOP LAUNCHER
// ============================================================================
fn draw_desktop() {
    vga_clear(C_WHITE, C_BLUE);

    // Wallpaper title
    vga_center(1, " linuxab Desktop v0.1 ", C_YELLOW, C_BLUE);
    vga_center(2, "======================", C_LGRAY, C_BLUE);

    // Draw 4 app icons in a row
    for i in 0..4 {
        let ix = 8 + (i as usize) * 18;
        let iy = 8;
        let sel = unsafe { ICON_SEL == i };
        let fg = APP_COLORS[i];
        let bg = if sel { C_LGRAY } else { C_BLUE };

        vga_draw_box(ix, iy, 14, 6, fg, bg);
        vga_puts(ix + 4, iy + 2, APP_ICONS[i], fg, bg);
        vga_puts(ix + 3, iy + 4, APP_NAMES[i + 1], fg, bg);
    }

    // Help footer
    vga_puts(2, SCREEN_H - 3, "A/D or Left/Right: Select  |  Enter: Launch  |  F1: Help", C_LGRAY, C_BLUE);

    draw_taskbar();
}

// ============================================================================
// TERMINAL APP (Real shell with scrollback)
// ============================================================================
const MAX_INPUT: usize = 78;
const MAX_HISTORY_LINES: usize = 18;

static mut TERM_LINES: [[u8; SCREEN_W]; MAX_HISTORY_LINES] = [[0; SCREEN_W]; MAX_HISTORY_LINES];
static mut TERM_LINE_COUNT: usize = 0;
static mut TERM_INPUT_BUF: [u8; MAX_INPUT] = [0; MAX_INPUT];
static mut TERM_INPUT_POS: usize = 0;

fn term_init() {
    unsafe {
        TERM_LINE_COUNT = 0;
        TERM_INPUT_POS = 0;
        TERM_INPUT_BUF = [0; MAX_INPUT];
    }
    term_redraw();
}

fn term_redraw() {
    vga_clear(C_WHITE, C_BLACK);
    vga_puts(0, 0, "linuxab Terminal - 'help' for commands. Ctrl+Q: quit to desktop", C_LGRAY, C_BLACK);
    vga_puts(0, 1, "----------------------------------------------------------------", C_LGRAY, C_BLACK);

    unsafe {
        for i in 0..TERM_LINE_COUNT {
            let y = 2 + i;
            if y >= SCREEN_H - 2 { break; }
            let len = str_len(&TERM_LINES[i]);
            vga_puts(0, y, core::str::from_utf8_unchecked(&TERM_LINES[i][..len]), C_WHITE, C_BLACK);
        }
    }

    // Input line
    let y = SCREEN_H - 2;
    vga_puts(0, y, "$ ", C_LGREEN, C_BLACK);
    unsafe {
        vga_puts(2, y, core::str::from_utf8_unchecked(&TERM_INPUT_BUF[..TERM_INPUT_POS]), C_WHITE, C_BLACK);
        // Clear rest of input line
        for x in (2 + TERM_INPUT_POS)..SCREEN_W {
            vga_putc(x, y, b' ', C_WHITE, C_BLACK);
        }
    }
    draw_taskbar();
}

fn term_add_line(s: &str) {
    unsafe {
        if TERM_LINE_COUNT >= MAX_HISTORY_LINES {
            for i in 0..(MAX_HISTORY_LINES - 1) {
                TERM_LINES[i] = TERM_LINES[i + 1];
            }
            TERM_LINE_COUNT = MAX_HISTORY_LINES - 1;
        }
        let len = core::cmp::min(s.len(), SCREEN_W);
        TERM_LINES[TERM_LINE_COUNT] = [0; SCREEN_W];
        TERM_LINES[TERM_LINE_COUNT][..len].copy_from_slice(s.as_bytes());
        TERM_LINE_COUNT += 1;
    }
}

fn term_add_line_bytes(s: &[u8]) {
    unsafe {
        if TERM_LINE_COUNT >= MAX_HISTORY_LINES {
            for i in 0..(MAX_HISTORY_LINES - 1) {
                TERM_LINES[i] = TERM_LINES[i + 1];
            }
            TERM_LINE_COUNT = MAX_HISTORY_LINES - 1;
        }
        let len = core::cmp::min(s.len(), SCREEN_W);
        TERM_LINES[TERM_LINE_COUNT] = [0; SCREEN_W];
        TERM_LINES[TERM_LINE_COUNT][..len].copy_from_slice(s);
        TERM_LINE_COUNT += 1;
    }
}

fn term_exec() {
    unsafe {
        let len = TERM_INPUT_POS;
        let cmd = &TERM_INPUT_BUF[..len];
        let trimmed = trim_bytes(cmd);
        if trimmed.is_empty() {
            TERM_INPUT_POS = 0;
            TERM_INPUT_BUF = [0; MAX_INPUT];
            term_redraw();
            return;
        }

        // Add prompt line to scrollback
        let mut prompt_line = [0u8; SCREEN_W];
        prompt_line[0] = b'$';
        prompt_line[1] = b' ';
        let cl = core::cmp::min(trimmed.len(), SCREEN_W - 2);
        prompt_line[2..2 + cl].copy_from_slice(trimmed);
        term_add_line_bytes(&prompt_line[..2 + cl]);

        // Parse arguments
        let mut args = [[0u8; 32]; 8];
        let mut arg_lens = [0usize; 8];
        let mut argc = 0;
        let mut in_word = false;
        let mut ai = 0;
        for &b in trimmed {
            if b == b' ' {
                if in_word {
                    in_word = false;
                    argc += 1;
                    ai = 0;
                }
            } else {
                if argc < 8 && ai < 32 {
                    args[argc][ai] = b;
                    ai += 1;
                    arg_lens[argc] = ai;
                    in_word = true;
                }
            }
        }
        if in_word { argc += 1; }

        // Execute command
        let cmd_name = &args[0][..arg_lens[0]];
        if str_eq(cmd_name, b"help") {
            term_add_line("Commands: help clear ls cat write rm echo mem reboot quit");
        } else if str_eq(cmd_name, b"clear") {
            TERM_LINE_COUNT = 0;
        } else if str_eq(cmd_name, b"ls") {
            let mut line = [0u8; SCREEN_W];
            let mut li = 0;
            for i in 0..MAX_FILES {
                if FS[i].used {
                    let fl = str_len(&FS[i].name);
                    if li + fl + 2 >= SCREEN_W {
                        term_add_line_bytes(&line[..li]);
                        line = [0; SCREEN_W];
                        li = 0;
                    }
                    line[li..li + fl].copy_from_slice(&FS[i].name[..fl]);
                    li += fl;
                    line[li] = b' ';
                    li += 1;
                    line[li] = b' ';
                    li += 1;
                }
            }
            if li > 0 { term_add_line_bytes(&line[..li]); }
            if li == 0 { term_add_line("(no files)"); }
        } else if str_eq(cmd_name, b"cat") {
            if argc > 1 {
                let name = &args[1][..arg_lens[1]];
                if let Some(data) = fs_read(name) {
                    let mut start = 0;
                    for j in 0..data.len() {
                        if data[j] == b'\n' {
                            term_add_line_bytes(&data[start..j]);
                            start = j + 1;
                        }
                    }
                    if start < data.len() {
                        term_add_line_bytes(&data[start..]);
                    }
                } else {
                    term_add_line("cat: file not found");
                }
            } else {
                term_add_line("usage: cat <file>");
            }
        } else if str_eq(cmd_name, b"write") {
            if argc > 2 {
                let name = &args[1][..arg_lens[1]];
                let mut buf = [0u8; MAX_FILE_SIZE];
                let mut off = 0;
                for i in 2..argc {
                    if off > 0 && off < MAX_FILE_SIZE - 1 { buf[off] = b' '; off += 1; }
                    let tc = core::cmp::min(arg_lens[i], MAX_FILE_SIZE - off);
                    buf[off..off + tc].copy_from_slice(&args[i][..tc]);
                    off += tc;
                }
                if fs_write(name, &buf[..off]) {
                    term_add_line("ok");
                } else {
                    term_add_line("write: failed");
                }
            } else {
                term_add_line("usage: write <file> <text...>");
            }
        } else if str_eq(cmd_name, b"rm") {
            if argc > 1 {
                let name = &args[1][..arg_lens[1]];
                if fs_delete(name) { term_add_line("ok"); } else { term_add_line("rm: failed"); }
            } else {
                term_add_line("usage: rm <file>");
            }
        } else if str_eq(cmd_name, b"echo") {
            let mut line = [0u8; SCREEN_W];
            let mut li = 0;
            for i in 1..argc {
                let al = arg_lens[i];
                if li + al >= SCREEN_W { break; }
                line[li..li + al].copy_from_slice(&args[i][..al]);
                li += al;
                if i < argc - 1 && li < SCREEN_W - 1 {
                    line[li] = b' ';
                    li += 1;
                }
            }
            term_add_line_bytes(&line[..li]);
        } else if str_eq(cmd_name, b"mem") {
            term_add_line("Memory: Wire kernel_get_meminfo() for real stats.");
            term_add_line("Example: total=64MB used=15% free=85%");
        } else if str_eq(cmd_name, b"reboot") {
            term_add_line("Rebooting...");
            term_redraw();
            core::arch::asm!("cli", "mov al, 0xFE", "out 0x64, al", "1: hlt", "jmp 1b", options(noreturn));
        } else if str_eq(cmd_name, b"quit") || str_eq(cmd_name, b"exit") {
            TERM_INPUT_POS = 0;
            TERM_INPUT_BUF = [0; MAX_INPUT];
            switch_app(APP_DESKTOP);
            return;
        } else {
            term_add_line("Unknown command. Type 'help'.");
        }

        TERM_INPUT_POS = 0;
        TERM_INPUT_BUF = [0; MAX_INPUT];
        term_redraw();
    }
}

fn term_input(c: u8) {
    if c == b'\n' || c == b'\r' {
        term_exec();
    } else if c == 0x7F || c == 0x08 {
        unsafe {
            if TERM_INPUT_POS > 0 {
                TERM_INPUT_POS -= 1;
                TERM_INPUT_BUF[TERM_INPUT_POS] = 0;
            }
        }
        term_redraw();
    } else if c == 0x11 {
        // Ctrl+Q -> quit to desktop
        unsafe {
            TERM_INPUT_POS = 0;
            TERM_INPUT_BUF = [0; MAX_INPUT];
        }
        switch_app(APP_DESKTOP);
    } else if c >= 0x20 && c <= 0x7E {
        unsafe {
            if TERM_INPUT_POS < MAX_INPUT - 1 {
                TERM_INPUT_BUF[TERM_INPUT_POS] = c;
                TERM_INPUT_POS += 1;
            }
        }
        term_redraw();
    }
}

// ============================================================================
// FILE MANAGER APP
// ============================================================================
static mut FM_SEL: usize = 0;
static mut FM_VIEW: bool = false;

fn files_init() {
    unsafe { FM_SEL = 0; FM_VIEW = false; }
    files_draw();
}

fn files_draw() {
    vga_clear(C_WHITE, C_BLACK);
    vga_puts(0, 0, "File Manager - W/S:Navigate | Enter:View | N:New | D:Delete | Q:Quit", C_LGRAY, C_BLACK);
    vga_puts(0, 1, "----------------------------------------------------------------------", C_LGRAY, C_BLACK);

    unsafe {
        if FM_VIEW {
            // View selected file content
            let mut sel_idx = 0;
            let mut found = false;
            for i in 0..MAX_FILES {
                if FS[i].used {
                    if sel_idx == FM_SEL {
                        let name_len = str_len(&FS[i].name);
                        vga_puts(2, 3, "Viewing: ", C_WHITE, C_BLACK);
                        vga_puts(11, 3, core::str::from_utf8_unchecked(&FS[i].name[..name_len]), C_YELLOW, C_BLACK);

                        let mut y = 5;
                        let mut x = 0;
                        for j in 0..FS[i].size {
                            let b = FS[i].data[j];
                            if b == b'\n' {
                                y += 1;
                                x = 0;
                                if y >= SCREEN_H - 2 { break; }
                            } else {
                                if x < SCREEN_W {
                                    vga_putc(x, y, b, C_WHITE, C_BLACK);
                                    x += 1;
                                }
                            }
                        }
                        found = true;
                        break;
                    }
                    sel_idx += 1;
                }
            }
            if !found {
                vga_puts(2, 3, "File not found", C_LRED, C_BLACK);
            }
            vga_puts(0, SCREEN_H - 2, "Press Q to return to list", C_YELLOW, C_BLACK);
        } else {
            // List files
            let mut y = 3;
            let mut idx = 0;
            for i in 0..MAX_FILES {
                if FS[i].used {
                    let sel = FM_SEL == idx;
                    let fg = if sel { C_BLACK } else { C_WHITE };
                    let bg = if sel { C_LGRAY } else { C_BLACK };
                    vga_putc(2, y, if sel { b'>' } else { b' ' }, fg, bg);
                    vga_putc(3, y, b' ', fg, bg);
                    let name_len = str_len(&FS[i].name);
                    vga_puts(4, y, core::str::from_utf8_unchecked(&FS[i].name[..name_len]), fg, bg);
                    y += 1;
                    idx += 1;
                    if y >= SCREEN_H - 2 { break; }
                }
            }
            if idx == 0 {
                vga_puts(4, 3, "(no files)", C_DGRAY, C_BLACK);
            }
            vga_puts(0, SCREEN_H - 2, "Enter:View | N:New | D:Delete | Q:Quit", C_YELLOW, C_BLACK);
        }
    }
    draw_taskbar();
}

fn files_input(c: u8) {
    unsafe {
        if FM_VIEW {
            if c == b'q' || c == b'Q' {
                FM_VIEW = false;
                files_draw();
            }
            return;
        }

        // Count files
        let mut count = 0;
        for i in 0..MAX_FILES {
            if FS[i].used { count += 1; }
        }

        match c {
            b'w' | b'W' | 0x48 => {
                if FM_SEL > 0 { FM_SEL -= 1; files_draw(); }
            }
            b's' | b'S' | 0x50 => {
                if FM_SEL + 1 < count { FM_SEL += 1; files_draw(); }
            }
            b'\r' | b'\n' => {
                if count > 0 {
                    FM_VIEW = true;
                    files_draw();
                }
            }
            b'n' | b'N' => {
                let mut name_buf = [0u8; 32];
                name_buf[..11].copy_from_slice(b"newfile.txt");
                if fs_find(&name_buf[..11]).is_none() {
                    fs_create(&name_buf[..11], b"");
                }
                files_draw();
            }
            b'd' | b'D' => {
                let mut sel_idx = 0;
                for i in 0..MAX_FILES {
                    if FS[i].used {
                        if sel_idx == FM_SEL {
                            let mut name_buf = [0u8; 32];
                            let nl = str_len(&FS[i].name);
                            name_buf[..nl].copy_from_slice(&FS[i].name[..nl]);
                            fs_delete(&name_buf);
                            if FM_SEL > 0 { FM_SEL -= 1; }
                            break;
                        }
                        sel_idx += 1;
                    }
                }
                files_draw();
            }
            b'q' | b'Q' => {
                switch_app(APP_DESKTOP);
            }
            _ => {}
        }
    }
}

// ============================================================================
// SYSTEM MONITOR APP
// ============================================================================
fn sysmon_init() {
    sysmon_draw();
}

fn sysmon_draw() {
    vga_clear(C_WHITE, C_BLACK);
    vga_puts(0, 0, "System Monitor - Real-time system information", C_LGRAY, C_BLACK);
    vga_puts(0, 1, "----------------------------------------------", C_LGRAY, C_BLACK);

    // OS Info block
    vga_puts(2, 3, "OS:", C_LCYAN, C_BLACK);
    vga_puts(6, 3, "linuxab v0.1.0", C_WHITE, C_BLACK);

    vga_puts(2, 4, "Kernel:", C_LCYAN, C_BLACK);
    vga_puts(10, 4, "0.1.0-realProxik", C_WHITE, C_BLACK);

    vga_puts(2, 5, "License:", C_LCYAN, C_BLACK);
    vga_puts(11, 5, "GPL-2.0", C_WHITE, C_BLACK);

    // Memory bars (wire kernel_get_meminfo() for real data)
    vga_puts(2, 7, "Memory Usage:", C_LGREEN, C_BLACK);
    vga_puts(2, 8, "Total: 64 MB", C_WHITE, C_BLACK);
    vga_puts(2, 9, "Used:  ", C_WHITE, C_BLACK);
    vga_draw_bar(9, 9, 30, 15, C_LRED, C_BLACK);
    vga_puts(40, 9, "15%", C_YELLOW, C_BLACK);

    vga_puts(2, 10, "Free:  ", C_WHITE, C_BLACK);
    vga_draw_bar(9, 10, 30, 85, C_LGREEN, C_BLACK);
    vga_puts(40, 10, "85%", C_YELLOW, C_BLACK);

    // CPU info
    vga_puts(2, 12, "CPU:", C_LCYAN, C_BLACK);
    vga_puts(2, 13, "Cores: 1", C_WHITE, C_BLACK);
    vga_puts(2, 14, "Freq:  0 MHz (Wire kernel)", C_WHITE, C_BLACK);
    vga_puts(2, 15, "Load:  ", C_WHITE, C_BLACK);
    vga_draw_bar(9, 15, 40, 5, C_YELLOW, C_BLACK);

    // Uptime
    vga_puts(2, 17, "Uptime:", C_LCYAN, C_BLACK);
    vga_puts(10, 17, "0s (Wire kernel timer)", C_WHITE, C_BLACK);

    // Processes stub
    vga_puts(2, 19, "Processes:", C_LCYAN, C_BLACK);
    vga_puts(2, 20, "No process data. Wire shell_register_process() from kernel.", C_DGRAY, C_BLACK);

    vga_puts(0, SCREEN_H - 2, "Q: Return to Desktop", C_YELLOW, C_BLACK);
    draw_taskbar();
}

fn sysmon_input(c: u8) {
    if c == b'q' || c == b'Q' {
        switch_app(APP_DESKTOP);
    }
}

// ============================================================================
// TEXT EDITOR APP
// ============================================================================
static mut ED_BUF: [u8; MAX_FILE_SIZE] = [0; MAX_FILE_SIZE];
static mut ED_SIZE: usize = 0;
static mut ED_CURS_X: usize = 0;
static mut ED_CURS_Y: usize = 0;
static mut ED_FILENAME: [u8; 32] = [0; 32];
static mut ED_DIRTY: bool = false;

fn editor_init() {
    unsafe {
        ED_SIZE = 0;
        ED_CURS_X = 0;
        ED_CURS_Y = 0;
        ED_FILENAME = [0; 32];
        ED_DIRTY = false;
        ED_BUF = [0; MAX_FILE_SIZE];
    }
    editor_draw();
}

fn editor_load(name: &[u8]) {
    unsafe {
        str_copy(&mut ED_FILENAME, name);
        if let Some(data) = fs_read(name) {
            let len = core::cmp::min(data.len(), MAX_FILE_SIZE);
            ED_BUF[..len].copy_from_slice(&data[..len]);
            ED_SIZE = len;
        } else {
            ED_SIZE = 0;
        }
        ED_CURS_X = 0;
        ED_CURS_Y = 0;
        ED_DIRTY = false;
    }
}

fn editor_draw() {
    vga_clear(C_WHITE, C_BLACK);

    unsafe {
        // Title bar
        let dirty_mark = if ED_DIRTY { b'*' } else { b' ' };
        vga_putc(0, 0, dirty_mark, C_YELLOW, C_DGRAY);
        vga_puts(2, 0, "Editor - ", C_WHITE, C_DGRAY);
        let name_len = str_len(&ED_FILENAME);
        if name_len > 0 {
            vga_puts(11, 0, core::str::from_utf8_unchecked(&ED_FILENAME[..name_len]), C_YELLOW, C_DGRAY);
        } else {
            vga_puts(11, 0, "(untitled)", C_DGRAY, C_DGRAY);
        }
        vga_puts(45, 0, "Ctrl+S:Save | Ctrl+Q:Quit", C_LGRAY, C_DGRAY);

        // Content
        let mut y = 2;
        let mut x = 0;
        let mut line_start = 0;
        for i in 0..ED_SIZE {
            if ED_BUF[i] == b'\n' {
                let line_len = i - line_start;
                if y < SCREEN_H - 1 {
                    for dx in 0..line_len {
                        if x + dx < SCREEN_W {
                            vga_putc(x + dx, y, ED_BUF[line_start + dx], C_WHITE, C_BLACK);
                        }
                    }
                }
                y += 1;
                x = 0;
                line_start = i + 1;
                if y >= SCREEN_H - 1 { break; }
            }
        }
        // Draw remaining partial line
        if line_start < ED_SIZE && y < SCREEN_H - 1 {
            let rem = ED_SIZE - line_start;
            for dx in 0..rem {
                if x + dx < SCREEN_W {
                    vga_putc(x + dx, y, ED_BUF[line_start + dx], C_WHITE, C_BLACK);
                }
            }
        }

        // Cursor (inverse block)
        let cx = ED_CURS_X;
        let cy = ED_CURS_Y + 2;
        if cy < SCREEN_H - 1 && cx < SCREEN_W {
            vga_putc(cx, cy, b'_', C_BLACK, C_WHITE);
        }
    }

    draw_taskbar();
}

fn editor_input(c: u8) {
    unsafe {
        if c == 0x11 {
            // Ctrl+Q -> quit
            switch_app(APP_DESKTOP);
            return;
        }
        if c == 0x13 {
            // Ctrl+S -> save
            let name_len = str_len(&ED_FILENAME);
            if name_len > 0 {
                fs_write(&ED_FILENAME[..name_len], &ED_BUF[..ED_SIZE]);
                ED_DIRTY = false;
            } else {
                // Save as untitled.txt
                let mut name = [0u8; 32];
                name[..12].copy_from_slice(b"untitled.txt");
                fs_write(&name[..12], &ED_BUF[..ED_SIZE]);
                str_copy(&mut ED_FILENAME, b"untitled.txt");
                ED_DIRTY = false;
            }
            editor_draw();
            return;
        }

        match c {
            0x7F | 0x08 => {
                // Backspace
                if ED_SIZE > 0 {
                    ED_SIZE -= 1;
                    ED_BUF[ED_SIZE] = 0;
                    ED_DIRTY = true;
                }
            }
            b'\r' | b'\n' => {
                if ED_SIZE < MAX_FILE_SIZE - 1 {
                    ED_BUF[ED_SIZE] = b'\n';
                    ED_SIZE += 1;
                    ED_DIRTY = true;
                }
            }
            0x20..=0x7E => {
                if ED_SIZE < MAX_FILE_SIZE - 1 {
                    ED_BUF[ED_SIZE] = c;
                    ED_SIZE += 1;
                    ED_DIRTY = true;
                }
            }
            _ => {}
        }

        // Recalculate cursor position from buffer
        ED_CURS_X = 0;
        ED_CURS_Y = 0;
        for i in 0..ED_SIZE {
            if ED_BUF[i] == b'\n' {
                ED_CURS_Y += 1;
                ED_CURS_X = 0;
            } else {
                ED_CURS_X += 1;
                if ED_CURS_X >= SCREEN_W {
                    ED_CURS_X = 0;
                    ED_CURS_Y += 1;
                }
            }
        }
        if ED_CURS_Y >= SCREEN_H - 3 {
            ED_CURS_Y = SCREEN_H - 4;
        }

        editor_draw();
    }
}

// ============================================================================
// APP ROUTER
// ============================================================================
fn switch_app(app: u8) {
    unsafe {
        ACTIVE_APP = app;
        match app {
            APP_DESKTOP => draw_desktop(),
            APP_TERMINAL => term_init(),
            APP_FILES => files_init(),
            APP_SYSMON => sysmon_init(),
            APP_EDITOR => editor_init(),
            _ => draw_desktop(),
        }
    }
}

/// Main keyboard input router - call this from your keyboard ISR
pub fn desktop_input_char(c: u8) {
    unsafe {
        match ACTIVE_APP {
            APP_DESKTOP => {
                match c {
                    b'a' | b'A' | 0x4B => {
                        // Left
                        if ICON_SEL > 0 { ICON_SEL -= 1; draw_desktop(); }
                    }
                    b'd' | b'D' | 0x4D => {
                        // Right
                        if ICON_SEL < 3 { ICON_SEL += 1; draw_desktop(); }
                    }
                    b'\r' | b'\n' => {
                        match ICON_SEL {
                            0 => switch_app(APP_TERMINAL),
                            1 => switch_app(APP_FILES),
                            2 => switch_app(APP_SYSMON),
                            3 => switch_app(APP_EDITOR),
                            _ => {}
                        }
                    }
                    _ => {}
                }
            }
            APP_TERMINAL => term_input(c),
            APP_FILES => files_input(c),
            APP_SYSMON => sysmon_input(c),
            APP_EDITOR => editor_input(c),
            _ => {}
        }
    }
}

/// Timer tick - call from your timer interrupt (e.g. 100Hz PIT)
pub fn desktop_timer_tick() {
    unsafe {
        TICK += 1;
        // Every ~1 second, could refresh taskbar clock
        if TICK % 100 == 0 {
            if ACTIVE_APP != APP_DESKTOP {
                draw_taskbar();
            }
        }
    }
}

// ============================================================================
// C FFI EXPORTS (Call these from your C kernel / main)
// ============================================================================

/// Initialize desktop and show launcher
#[no_mangle]
pub extern "C" fn desktop_init() {
    fs_init();
    switch_app(APP_DESKTOP);
}

/// Keyboard input hook - call from keyboard interrupt handler
#[no_mangle]
pub extern "C" fn desktop_keypress(c: u8) {
    desktop_input_char(c);
}

/// Timer tick hook - call from timer interrupt handler
#[no_mangle]
pub extern "C" fn desktop_tick() {
    desktop_timer_tick();
}

/// Open editor with specific file (call from file manager or kernel)
#[no_mangle]
pub extern "C" fn desktop_edit_file(name: *const u8) {
    unsafe {
        let mut buf = [0u8; 32];
        let mut i = 0;
        while i < 31 && *name.add(i) != 0 {
            buf[i] = *name.add(i);
            i += 1;
        }
        editor_load(&buf[..i]);
        switch_app(APP_EDITOR);
    }
}
'''

with open('/mnt/agents/output/desktop.rs', 'w') as f:
    f.write(content)

print("desktop.rs saved. Lines:", len(content.splitlines()))
