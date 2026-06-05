// SPDX-License-Identifier: GPL-2.0
/*
 * linuxab/drivers/acpi/acpi.c
 * ACPI subsystem initialization
 */

#include "acpi.h"
#include "acpi_internal.h"
#include "printk.h"

int acpi_init(void)
{
    int ret;
    
    printk(KERN_INFO "ACPI: Initializing subsystem\n");
    
    ret = acpi_parse_rsdp();
    if (ret) {
        printk(KERN_WARNING "ACPI: No RSDP found, ACPI not available\n");
        return ret;
    }
    
    ret = acpi_parse_fadt();
    if (ret) {
        printk(KERN_ERR "ACPI: Failed to parse FADT\n");
        return ret;
    }
    
    acpi_parse_madt();
    acpi_parse_mcfg();
    acpi_ec_scan();
    
    /* Enable ACPI mode */
    acpi_enable();
    
    /* Initialize subsystems */
    acpi_bus_init();
    acpi_gpe_init();
    acpi_ec_init();
    acpi_power_init();
    acpi_processor_init();
    acpi_thermal_init();
    acpi_battery_init();
    acpi_button_init();
    acpi_pci_root_init();
    
    printk(KERN_INFO "ACPI: Subsystem initialized\n");
    return 0;
}

void acpi_shutdown(void)
{
    acpi_disable();
}