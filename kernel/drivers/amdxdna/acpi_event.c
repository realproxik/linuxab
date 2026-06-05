// SPDX-License-Identifier: GPL-2.0
/*
 * linuxab/drivers/acpi/event.c
 * ACPI event handling (Fixed events + GPEs)
 */

#include "acpi.h"
#include "acpi_internal.h"
#include "printk.h"

struct acpi_gpe_handler_info acpi_gpe_handlers[ACPI_GPE_MAX];

/* PM1 fixed events */
#define ACPI_EVENT_TIMER    0
#define ACPI_EVENT_GLOBAL   1
#define ACPI_EVENT_POWER    2
#define ACPI_EVENT_SLEEP    3
#define ACPI_EVENT_RTC      4
#define ACPI_EVENT_WAKE     5
#define ACPI_EVENT_MAX      6

static uint16_t acpi_fixed_event_en[ACPI_EVENT_MAX] = {
    0x0001, /* TMR */
    0x0020, /* GBL */
    0x0100, /* PWR */
    0x0200, /* SLP */
    0x0400, /* RTC */
    0x0800, /* WAK */
};

static void acpi_clear_gpe(uint32_t gpe)
{
    uint64_t base = (gpe < 8) ? acpi_gpe0_blk : acpi_gpe1_blk;
    uint32_t bit = gpe % 8;
    
    if (base) {
        uint8_t val;
        if (base < 0x10000) {
            __asm__ volatile ("inb %1, %0" : "=a"(val) : "Nd"((uint16_t)(base + 1)));
            val |= (1 << bit);
            __asm__ volatile ("outb %0, %1" :: "a"(val), "Nd"((uint16_t)(base + 1)));
        }
    }
}

static void acpi_enable_gpe(uint32_t gpe)
{
    uint64_t base = (gpe < 8) ? acpi_gpe0_blk : acpi_gpe1_blk;
    uint32_t bit = gpe % 8;
    uint32_t len = (gpe < 8) ? acpi_gpe0_len : acpi_gpe1_len;
    
    if (base && len > 1) {
        uint64_t en_addr = base + len / 2;
        uint8_t val;
        if (en_addr < 0x10000) {
            __asm__ volatile ("inb %1, %0" : "=a"(val) : "Nd"((uint16_t)en_addr));
            val |= (1 << bit);
            __asm__ volatile ("outb %0, %1" :: "a"(val), "Nd"((uint16_t)en_addr));
        }
    }
}

static void acpi_disable_gpe(uint32_t gpe)
{
    uint64_t base = (gpe < 8) ? acpi_gpe0_blk : acpi_gpe1_blk;
    uint32_t bit = gpe % 8;
    uint32_t len = (gpe < 8) ? acpi_gpe0_len : acpi_gpe1_len;
    
    if (base && len > 1) {
        uint64_t en_addr = base + len / 2;
        uint8_t val;
        if (en_addr < 0x10000) {
            __asm__ volatile ("inb %1, %0" : "=a"(val) : "Nd"((uint16_t)en_addr));
            val &= ~(1 << bit);
            __asm__ volatile ("outb %0, %1" :: "a"(val), "Nd"((uint16_t)en_addr));
        }
    }
}

void acpi_gpe_init(void)
{
    memset(acpi_gpe_handlers, 0, sizeof(acpi_gpe_handlers));
    
    /* Disable all GPEs initially */
    for (int i = 0; i < ACPI_GPE_MAX; i++) {
        acpi_disable_gpe(i);
        acpi_clear_gpe(i);
    }
}

int acpi_gpe_install_handler(uint32_t gpe, int (*handler)(void *), void *context)
{
    if (gpe >= ACPI_GPE_MAX)
        return -1;
    
    acpi_gpe_handlers[gpe].gpe_number = gpe;
    acpi_gpe_handlers[gpe].handler = handler;
    acpi_gpe_handlers[gpe].context = context;
    
    acpi_enable_gpe(gpe);
    return 0;
}

void acpi_gpe_enable(uint32_t gpe)
{
    acpi_enable_gpe(gpe);
}

void acpi_gpe_disable(uint32_t gpe)
{
    acpi_disable_gpe(gpe);
}

void acpi_gpe_dispatch(uint32_t gpe)
{
    if (gpe < ACPI_GPE_MAX && acpi_gpe_handlers[gpe].handler) {
        acpi_gpe_handlers[gpe].handler(acpi_gpe_handlers[gpe].context);
    }
    acpi_clear_gpe(gpe);
}

void acpi_fixed_event_dispatch(uint32_t event)
{
    switch (event) {
    case ACPI_EVENT_POWER:
        printk(KERN_INFO "ACPI: Power button pressed\n");
        /* TODO: Trigger shutdown */
        break;
    case ACPI_EVENT_SLEEP:
        printk(KERN_INFO "ACPI: Sleep button pressed\n");
        break;
    case ACPI_EVENT_RTC:
        printk(KERN_INFO "ACPI: RTC alarm\n");
        break;
    default:
        break;
    }
    
    /* Clear status */
    acpi_write_pm1a_status(acpi_fixed_event_en[event]);
}

void acpi_sci_handler(void)
{
    uint32_t pm1a_sts = acpi_read_pm1a_status();
    uint32_t pm1a_en = acpi_read_pm1a_enable();
    uint32_t fixed = pm1a_sts & pm1a_en;
    
    /* Check fixed events */
    for (int i = 0; i < ACPI_EVENT_MAX; i++) {
        if (fixed & acpi_fixed_event_en[i])
            acpi_fixed_event_dispatch(i);
    }
    
    /* Check GPEs */
    if (acpi_gpe0_blk) {
        uint8_t sts, en;
        __asm__ volatile ("inb %1, %0" : "=a"(sts) : "Nd"((uint16_t)acpi_gpe0_blk));
        __asm__ volatile ("inb %1, %0" : "=a"(en) : "Nd"((uint16_t)(acpi_gpe0_blk + acpi_gpe0_len / 2)));
        
        sts &= en;
        for (int i = 0; i < 8; i++) {
            if (sts & (1 << i))
                acpi_gpe_dispatch(i);
        }
    }
}