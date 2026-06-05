// SPDX-License-Identifier: GPL-2.0
/*
 * linuxab/drivers/acpi/button.c
 * ACPI button driver (power, sleep, lid)
 */

#include "acpi.h"
#include "acpi_internal.h"
#include "printk.h"

#define ACPI_BUTTON_TYPE_POWER      0
#define ACPI_BUTTON_TYPE_SLEEP      1
#define ACPI_BUTTON_TYPE_LID        2

static struct acpi_device_id acpi_button_ids[] = {
    {"PNP0C0C", 0}, /* Power button */
    {"PNP0C0E", 0}, /* Sleep button */
    {"PNP0C0D", 0}, /* Lid */
    {"ACPI_FPB", 0}, /* Fixed power button */
    {"ACPI_FSB", 0}, /* Fixed sleep button */
    {"", 0}
};

struct acpi_button {
    uint32_t type;
    uint32_t pushed;
};

static int acpi_button_add(struct acpi_device *device)
{
    struct acpi_button *button;
    const char *type_str = "unknown";
    
    button = kmalloc_page();
    if (!button) return -1;
    
    memset(button, 0, sizeof(*button));
    
    if (strncmp(device->pnp.bus_id, "PNP0C0C", 7) == 0 ||
        strncmp(device->pnp.bus_id, "ACPI_FPB", 8) == 0) {
        button->type = ACPI_BUTTON_TYPE_POWER;
        type_str = "power";
    } else if (strncmp(device->pnp.bus_id, "PNP0C0E", 7) == 0 ||
               strncmp(device->pnp.bus_id, "ACPI_FSB", 8) == 0) {
        button->type = ACPI_BUTTON_TYPE_SLEEP;
        type_str = "sleep";
    } else if (strncmp(device->pnp.bus_id, "PNP0C0D", 7) == 0) {
        button->type = ACPI_BUTTON_TYPE_LID;
        type_str = "lid";
    }
    
    device->driver_data = button;
    
    printk(KERN_INFO "ACPI: %s button [%s]\n", type_str, device->name);
    return 0;
}

static struct acpi_driver acpi_button_driver = {
    .name = "button",
    .ids = acpi_button_ids,
    .add = acpi_button_add,
};

int acpi_button_init(void)
{
    return acpi_bus_register_driver(&acpi_button_driver);
}