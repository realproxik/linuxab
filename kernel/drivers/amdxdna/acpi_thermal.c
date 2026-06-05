// SPDX-License-Identifier: GPL-2.0
/*
 * linuxab/drivers/acpi/thermal.c
 * ACPI thermal zone driver
 */

#include "acpi.h"
#include "acpi_internal.h"
#include "printk.h"

static struct acpi_device_id acpi_thermal_ids[] = {
    {"LNXTHERM", 0},
    {"ACPI_THERMAL", 0},
    {"INT3400", 0}, /* Intel DPTF */
    {"INT3403", 0},
    {"", 0}
};

struct acpi_thermal {
    uint32_t state;
    uint32_t temperature; /* deci-Kelvin */
    uint32_t critical;
    uint32_t passive;
    uint32_t active[2];
};

static int acpi_thermal_add(struct acpi_device *device)
{
    struct acpi_thermal *tz;
    
    tz = kmalloc_page();
    if (!tz) return -1;
    
    memset(tz, 0, sizeof(*tz));
    tz->critical = 3732; /* 100C in deci-Kelvin */
    tz->passive = 3532;  /* 80C */
    
    device->driver_data = tz;
    
    printk(KERN_INFO "ACPI: Thermal zone [%s] (%d.%d C)\n",
           device->name, tz->temperature / 10 - 273, tz->temperature % 10);
    return 0;
}

static struct acpi_driver acpi_thermal_driver = {
    .name = "thermal",
    .ids = acpi_thermal_ids,
    .add = acpi_thermal_add,
};

int acpi_thermal_init(void)
{
    return acpi_bus_register_driver(&acpi_thermal_driver);
}