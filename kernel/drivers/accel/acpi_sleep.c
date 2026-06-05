// SPDX-License-Identifier: GPL-2.0
/*
 * linuxab/drivers/acpi/sleep.c
 * ACPI sleep state management
 */

#include "acpi.h"
#include "acpi_internal.h"
#include "printk.h"

#define ACPI_PM1_CTRL_SLEEP_ENABLE  0x2000
#define ACPI_PM1_CTRL_SLEEP_MASK    0x1C00
#define ACPI_PM1_STATUS_WAK_STS     0x8000

uint64_t acpi_pm1a_ctrl = 0;
uint64_t acpi_pm1a_evt = 0;
uint64_t acpi_pm1b_ctrl = 0;
uint64_t acpi_pm1b_evt = 0;
uint64_t acpi_pm_tmr = 0;

static uint32_t acpi_read_pm1a_ctrl(void)
{
    if (acpi_pm1a_ctrl < 0x10000) {
        uint32_t val;
        __asm__ volatile ("inw %1, %0" : "=a"(val) : "Nd"((uint16_t)acpi_pm1a_ctrl));
        return val;
    }
    return *(volatile uint32_t *)(0xFFFFFFFF80000000ULL + acpi_pm1a_ctrl);
}

static void acpi_write_pm1a_ctrl(uint32_t val)
{
    if (acpi_pm1a_ctrl < 0x10000) {
        __asm__ volatile ("outw %0, %1" :: "a"((uint16_t)val), "Nd"((uint16_t)acpi_pm1a_ctrl));
    } else {
        *(volatile uint32_t *)(0xFFFFFFFF80000000ULL + acpi_pm1a_ctrl) = val;
    }
}

static uint32_t acpi_read_pm1a_sts(void)
{
    if (acpi_pm1a_evt < 0x10000) {
        uint32_t val;
        __asm__ volatile ("inw %1, %0" : "=a"(val) : "Nd"((uint16_t)acpi_pm1a_evt));
        return val;
    }
    return *(volatile uint32_t *)(0xFFFFFFFF80000000ULL + acpi_pm1a_evt);
}

static void acpi_write_pm1a_sts(uint32_t val)
{
    if (acpi_pm1a_evt < 0x10000) {
        __asm__ volatile ("outw %0, %1" :: "a"((uint16_t)val), "Nd"((uint16_t)acpi_pm1a_evt));
    } else {
        *(volatile uint32_t *)(0xFFFFFFFF80000000ULL + acpi_pm1a_evt) = val;
    }
}

uint32_t acpi_read_pm1a_status(void) { return acpi_read_pm1a_sts(); }
uint32_t acpi_read_pm1a_enable(void)
{
    /* PM1a_EN is PM1a_EVT + PM1_EVT_LEN/2 */
    uint64_t addr = acpi_pm1a_evt + 2;
    if (addr < 0x10000) {
        uint32_t val;
        __asm__ volatile ("inw %1, %0" : "=a"(val) : "Nd"((uint16_t)addr));
        return val;
    }
    return *(volatile uint32_t *)(0xFFFFFFFF80000000ULL + addr);
}

void acpi_write_pm1a_status(uint32_t val) { acpi_write_pm1a_sts(val); }
void acpi_write_pm1a_enable(uint32_t val)
{
    uint64_t addr = acpi_pm1a_evt + 2;
    if (addr < 0x10000) {
        __asm__ volatile ("outw %0, %1" :: "a"((uint16_t)val), "Nd"((uint16_t)addr));
    } else {
        *(volatile uint32_t *)(0xFFFFFFFF80000000ULL + addr) = val;
    }
}

void acpi_write_pm1a_control(uint32_t val)
{
    acpi_write_pm1a_ctrl(val);
}

int acpi_enable(void)
{
    if (!acpi_fadt)
        return -1;
    
    if (acpi_fadt->smi_command && acpi_fadt->acpi_enable) {
        /* Write ACPI_ENABLE to SMI_CMD */
        __asm__ volatile ("outb %0, %1" :: "a"(acpi_fadt->acpi_enable),
                          "Nd"((uint16_t)acpi_fadt->smi_command));
        /* Wait for SCI_EN */
        int timeout = 1000;
        while (timeout--) {
            if (acpi_read_pm1a_ctrl() & 1)
                break;
            arch_timer_mdelay(1);
        }
    }
    
    printk(KERN_INFO "ACPI: Enabled\n");
    return 0;
}

int acpi_disable(void)
{
    if (!acpi_fadt)
        return -1;
    
    if (acpi_fadt->smi_command && acpi_fadt->acpi_disable) {
        __asm__ volatile ("outb %0, %1" :: "a"(acpi_fadt->acpi_disable),
                          "Nd"((uint16_t)acpi_fadt->smi_command));
    }
    
    printk(KERN_INFO "ACPI: Disabled\n");
    return 0;
}

int acpi_enter_sleep_state(uint8_t state)
{
    uint32_t pm1a_ctrl;
    uint32_t pm1b_ctrl;
    
    if (state < 1 || state > 5)
        return -1;
    
    if (!acpi_fadt)
        return -1;
    
    printk(KERN_INFO "ACPI: Entering S%d\n", state);
    
    /* Clear PM1 status */
    acpi_write_pm1a_sts(ACPI_PM1_STATUS_WAK_STS);
    
    /* Setup sleep type */
    pm1a_ctrl = (acpi_read_pm1a_ctrl() & ~ACPI_PM1_CTRL_SLEEP_MASK) |
                ((state & 7) << 10) | ACPI_PM1_CTRL_SLEEP_ENABLE;
    pm1b_ctrl = pm1a_ctrl;
    
    /* Disable interrupts */
    __asm__ volatile ("cli");
    
    /* Write PM1 control */
    acpi_write_pm1a_ctrl(pm1a_ctrl);
    if (acpi_pm1b_ctrl)
        acpi_write_pm1b_ctrl(pm1b_ctrl);
    
    /* For S5, we might not wake up */
    if (state == 5) {
        printk(KERN_INFO "ACPI: Power off\n");
        while (1) __asm__ volatile ("hlt");
    }
    
    /* Wait for wake */
    while (!(acpi_read_pm1a_sts() & ACPI_PM1_STATUS_WAK_STS))
        __asm__ volatile ("hlt");
    
    printk(KERN_INFO "ACPI: Woke from S%d\n", state);
    return 0;
}

int acpi_sleep_setup(void)
{
    /* Setup wake GPEs */
    return 0;
}

void acpi_sleep_prepare(uint8_t state)
{
    /* Run _PTS(state) if available */
}

void acpi_sleep_enter(uint8_t state)
{
    acpi_enter_sleep_state(state);
}

void acpi_sleep_exit(uint8_t state)
{
    /* Run _WAK(state) if available */
    acpi_write_pm1a_sts(ACPI_PM1_STATUS_WAK_STS);
}