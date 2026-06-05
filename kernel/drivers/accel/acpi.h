
/* SPDX-License-Identifier: GPL-2.0 */
/*
 * linuxab/include/linuxab/acpi.h
 * ACPI core definitions for linuxab
 */

#ifndef _LINUXAB_ACPI_H
#define _LINUXAB_ACPI_H

#include "types.h"

/* ACPI table signatures */
#define ACPI_SIG_RSDP       "RSD PTR "
#define ACPI_SIG_RSDT       "RSDT"
#define ACPI_SIG_XSDT       "XSDT"
#define ACPI_SIG_FACP       "FACP"
#define ACPI_SIG_FADT       ACPI_SIG_FACP
#define ACPI_SIG_DSDT       "DSDT"
#define ACPI_SIG_SSDT       "SSDT"
#define ACPI_SIG_MADT       "APIC"
#define ACPI_SIG_MCFG       "MCFG"
#define ACPI_SIG_HPET       "HPET"
#define ACPI_SIG_SRAT       "SRAT"
#define ACPI_SIG_SLIT       "SLIT"
#define ACPI_SIG_CPEP       "CPEP"
#define ACPI_SIG_ECDT       "ECDT"
#define ACPI_SIG_ERST       "ERST"
#define ACPI_SIG_BERT       "BERT"
#define ACPI_SIG_HEST       "HEST"
#define ACPI_SIG_EINJ       "EINJ"
#define ACPI_SIG_SBST       "SBST"
#define ACPI_SIG_DBGP       "DBGP"
#define ACPI_SIG_DBG2       "DBG2"
#define ACPI_SIG_TCPA       "TCPA"
#define ACPI_SIG_UEFI       "UEFI"
#define ACPI_SIG_RASF       "RASF"
#define ACPI_SIG_PMTT       "PMTT"
#define ACPI_SIG_LPIT       "LPIT"

/* ACPI root table pointers */
#define ACPI_EBDA_PTR_LOCATION      0x40E
#define ACPI_EBDA_PTR_LENGTH        2
#define ACPI_EBDA_WINDOW_SIZE       1024
#define ACPI_HI_RSDP_WINDOW_BASE    0xE0000
#define ACPI_HI_RSDP_WINDOW_SIZE    0x20000
#define ACPI_RSDP_CHECKSUM_LENGTH   20
#define ACPI_RSDP_XCHECKSUM_LENGTH  36

/* ACPI address space types */
#define ACPI_ADR_SPACE_SYSTEM_MEMORY    0
#define ACPI_ADR_SPACE_SYSTEM_IO        1
#define ACPI_ADR_SPACE_PCI_CONFIG       2
#define ACPI_ADR_SPACE_EC               3
#define ACPI_ADR_SPACE_SMBUS            4
#define ACPI_ADR_SPACE_CMOS             5
#define ACPI_ADR_SPACE_PCI_BAR_TARGET   6
#define ACPI_ADR_SPACE_IPMI             7
#define ACPI_ADR_SPACE_GPIO             8
#define ACPI_ADR_SPACE_GSBUS            9
#define ACPI_ADR_SPACE_PLATFORM_COMM    10
#define ACPI_ADR_SPACE_PLATFORM_RT      11
#define ACPI_ADR_SPACE_GENERIC_SOC      12

/* ACPI table header */
struct acpi_table_header {
    char     signature[4];
    uint32_t length;
    uint8_t  revision;
    uint8_t  checksum;
    char     oem_id[6];
    char     oem_table_id[8];
    uint32_t oem_revision;
    char     asl_compiler_id[4];
    uint32_t asl_compiler_revision;
} __attribute__((packed));

/* RSDP */
struct acpi_table_rsdp {
    char     signature[8];
    uint8_t  checksum;
    char     oem_id[6];
    uint8_t  revision;
    uint32_t rsdt_physical_address;
    uint32_t length;
    uint64_t xsdt_physical_address;
    uint8_t  extended_checksum;
    uint8_t  reserved[3];
} __attribute__((packed));

/* RSDT/XSDT */
struct acpi_table_rsdt {
    struct acpi_table_header header;
    uint32_t table_offset_entry[1];
};

struct acpi_table_xsdt {
    struct acpi_table_header header;
    uint64_t table_offset_entry[1];
};

