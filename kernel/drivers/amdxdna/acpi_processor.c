// SPDX-License-Identifier: GPL-2.0
/*
 * linuxab/drivers/acpi/processor.c
 * ACPI processor driver (P-states, C-states)
 */

#include "acpi.h"
#include "acpi_internal.h"
#include "printk.h"

static struct acpi_device_id acpi_processor_ids[] = {
    {"ACPI0007", 0},
    {"LNXCPU", 0},
    {"INTC0002", 0}, /* AMD */
    {"", 0}
};

struct acpi_processor {
    uint32_t id;
    uint32_t pblk;
    uint32_t acpi_id;
    uint32_t performance_state;
};

static int acpi_processor_add(struct acpi_device *device)
{
    struct acpi_processor *pr;
    
    pr = kmalloc_page();
    if (!pr) return -1;
    
    memset(pr, 0, sizeof(*pr));
    pr->acpi_id = 0; /* Parse from _UID */
    
    device->driver_data = pr;
    
    printk(KERN_INFO "ACPI: Processor [%s] ACPI ID %d\n",
           device->name, pr->acpi_id);
    
    /* TODO: Evaluate _PDC, _PPC, _PCT, _PSS, _CST, _CSD */
    
    return 0;
}

static struct acpi_driver acpi_processor_driver = {
    .name = "processor",
    .ids = acpi_processor_ids,
    .add = acpi_processor_add,
};

int acpi_processor_init(void)
{
    return acpi_bus_register_driver(&acpi_processor_driver);
}