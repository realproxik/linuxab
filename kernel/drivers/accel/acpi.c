// SPDX-License-Identifier: GPL-2.0
/*
 * linuxab/drivers/acpi/bus.c
 * ACPI bus and driver model
 */

#include "acpi.h"
#include "acpi_internal.h"
#include "printk.h"

static struct acpi_device acpi_devices[ACPI_MAX_DEVICES];
static uint32_t acpi_device_count = 0;
static struct acpi_driver *acpi_drivers[ACPI_MAX_DRIVERS];
static uint32_t acpi_driver_count = 0;

struct acpi_namespace_node *acpi_root_node = NULL;

static struct acpi_device *acpi_device_alloc(void)
{
    if (acpi_device_count >= ACPI_MAX_DEVICES)
        return NULL;
    memset(&acpi_devices[acpi_device_count], 0, sizeof(struct acpi_device));
    return &acpi_devices[acpi_device_count++];
}

static int acpi_match_device_ids(struct acpi_device *dev, struct acpi_device_id *ids)
{
    for (uint32_t i = 0; ids[i].id[0]; i++) {
        if (strncmp(dev->pnp.bus_id, ids[i].id, strlen(ids[i].id)) == 0)
            return 1;
    }
    return 0;
}

int acpi_bus_register_driver(struct acpi_driver *driver)
{
    if (acpi_driver_count >= ACPI_MAX_DRIVERS)
        return -1;
    
    acpi_drivers[acpi_driver_count++] = driver;
    printk(KERN_INFO "ACPI: Registered driver %s\n", driver->name);
    
    /* Probe existing devices */
    for (uint32_t i = 0; i < acpi_device_count; i++) {
        struct acpi_device *dev = &acpi_devices[i];
        if (!dev->driver && acpi_match_device_ids(dev, driver->ids)) {
            if (driver->add(dev) == 0) {
                dev->driver = driver;
                printk(KERN_INFO "ACPI: Bound %s to %s\n", dev->name, driver->name);
            }
        }
    }
    
    return 0;
}

void acpi_bus_unregister_driver(struct acpi_driver *driver)
{
    /* TODO: Unbind and remove devices */
}

int acpi_bus_add_device(struct acpi_device *parent, const char *name,
                          uint32_t type, void *handle)
{
    struct acpi_device *dev = acpi_device_alloc();
    if (!dev)
        return -1;
    
    strncpy(dev->name, name, sizeof(dev->name) - 1);
    dev->name[sizeof(dev->name) - 1] = '\0';
    snprintf(dev->pnp.bus_id, sizeof(dev->pnp.bus_id), "%s", name);
    
    dev->parent = parent;
    dev->handle = handle;
    dev->power.state = -1;
    
    if (parent) {
        /* Add to parent's children list */
    }
    
    printk(KERN_DEBUG "ACPI: Device %s added\n", dev->name);
    
    /* Try to bind driver */
    for (uint32_t i = 0; i < acpi_driver_count; i++) {
        struct acpi_driver *drv = acpi_drivers[i];
        if (acpi_match_device_ids(dev, drv->ids)) {
            if (drv->add(dev) == 0) {
                dev->driver = drv;
                printk(KERN_INFO "ACPI: Bound %s to %s\n", dev->name, drv->name);
                break;
            }
        }
    }
    
    return 0;
}

int acpi_bus_init(void)
{
    printk(KERN_INFO "ACPI: Bus initialization\n");
    
    /* Create root device */
    acpi_bus_add_device(NULL, "ACPI", 0, NULL);
    
    return 0;
}