/* FADT (Fixed ACPI Description Table) */
struct acpi_table_fadt {
    struct acpi_table_header header;
    uint32_t facs;
    uint32_t dsdt;
    uint8_t  model;
    uint8_t  preferred_profile;
    uint16_t sci_interrupt;
    uint32_t smi_command;
    uint8_t  acpi_enable;
    uint8_t  acpi_disable;
    uint8_t  s4_bios_request;
    uint8_t  pstate_control;
    uint32_t pm1a_event_block;
    uint32_t pm1b_event_block;
    uint32_t pm1a_control_block;
    uint32_t pm1b_control_block;
    uint32_t pm2_control_block;
    uint32_t pm_timer_block;
    uint32_t gpe0_block;
    uint32_t gpe1_block;
    uint8_t  pm1_event_length;
    uint8_t  pm1_control_length;
    uint8_t  pm2_control_length;
    uint8_t  pm_timer_length;
    uint8_t  gpe0_block_length;
    uint8_t  gpe1_block_length;
    uint8_t  gpe1_base;
    uint8_t  cst_control;
    uint16_t c2_latency;
    uint16_t c3_latency;
    uint16_t flush_size;
    uint16_t flush_stride;
    uint8_t  duty_offset;
    uint8_t  duty_width;
    uint8_t  day_alarm;
    uint8_t  month_alarm;
    uint8_t  century;
    uint16_t boot_flags;
    uint8_t  reserved;
    uint32_t flags;
    struct acpi_generic_address64 reset_register;
    uint8_t  reset_value;
    uint16_t arm_boot_flags;
    uint8_t  minor_revision;
    uint64_t x_firmware_ctrl;
    uint64_t x_dsdt;
    struct acpi_generic_address64 x_pm1a_event_block;
    struct acpi_generic_address64 x_pm1b_event_block;
    struct acpi_generic_address64 x_pm1a_control_block;
    struct acpi_generic_address64 x_pm1b_control_block;
    struct acpi_generic_address64 x_pm2_control_block;
    struct acpi_generic_address64 x_pm_timer_block;
    struct acpi_generic_address64 x_gpe0_block;
    struct acpi_generic_address64 x_gpe1_block;
    struct acpi_generic_address64 sleep_control;
    struct acpi_generic_address64 sleep_status;
    uint64_t hypervisor_id;
} __attribute__((packed));

struct acpi_generic_address64 {
    uint8_t  space_id;
    uint8_t  bit_width;
    uint8_t  bit_offset;
    uint8_t  access_width;
    uint64_t address;
} __attribute__((packed));

/* MADT */
struct acpi_table_madt {
    struct acpi_table_header header;
    uint32_t address;
    uint32_t flags;
};

/* MADT subtable header */
struct acpi_madt_header {
    uint8_t type;
    uint8_t length;
};

/* MADT types */
#define ACPI_MADT_TYPE_LOCAL_APIC           0
#define ACPI_MADT_TYPE_IO_APIC              1
#define ACPI_MADT_TYPE_INTERRUPT_OVERRIDE   2
#define ACPI_MADT_TYPE_NMI_SOURCE           3
#define ACPI_MADT_TYPE_LOCAL_APIC_NMI       4
#define ACPI_MADT_TYPE_LOCAL_APIC_OVERRIDE   5
#define ACPI_MADT_TYPE_IO_SAPIC             6
#define ACPI_MADT_TYPE_LOCAL_SAPIC          7
#define ACPI_MADT_TYPE_INTERRUPT_SOURCE     8
#define ACPI_MADT_TYPE_LOCAL_X2APIC         9
#define ACPI_MADT_TYPE_LOCAL_X2APIC_NMI     10
#define ACPI_MADT_TYPE_GENERIC_INTERRUPT    11
#define ACPI_MADT_TYPE_GENERIC_DISTRIBUTOR  12
#define ACPI_MADT_TYPE_GENERIC_MSI_FRAME    13
#define ACPI_MADT_TYPE_GENERIC_REDISTRIBUTOR 14
#define ACPI_MADT_TYPE_GENERIC_TRANSLATOR   15

/* MCFG */
struct acpi_table_mcfg {
    struct acpi_table_header header;
    uint8_t  reserved[8];
};

