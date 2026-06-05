// SPDX-License-Identifier: GPL-2.0
/*
 * linuxab/drivers/acpi/power.c
 * ACPI power resources
 */

#include "acpi.h"
#include "acpi_internal.h"
#include "printk.h"

static struct acpi_device_id acpi_power_ids[] = {
    {"LNXPOWER", 0},
    {"", 0}
};

static int acpi_power_add(struct acpi_device *device)
{
    printk(KERN_INFO "ACPI: Power resource %s\n", device->name);
    return 0;
}

static struct acpi_driver acpi_power_driver = {
    .name = "power",
    .ids = acpi_power_ids,
    .add = acpi_power_add,
};

int acpi_power_init(void)
{
    return acpi_bus_register_driver(&acpi_power_driver);
}