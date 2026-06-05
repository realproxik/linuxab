// SPDX-License-Identifier: GPL-2.0
/*
 * linuxab/drivers/acpi/ec.c
 * ACPI Embedded Controller driver
 */

#include "acpi.h"
#include "acpi_internal.h"
#include "printk.h"

#define EC_SC           0x66  /* Status/Command */
#define EC_DATA         0x62  /* Data */
#define EC_TIMEOUT      1000

/* Status register bits */
#define EC_OBF          0x01  /* Output Buffer Full */
#define EC_IBF          0x02  /* Input Buffer Full */
#define EC_CMD          0x08  /* CMD bit */
#define EC_BURST        0x10  /* Burst mode */
#define EC_SCI          0x20  /* SCI pending */

/* Commands */
#define EC_READ         0x80
#define EC_WRITE        0x81
#define EC_BURST_ENABLE 0x82
#define EC_BURST_DISABLE 0x83
#define EC_QUERY        0x84

struct acpi_ec *acpi_ec_ecdt = NULL;
static struct acpi_ec acpi_ec_controllers[4];
static int acpi_ec_count = 0;

static uint8_t ec_read_status(void)
{
    uint8_t val;
    __asm__ volatile ("inb %1, %0" : "=a"(val) : "Nd"(EC_SC));
    return val;
}

static uint8_t ec_read_data(void)
{
    uint8_t val;
    __asm__ volatile ("inb %1, %0" : "=a"(val) : "Nd"(EC_DATA));
    return val;
}

static void ec_write_data(uint8_t val)
{
    __asm__ volatile ("outb %0, %1" :: "a"(val), "Nd"(EC_DATA));
}

static void ec_write_cmd(uint8_t val)
{
    __asm__ volatile ("outb %0, %1" :: "a"(val), "Nd"(EC_SC));
}

static int ec_wait_ibf_clear(void)
{
    int timeout = EC_TIMEOUT;
    while (timeout--) {
        if (!(ec_read_status() & EC_IBF))
            return 0;
        arch_timer_udelay(1);
    }
    return -1;
}

static int ec_wait_obf_set(void)
{
    int timeout = EC_TIMEOUT;
    while (timeout--) {
        if (ec_read_status() & EC_OBF)
            return 0;
        arch_timer_udelay(1);
    }
    return -1;
}

static int ec_transaction(uint8_t cmd, uint8_t addr, uint8_t *data, int is_write)
{
    if (ec_wait_ibf_clear() != 0)
        return -1;
    
    ec_write_cmd(cmd);
    
    if (ec_wait_ibf_clear() != 0)
        return -1;
    
    ec_write_data(addr);
    
    if (is_write) {
        if (ec_wait_ibf_clear() != 0)
            return -1;
        ec_write_data(*data);
        return 0;
    } else {
        if (ec_wait_obf_set() != 0)
            return -1;
        *data = ec_read_data();
        return 0;
    }
}

int acpi_ec_read(uint8_t address, uint8_t *data)
{
    return ec_transaction(EC_READ, address, data, 0);
}

int acpi_ec_write(uint8_t address, uint8_t data)
{
    return ec_transaction(EC_WRITE, address, &data, 1);
}

void acpi_ec_scan(void)
{
    /* Look for ECDT */
    struct acpi_table_ecdt *ecdt = (struct acpi_table_ecdt *)acpi_get_table(ACPI_SIG_ECDT, 0);
    if (ecdt) {
        struct acpi_ec *ec = &acpi_ec_controllers[0];
        ec->command_addr = ecdt->control.address;
        ec->data_addr = ecdt->data.address;
        ec->gpe = ecdt->gpe_bit;
        ec->uid = ecdt->uid;
        ec->is_ecdt = 1;
        acpi_ec_ecdt = ec;
        acpi_ec_count = 1;
        printk(KERN_INFO "ACPI: ECDT EC at GPE 0x%x, ports 0x%llx/0x%llx\n",
               ec->gpe, ec->command_addr, ec->data_addr);
    }
}

void acpi_ec_install_handlers(void)
{
    /* TODO: Install GPE handler for EC */
}

int acpi_ec_init(void)
{
    acpi_ec_scan();
    acpi_ec_install_handlers();
    return 0;
}