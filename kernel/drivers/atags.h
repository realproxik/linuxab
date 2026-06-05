// SPDX-License-Identifier: GPL-2.0
/*
 * linuxab/include/linuxab/atags.h
 * Boot parameter tags for linuxab
 * Compatible with ARM-style ATAGS and x86 boot_params
 */

#ifndef _LINUXAB_ATAGS_H
#define _LINUXAB_ATAGS_H

#include <stdint.h>

#define ATAG_NONE       0x00000000
#define ATAG_CORE       0x54410001
#define ATAG_MEM        0x54410002
#define ATAG_VIDEOTEXT  0x54410003
#define ATAG_RAMDISK    0x54410004
#define ATAG_INITRD2    0x54420005
#define ATAG_SERIAL     0x54410006
#define ATAG_REVISION   0x54410007
#define ATAG_VIDEOLFB   0x54410008
#define ATAG_CMDLINE    0x54410009
#define ATAG_MEM64      0x5441000A

struct atag_header {
    uint32_t size; /* Size in words including header */
    uint32_t tag;
};

struct atag_core {
    uint32_t flags;
    uint32_t pagesize;
    uint32_t rootdev;
};

struct atag_mem {
    uint32_t size;
    uint32_t start;
};

struct atag_mem64 {
    uint64_t size;
    uint64_t start;
};

struct atag_serialnr {
    uint32_t low;
    uint32_t high;
};

struct atag_revision {
    uint32_t rev;
};

struct atag_videotext {
    uint8_t  x;
    uint8_t  y;
    uint16_t video_page;
    uint8_t  video_mode;
    uint8_t  video_cols;
    uint16_t video_ega_bx;
    uint8_t  video_lines;
    uint8_t  video_isvga;
    uint16_t video_points;
};

struct atag_ramdisk {
    uint32_t flags;
    uint32_t size;
    uint32_t start;
};

struct atag_initrd2 {
    uint32_t start;
    uint32_t size;
};

struct atag_videolfb {
    uint16_t lfb_width;
    uint16_t lfb_height;
    uint16_t lfb_depth;
    uint16_t lfb_linelength;
    uint32_t lfb_base;
    uint32_t lfb_size;
    uint8_t  red_size;
    uint8_t  red_pos;
    uint8_t  green_size;
    uint8_t  green_pos;
    uint8_t  blue_size;
    uint8_t  blue_pos;
    uint8_t  rsvd_size;
    uint8_t  rsvd_pos;
};

struct atag_cmdline {
    char cmdline[1];
};

struct atag {
    struct atag_header hdr;
    union {
        struct atag_core      core;
        struct atag_mem       mem;
        struct atag_mem64     mem64;
        struct atag_serialnr  serialnr;
        struct atag_revision  revision;
        struct atag_videotext videotext;
        struct atag_ramdisk   ramdisk;
        struct atag_initrd2   initrd2;
        struct atag_videolfb  videolfb;
        struct atag_cmdline   cmdline;
    } u;
};

/* Iterator */
#define atag_next(t)    ((struct atag *)((uint32_t *)(t) + (t)->hdr.size))
#define atag_size(type) ((sizeof(struct atag_header) + sizeof(struct type)) >> 2)

/* x86 boot_params compatibility */
struct x86_boot_params {
    uint8_t  setup_sects;
    uint16_t root_flags;
    uint32_t syssize;
    uint16_t ram_size;
    uint16_t vid_mode;
    uint16_t root_dev;
    uint16_t boot_flag;
    uint16_t jump;
    uint32_t header;
    uint16_t version;
    uint32_t realmode_swtch;
    uint16_t start_sys_seg;
    uint16_t kernel_version;
    uint8_t  type_of_loader;
    uint8_t  loadflags;
    uint16_t setup_move_size;
    uint32_t code32_start;
    uint32_t ramdisk_image;
    uint32_t ramdisk_size;
    uint32_t bootsect_kludge;
    uint16_t heap_end_ptr;
    uint8_t  ext_loader_ver;
    uint8_t  ext_loader_type;
    uint32_t cmd_line_ptr;
    uint32_t initrd_addr_max;
    uint32_t kernel_alignment;
    uint8_t  relocatable_kernel;
    uint8_t  min_alignment;
    uint16_t xloadflags;
    uint32_t cmdline_size;
    uint32_t hardware_subarch;
    uint64_t hardware_subarch_data;
    uint32_t payload_offset;
    uint32_t payload_length;
    uint64_t setup_data;
    uint64_t pref_address;
    uint32_t init_size;
    uint32_t handover_offset;
    uint32_t kernel_info_offset;
    
    /* Our extensions */
    uint64_t atags_phys;
    uint32_t atags_size;
} __attribute__((packed));

extern struct x86_boot_params boot_params;

void atags_parse(void *atags_phys);
const char *atags_get_cmdline(void);
uint64_t atags_get_mem_start(void);
uint64_t atags_get_mem_size(void);

#endif /* _LINUXAB_ATAGS_H */
