// SPDX-License-Identifier: GPL-2.0
/*
 * linuxab/drivers/acpi/osl.c
 * ACPI OS services layer
 */

#include "acpi.h"
#include "acpi_internal.h"
#include "printk.h"
#include "paging.h"

static uint8_t acpi_osl_mapped_pages[512] = {0};
static uint64_t acpi_osl_map_base = 0xFFFFFFFF81000000ULL; /* After kernel */

int acpi_os_map_memory(uint64_t phys, uint64_t size, void **virt)
{
    uint64_t pages = (size + 4095) / 4096;
    uint64_t vaddr = acpi_osl_map_base;
    
    for (uint64_t i = 0; i < pages; i++) {
        map_page(NULL, vaddr + i * 4096, phys + i * 4096, 0x03); /* RW */
    }
    
    acpi_osl_map_base += pages * 4096;
    *virt = (void *)(vaddr + (phys & 0xFFF));
    return 0;
}

void acpi_os_unmap_memory(void *virt, uint64_t size)
{
    /* Simplified: leak mappings */
}

uint8_t acpi_os_read_port(uint64_t address, uint32_t width)
{
    uint8_t val8;
    uint16_t val16;
    uint32_t val32;
    
    switch (width) {
    case 8:
        __asm__ volatile ("inb %1, %0" : "=a"(val8) : "Nd"((uint16_t)address));
        return val8;
    case 16:
        __asm__ volatile ("inw %1, %0" : "=a"(val16) : "Nd"((uint16_t)address));
        return (uint8_t)val16; /* truncated for return type */
    case 32:
        __asm__ volatile ("inl %1, %0" : "=a"(val32) : "Nd"((uint16_t)address));
        return (uint8_t)val32;
    default:
        return 0;
    }
}

void acpi_os_write_port(uint64_t address, uint8_t value, uint32_t width)
{
    switch (width) {
    case 8:
        __asm__ volatile ("outb %0, %1" :: "a"(value), "Nd"((uint16_t)address));
        break;
    case 16: {
        uint16_t v = value;
        __asm__ volatile ("outw %0, %1" :: "a"(v), "Nd"((uint16_t)address));
        break;
    }
    case 32: {
        uint32_t v = value;
        __asm__ volatile ("outl %0, %1" :: "a"(v), "Nd"((uint16_t)address));
        break;
    }
    }
}

int acpi_os_read_memory(uint64_t phys, uint64_t *value, uint32_t width)
{
    void *virt;
    int ret;
    
    ret = acpi_os_map_memory(phys, 8, &virt);
    if (ret) return ret;
    
    switch (width) {
    case 8:  *value = *(uint8_t *)virt; break;
    case 16: *value = *(uint16_t *)virt; break;
    case 32: *value = *(uint32_t *)virt; break;
    case 64: *value = *(uint64_t *)virt; break;
    default: return -1;
    }
    
    return 0;
}

int acpi_os_write_memory(uint64_t phys, uint64_t value, uint32_t width)
{
    void *virt;
    int ret;
    
    ret = acpi_os_map_memory(phys, 8, &virt);
    if (ret) return ret;
    
    switch (width) {
    case 8:  *(uint8_t *)virt = value; break;
    case 16: *(uint16_t *)virt = value; break;
    case 32: *(uint32_t *)virt = value; break;
    case 64: *(uint64_t *)virt = value; break;
    default: return -1;
    }
    
    return 0;
}

void acpi_os_sleep(uint64_t ms)
{
    arch_timer_mdelay((unsigned int)ms);
}

void acpi_os_stall(uint32_t us)
{
    arch_timer_udelay(us);
}