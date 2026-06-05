// SPDX-License-Identifier: GPL-2.0
/*
 * linuxab/drivers/acpi/pci_root.c
 * ACPI PCI root bridge driver
 */

#include "acpi.h"
#include "acpi_internal.h"
#include "printk.h"

static struct acpi_device_id acpi_pci_root_ids[] = {
    {"PNP0A03", 0}, /* PCI bus */
    {"PNP0A08", 0}, /* PCIe bus */
    {"", 0}
};

struct acpi_pci_root {
    uint16_t segment;
    uint8_t  bus_nr;
    uint64_t mcfg_base;
};

static int acpi_pci_root_add(struct acpi_device *device)
{
    struct acpi_pci_root *root;
    
    root = kmalloc_page();
    if (!root) return -1;
    
    memset(root, 0, sizeof(*root));
    root->segment = 0;
    root->bus_nr = 0; /* Parse from _BBN */
    
    device->driver_data = root;
    
    printk(KERN_INFO "ACPI: PCI root bridge [%s] seg=%d bus=%d\n",
           device->name, root->segment, root->bus_nr);
    
    /* TODO: Start PCI bus enumeration */
    
    return 0;
}

static struct acpi_driver acpi_pci_root_driver = {
    .name = "pci_root",
    .ids = acpi_pci_root_ids,
    .add = acpi_pci_root_add,
};

int acpi_pci_root_init(void)
{
    return acpi_bus_register_driver(&acpi_pci_root_driver);
}