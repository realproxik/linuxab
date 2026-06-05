// SPDX-License-Identifier: GPL-2.0
/*
 * linuxab/drivers/acpi/resource.c
 * ACPI resource parsing (_CRS, _PRS)
 */

#include "acpi.h"
#include "acpi_internal.h"
#include "printk.h"

/* Small resource types */
#define ACPI_RESOURCE_IRQ               0x20
#define ACPI_RESOURCE_DMA               0x21
#define ACPI_RESOURCE_START_DEPENDENT   0x22
#define ACPI_RESOURCE_END_DEPENDENT     0x23
#define ACPI_RESOURCE_IO                0x24
#define ACPI_RESOURCE_FIXED_IO          0x25
#define ACPI_RESOURCE_FIXED_DMA         0x26
#define ACPI_RESOURCE_RESERVED_S1       0x27
#define ACPI_RESOURCE_RESERVED_S2       0x28
#define ACPI_RESOURCE_RESERVED_S3       0x29
#define ACPI_RESOURCE_RESERVED_S4       0x2A
#define ACPI_RESOURCE_VENDOR_SMALL      0x2B
#define ACPI_RESOURCE_END_TAG           0x2F

/* Large resource types */
#define ACPI_RESOURCE_MEMORY24          0x81
#define ACPI_RESOURCE_GENERIC_REG       0x82
#define ACPI_RESOURCE_RESERVED_L1       0x83
#define ACPI_RESOURCE_VENDOR_LARGE      0x84
#define ACPI_RESOURCE_MEMORY32          0x85
#define ACPI_RESOURCE_FIXED_MEMORY32    0x86
#define ACPI_RESOURCE_ADDRESS16         0x87
#define ACPI_RESOURCE_ADDRESS32         0x88
#define ACPI_RESOURCE_ADDRESS64         0x89
#define ACPI_RESOURCE_EXTENDED_ADDRESS  0x8A
#define ACPI_RESOURCE_EXTENDED_IRQ      0x8B
#define ACPI_RESOURCE_GPIO              0x8C
#define ACPI_RESOURCE_PIN_FUNCTION      0x8D
#define ACPI_RESOURCE_PIN_CONFIG        0x8E
#define ACPI_RESOURCE_PIN_GROUP         0x8F
#define ACPI_RESOURCE_PIN_GROUP_FUNCTION 0x90
#define ACPI_RESOURCE_PIN_GROUP_CONFIG  0x91
#define ACPI_RESOURCE_CLOCK_INPUT       0x92

struct acpi_resource_io {
    uint8_t  info;
    uint16_t minimum;
    uint16_t maximum;
    uint8_t  alignment;
    uint8_t  length;
};

struct acpi_resource_memory32 {
    uint8_t  info;
    uint16_t minimum;
    uint16_t maximum;
    uint16_t alignment;
    uint16_t length;
};

struct acpi_resource_address64 {
    uint8_t  type;
    uint8_t  flags;
    uint8_t  tflags;
    uint64_t granularity;
    uint64_t minimum;
    uint64_t maximum;
    uint64_t translation;
    uint64_t length;
    uint64_t source;
};

struct acpi_resource_extended_irq {
    uint8_t  flags;
    uint8_t  count;
    uint32_t interrupts[1];
};

struct acpi_resource {
    uint32_t type;
    union {
        struct acpi_resource_io io;
        struct acpi_resource_memory32 memory32;
        struct acpi_resource_address64 address64;
        struct acpi_resource_extended_irq ext_irq;
    } data;
};

static int acpi_parse_resource_data(uint8_t *buffer, uint32_t length,
                                    struct acpi_resource *res, uint32_t *consumed)
{
    uint8_t tag = buffer[0];
    uint32_t len;
    
    if (tag & 0x80) {
        /* Large resource */
        len = buffer[1] | (buffer[2] << 8);
        *consumed = len + 3;
        
        switch (tag) {
        case ACPI_RESOURCE_ADDRESS64:
            res->type = tag;
            /* Parse address64 */
            break;
        case ACPI_RESOURCE_EXTENDED_IRQ:
            res->type = tag;
            break;
        default:
            break;
        }
    } else {
        /* Small resource */
        len = tag & 0x07;
        *consumed = len + 1;
        tag &= 0xF8;
        
        switch (tag) {
        case ACPI_RESOURCE_IRQ:
            res->type = tag;
            break;
        case ACPI_RESOURCE_IO:
            res->type = tag;
            if (len >= 7) {
                res->data.io.info = buffer[1];
                res->data.io.minimum = buffer[2] | (buffer[3] << 8);
                res->data.io.maximum = buffer[4] | (buffer[5] << 8);
                res->data.io.alignment = buffer[6];
                res->data.io.length = buffer[7];
            }
            break;
        case ACPI_RESOURCE_END_TAG:
            return 1; /* End */
        default:
            break;
        }
    }
    
    return 0;
}

int acpi_parse_resources(void *buffer, uint32_t length,
                         int (*handler)(struct acpi_resource *, void *),
                         void *context)
{
    uint8_t *ptr = buffer;
    uint8_t *end = ptr + length;
    uint32_t consumed;
    struct acpi_resource res;
    int ret;
    
    while (ptr < end) {
        memset(&res, 0, sizeof(res));
        ret = acpi_parse_resource_data(ptr, end - ptr, &res, &consumed);
        if (ret == 1)
            break; /* End tag */
        if (ret < 0)
            return ret;
        
        if (handler)
            handler(&res, context);
        
        ptr += consumed;
    }
    
    return 0;
}