struct acpi_mcfg_allocation {
    uint64_t address;
    uint16_t pci_segment;
    uint8_t  start_bus_number;
    uint8_t  end_bus_number;
    uint32_t reserved;
} __attribute__((packed));

/* ECDT */
struct acpi_table_ecdt {
    struct acpi_table_header header;
    struct acpi_ec_id id;
    uint16_t uid;
    uint8_t  gpe_bit;
    struct acpi_generic_address64 control;
    struct acpi_generic_address64 data;
    uint32_t uid32;
    uint8_t  lid;
};

struct acpi_ec_id {
    uint8_t  id[2];
};

/* ACPI device */
struct acpi_device;
struct acpi_driver;

#define ACPI_BUS_TYPE_DEVICE    0
#define ACPI_BUS_TYPE_POWER     1
#define ACPI_BUS_TYPE_PROCESSOR 2
#define ACPI_BUS_TYPE_THERMAL   3
#define ACPI_BUS_TYPE_BUTTON    4

struct acpi_device_id {
    char id[16];
    uint32_t driver_data;
};

struct acpi_device_pnp {
    char bus_id[16];
    struct acpi_device_id *id;
    char unique_id[16];
};

struct acpi_device_power {
    int state;
    struct acpi_device *resources;
};

struct acpi_device_perf {
    int state;
};

struct acpi_device_wakeup {
    int gpe_number;
    uint64_t sleep_state;
};

struct acpi_device {
    struct acpi_device *parent;
    struct list_head children;
    struct list_head node;
    
    struct acpi_device_pnp pnp;
    struct acpi_device_power power;
    struct acpi_device_perf performance;
    struct acpi_device_wakeup wakeup;
    
    struct acpi_table_header *handle;
    struct acpi_driver *driver;
    void *driver_data;
    
    uint32_t flags;
    char name[32];
};

struct acpi_driver {
    char name[32];
    char class[32];
    struct acpi_device_id *ids;
    int (*add)(struct acpi_device *device);
    int (*remove)(struct acpi_device *device);
    void (*notify)(struct acpi_device *device, uint32_t event);
};

/* Sleep states */
#define ACPI_STATE_S0       0
#define ACPI_STATE_S1       1
#define ACPI_STATE_S2       2
#define ACPI_STATE_S3       3
#define ACPI_STATE_S4       4
#define ACPI_STATE_S5       5
#define ACPI_S_STATES_MAX   6

/* GPE */
#define ACPI_GPE_MAX        256

struct acpi_gpe_handler_info {
    uint32_t gpe_number;
    int (*handler)(void *context);
    void *context;
};

/* EC */
#define ACPI_EC_UDELAY      100
#define ACPI_EC_UDELAY_GLK  1000
#define ACPI_EC_MSI_UDELAY  550

struct acpi_ec {
    uint64_t command_addr;
    uint64_t data_addr;
    uint8_t  gpe;
    uint8_t  handle;
    uint16_t uid;
    uint8_t  global_lock;
    uint8_t  is_ecdt;
};

/* Global functions */
int acpi_init(void);
void acpi_shutdown(void);

struct acpi_table_header *acpi_get_table(const char *signature, uint32_t instance);
int acpi_parse_table_header(void *table, struct acpi_table_header *header);
int acpi_validate_rsdp(struct acpi_table_rsdp *rsdp);
int acpi_validate_checksum(void *table, uint32_t length);

uint32_t acpi_read_pm1a_status(void);
uint32_t acpi_read_pm1a_enable(void);
void acpi_write_pm1a_status(uint32_t val);
void acpi_write_pm1a_enable(uint32_t val);
void acpi_write_pm1a_control(uint32_t val);

int acpi_enter_sleep_state(uint8_t state);
int acpi_disable(void);
int acpi_enable(void);

int acpi_bus_init(void);
int acpi_bus_register_driver(struct acpi_driver *driver);
void acpi_bus_unregister_driver(struct acpi_driver *driver);

int acpi_ec_init(void);
int acpi_ec_read(uint8_t address, uint8_t *data);
int acpi_ec_write(uint8_t address, uint8_t data);

int acpi_processor_init(void);
int acpi_thermal_init(void);
int acpi_battery_init(void);
int acpi_button_init(void);
int acpi_power_init(void);
int acpi_pci_root_init(void);

#endif /* _LINUXAB_ACPI_H */