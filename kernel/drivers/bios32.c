// SPDX-License-Identifier: GPL-2.0
/*
 * linuxab/arch/x86/kernel/bios32.c
 * BIOS32 Service Directory and PCI BIOS calls
 */

#include <stdint.h>
#include <stdbool.h>

#define BIOS32_SIGNATURE        ((uint32_t)0x5F32335F) /* "_32_" */
#define PCI_SIGNATURE           ((uint32_t)0x5F504349) /* "_PCI" */
#define PCI_BIOS_PRESENT_STATUS 0xB101

/* BIOS32 Service Directory entry point (physical) */
static uint32_t bios32_entry = 0;
static uint32_t pci_entry = 0;

/* BIOS32 SD header at 0xE0000-0xFFFFF */
struct bios32_sd {
    uint32_t signature;
    uint32_t entry;
    uint8_t  revision;
    uint8_t  length;
    uint8_t  checksum;
    uint8_t  reserved[5];
} __attribute__((packed));

static inline uint8_t inb(uint16_t port)
{
    uint8_t v;
    __asm__ volatile ("inb %1, %0" : "=a"(v) : "Nd"(port));
    return v;
}

static inline void outb(uint16_t port, uint8_t val)
{
    __asm__ volatile ("outb %0, %1" :: "a"(val), "Nd"(port));
}

static inline void io_delay(void)
{
    __asm__ volatile ("outb %al, $0x80");
}

/* Simple checksum validation */
static bool bios32_valid(struct bios32_sd *sd)
{
    uint8_t *p = (uint8_t *)sd;
    uint8_t sum = 0;
    
    for (int i = 0; i < sd->length * 16; i++)
        sum += p[i];
    
    return sum == 0;
}

/* Scan for BIOS32 SD */
static uint32_t bios32_scan(void)
{
    uint32_t *addr;
    
    for (addr = (uint32_t *)0xE0000; addr < (uint32_t *)0x100000; addr += 4) {
        if (*addr == BIOS32_SIGNATURE) {
            struct bios32_sd *sd = (struct bios32_sd *)addr;
            if (bios32_valid(sd))
                return sd->entry;
        }
    }
    return 0;
}

/* Call BIOS32 service */
static uint32_t bios32_service(uint32_t service, uint32_t *base, uint32_t *length)
{
    if (!bios32_entry)
        return 0;
    
    uint32_t result = 0;
    uint32_t _base = 0, _length = 0;
    
    __asm__ volatile (
        "lcall *%%cs:%[entry]\n"
        "jc 1f\n"
        "xor %%eax, %%eax\n"
        "1:"
        : "=a"(result), "=b"(_base), "=c"(_length)
        : "a"(service), [entry] "m"(bios32_entry)
        : "memory", "cc"
    );
    
    *base = _base;
    *length = _length;
    return result;
}

/* PCI BIOS calls */
static uint32_t pci_bios_call(uint32_t ax, uint32_t bx, uint32_t cx, uint32_t dx,
                              uint32_t *ret_bx, uint32_t *ret_cx, uint32_t *ret_dx)
{
    uint32_t _ax = ax, _bx = bx, _cx = cx, _dx = dx;
    
    __asm__ volatile (
        "lcall *%%cs:%[entry]\n"
        "jc 1f\n"
        "xor %%ah, %%ah\n"
        "1:"
        : "+a"(_ax), "+b"(_bx), "+c"(_cx), "+d"(_dx)
        : [entry] "m"(pci_entry)
        : "memory", "cc", "si", "di"
    );
    
    if (ret_bx) *ret_bx = _bx;
    if (ret_cx) *ret_cx = _cx;
    if (ret_dx) *ret_dx = _dx;
    
    return _ax;
}

void bios32_init(void)
{
    uint32_t base, length;
    
    bios32_entry = bios32_scan();
    if (!bios32_entry) {
        /* No BIOS32 - try direct PCI config access */
        return;
    }
    
    /* Check PCI BIOS */
    if (bios32_service(PCI_SIGNATURE, &base, &length) == 0) {
        pci_entry = base + 0x00; /* Entry at base + 0 */
        /* Real PCI BIOS entry is at base + 0x03 after entry point */
        pci_entry = base + 0x03;
    }
}

bool pci_bios_present(void)
{
    return pci_entry != 0;
}

uint32_t pci_bios_read_config_byte(uint8_t bus, uint8_t devfn, uint8_t reg, uint8_t *val)
{
    uint32_t bx = (bus << 8) | devfn;
    uint32_t cx = reg;
    uint32_t dx = 0;
    uint32_t ret = pci_bios_call(0xB108, bx, cx, dx, &bx, &cx, &dx);
    *val = (uint8_t)cx;
    return ret;
}

uint32_t pci_bios_read_config_word(uint8_t bus, uint8_t devfn, uint8_t reg, uint16_t *val)
{
    uint32_t bx = (bus << 8) | devfn;
    uint32_t cx = reg;
    uint32_t dx = 0;
    uint32_t ret = pci_bios_call(0xB109, bx, cx, dx, &bx, &cx, &dx);
    *val = (uint16_t)cx;
    return ret;
}

uint32_t pci_bios_read_config_dword(uint8_t bus, uint8_t devfn, uint8_t reg, uint32_t *val)
{
    uint32_t bx = (bus << 8) | devfn;
    uint32_t cx = reg;
    uint32_t dx = 0;
    uint32_t ret = pci_bios_call(0xB10A, bx, cx, dx, &bx, &cx, &dx);
    *val = cx;
    return ret;
}

uint32_t pci_bios_write_config_byte(uint8_t bus, uint8_t devfn, uint8_t reg, uint8_t val)
{
    uint32_t bx = (bus << 8) | devfn;
    uint32_t cx = (reg << 8) | val;
    return pci_bios_call(0xB10B, bx, cx, 0, NULL, NULL, NULL);
}

uint32_t pci_bios_write_config_word(uint8_t bus, uint8_t devfn, uint8_t reg, uint16_t val)
{
    uint32_t bx = (bus << 8) | devfn;
    uint32_t cx = (reg << 16) | val;
    return pci_bios_call(0xB10C, bx, cx, 0, NULL, NULL, NULL);
}

uint32_t pci_bios_write_config_dword(uint8_t bus, uint8_t devfn, uint8_t reg, uint32_t val)
{
    uint32_t bx = (bus << 8) | devfn;
    uint32_t cx = reg;
    uint32_t dx = val;
    return pci_bios_call(0xB10D, bx, cx, dx, NULL, NULL, NULL);
}
