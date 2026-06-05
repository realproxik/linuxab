// SPDX-License-Identifier: GPL-2.0
/*
 * linuxab/drivers/acpi/battery.c
 * ACPI battery driver
 */

#include "acpi.h"
#include "acpi_internal.h"
#include "printk.h"

static struct acpi_device_id acpi_battery_ids[] = {
    {"PNP0C0A", 0}, /* Control method battery */
    {"ACPI0001", 0}, /* Smart battery */
    {"", 0}
};

struct acpi_battery {
    uint32_t present;
    uint32_t design_capacity;
    uint32_t full_capacity;
    uint32_t current_capacity;
    uint32_t voltage;
    uint32_t rate;
    uint32_t state;
#define ACPI_BATTERY_STATE_DISCHARGING  0x01
#define ACPI_BATTERY_STATE_CHARGING     0x02
#define ACPI_BATTERY_STATE_CRITICAL     0x04
};

static int acpi_battery_add(struct acpi_device *device)
{
    struct acpi_battery *battery;
    
    battery = kmalloc_page();
    if (!battery) return -1;
    
    memset(battery, 0, sizeof(*battery));
    battery->present = 1;
    
    device->driver_data = battery;
    
    printk(KERN_INFO "ACPI: Battery [%s] present\n", device->name);
    return 0;
}

static struct acpi_driver acpi_battery_driver = {
    .name = "battery",
    .ids = acpi_battery_ids,
    .add = acpi_battery_add,
};

int acpi_battery_init(void)
{
    return acpi_bus_register_driver(&acpi_battery_driver);
}