/* SPDX-License-Identifier: GPL-2.0 */
/*
 * linuxab/drivers/acpi/internal.h
 * Internal ACPI definitions
 */

#ifndef _LINUXAB_ACPI_INTERNAL_H
#define _LINUXAB_ACPI_INTERNAL_H

#include "acpi.h"

#define ACPI_MAX_TABLES     64
#define ACPI_MAX_DEVICES    256
#define ACPI_MAX_DRIVERS    32

struct acpi_table_list {
    struct acpi_table_header *tables[ACPI_MAX_TABLES];
    uint32_t count;
};

struct acpi_namespace_node {
    char name[4];
    uint32_t type;
    struct acpi_namespace_node *parent;
    struct acpi_namespace_node *child;
    struct acpi_namespace_node *peer;
    void *object;
};

/* Table management */
extern struct acpi_table_list acpi_tables;
extern struct acpi_table_fadt *acpi_fadt;
extern struct acpi_table_rsdp *acpi_rsdp;
extern uint64_t acpi_rsdp_phys;

/* Namespace */
extern struct acpi_namespace_node *acpi_root_node;

/* PM registers */
extern uint64_t acpi_pm1a_ctrl;
extern uint64_t acpi_pm1a_evt;
extern uint64_t acpi_pm1b_ctrl;
extern uint64_t acpi_pm1b_evt;
extern uint64_t acpi_pm_tmr;
extern uint64_t acpi_gpe0_blk;
extern uint64_t acpi_gpe1_blk;
extern uint8_t acpi_gpe0_len;
extern uint8_t acpi_gpe1_len;

/* EC */
extern struct acpi_ec *acpi_ec_ecdt;

/* GPE handlers */
extern struct acpi_gpe_handler_info acpi_gpe_handlers[ACPI_GPE_MAX];

/* Internal functions */
int acpi_parse_rsdp(void);
int acpi_parse_fadt(void);
int acpi_parse_madt(void);
int acpi_parse_mcfg(void);
int acpi_parse_ecdt(void);

void acpi_install_table(struct acpi_table_header *table);
void acpi_unmap_table(void *table);

int acpi_os_map_memory(uint64_t phys, uint64_t size, void **virt);
void acpi_os_unmap_memory(void *virt, uint64_t size);

uint8_t acpi_os_read_port(uint64_t address, uint32_t width);
void acpi_os_write_port(uint64_t address, uint8_t value, uint32_t width);

int acpi_os_read_memory(uint64_t phys, uint64_t *value, uint32_t width);
int acpi_os_write_memory(uint64_t phys, uint64_t value, uint32_t width);

void acpi_ns_build(void *dsdt);
struct acpi_namespace_node *acpi_ns_lookup(struct acpi_namespace_node *parent,
                                            const char *name);

void acpi_ec_scan(void);
void acpi_ec_install_handlers(void);

void acpi_gpe_init(void);
int acpi_gpe_install_handler(uint32_t gpe, int (*handler)(void *), void *context);
void acpi_gpe_enable(uint32_t gpe);
void acpi_gpe_disable(uint32_t gpe);
void acpi_gpe_dispatch(uint32_t gpe);

void acpi_fixed_event_dispatch(uint32_t event);

int acpi_sleep_setup(void);
void acpi_sleep_prepare(uint8_t state);
void acpi_sleep_enter(uint8_t state);
void acpi_sleep_exit(uint8_t state);

int acpi_processor_add(struct acpi_device *device);
int acpi_thermal_add(struct acpi_device *device);
int acpi_battery_add(struct acpi_device *device);
int acpi_button_add(struct acpi_device *device);
int acpi_power_add(struct acpi_device *device);
int acpi_pci_root_add(struct acpi_device *device);

#endif /* _LINUXAB_ACPI_INTERNAL_H */