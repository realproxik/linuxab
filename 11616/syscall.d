module os.syscall;

// linuxab OS syscall interface
// Matches your kernel's syscall table

extern(C) @nogc nothrow:

// Syscall numbers - adjust to match your kernel's actual table
enum SYS : long {
    exit       = 0,
    print      = 1,   // debug print to serial/framebuffer
    open       = 2,
    read       = 3,
    write      = 4,
    close      = 5,
    mmap       = 6,
    munmap     = 7,
    exec       = 8,   // execute app
    getpid     = 9,
    wait       = 10,
    fb_init    = 20,  // framebuffer: get info
    fb_blit    = 21,  // framebuffer: blit pixels
    fb_swap    = 22,  // framebuffer: swap buffers
    input_poll = 30,  // keyboard/mouse input
    time_ms    = 40,  // uptime milliseconds
    net_get    = 50,  // download: HTTP GET
    net_post   = 51,
    fs_create  = 60,  // filesystem
    fs_delete  = 61,
    fs_list    = 62,
}

// Inline assembly syscall wrapper
long syscall(long num, long a1 = 0, long a2 = 0, long a3 = 0, 
             long a4 = 0, long a5 = 0, long a6 = 0) {
    long ret;
    version (X86_64) {
        asm {
            "mov %0, %%rax\n"
            "mov %1, %%rdi\n"
            "mov %2, %%rsi\n"
            "mov %3, %%rdx\n"
            "mov %4, %%r10\n"
            "mov %5, %%r8\n"
            "mov %6, %%r9\n"
            "syscall\n"
            "mov %%rax, %7\n"
            : "=r" (ret)
            : "r" (num), "r" (a1), "r" (a2), "r" (a3), 
              "r" (a4), "r" (a5), "r" (a6)
            : "rax", "rdi", "rsi", "rdx", "r10", "r8", "r9", "rcx", "r11", "memory";
        }
    } else version (X86) {
        asm {
            "int $0x80\n"
            : "=a" (ret)
            : "a" (num), "b" (a1), "c" (a2), "d" (a3),
              "S" (a4), "D" (a5)
            : "memory";
        }
    }
    return ret;
}

// Convenience wrappers
void sys_exit(int code) { syscall(SYS.exit, code); }
long sys_open(const(char)* path, int flags) { return syscall(SYS.open, cast(long)path, flags); }
long sys_read(long fd, void* buf, long count) { return syscall(SYS.read, fd, cast(long)buf, count); }
long sys_write(long fd, const(void)* buf, long count) { return syscall(SYS.write, fd, cast(long)buf, count); }
void sys_close(long fd) { syscall(SYS.close, fd); }
void* sys_mmap(void* addr, long len, int prot, int flags, long fd, long off) {
    return cast(void*)syscall(SYS.mmap, cast(long)addr, len, prot, flags, fd, off);
}
void sys_munmap(void* addr, long len) { syscall(SYS.munmap, cast(long)addr, len); }
long sys_time_ms() { return syscall(SYS.time_ms); }

// Framebuffer
struct FBInfo {
    uint width;
    uint height;
    uint pitch;
    uint bpp;
    uint* buffer;
}

bool fb_get_info(FBInfo* info) {
    return syscall(SYS.fb_init, cast(long)info) == 0;
}
void fb_blit(uint* pixels, uint x, uint y, uint w, uint h) {
    syscall(SYS.fb_blit, cast(long)pixels, x, y, w, h);
}
void fb_swap() { syscall(SYS.fb_swap); }

// Input
struct InputEvent {
    uint type;   // 0=key, 1=mouse_move, 2=mouse_click
    uint code;   // keycode or button
    int  x;      // mouse x
    int  y;      // mouse y
}

bool input_poll(InputEvent* ev) {
    return syscall(SYS.input_poll, cast(long)ev) == 1;
}

// Network download
long net_get(const(char)* url, void* buf, long max_len) {
    return syscall(SYS.net_get, cast(long)url, cast(long)buf, max_len);
}

// Filesystem
long fs_create(const(char)* path, const(void)* data, long len) {
    return syscall(SYS.fs_create, cast(long)path, cast(long)data, len);
}
long fs_delete(const(char)* path) {
    return syscall(SYS.fs_delete, cast(long)path);
}
long fs_list(const(char)* dir, char* buf, long max) {
    return syscall(SYS.fs_list, cast(long)dir, cast(long)buf, max);
}