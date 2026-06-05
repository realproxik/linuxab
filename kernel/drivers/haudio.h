/* SPDX-License-Identifier: GPL-2.0 */
/*
 * linuxab/drivers/sound/hdaudio.h
 * Intel HD Audio (Azalia) driver header
 */

#ifndef _LINUXAB_HDAUDIO_H
#define _LINUXAB_HDAUDIO_H

#include <stdint.h>
#include <stdbool.h>

/* PCI class */
#define PCI_CLASS_MULTIMEDIA_HDA    0x0403

/* HDA registers (memory mapped) */
#define HDA_REG_GCAP        0x00    /* Global Capabilities */
#define HDA_REG_VMIN        0x02
#define HDA_REG_VMAJ        0x03
#define HDA_REG_GCTL        0x08    /* Global Control */
#define HDA_REG_WAKEEN      0x0C
#define HDA_REG_STATESTS    0x0E    /* State Change Status */
#define HDA_REG_GSTS        0x10
#define HDA_REG_OUTPAY      0x18
#define HDA_REG_INPAY       0x1A
#define HDA_REG_CORBLBASE   0x40    /* CORB lower base */
#define HDA_REG_CORBUBASE   0x44    /* CORB upper base */
#define HDA_REG_CORBWP      0x48    /* CORB write pointer */
#define HDA_REG_CORBRP      0x4A    /* CORB read pointer */
#define HDA_REG_CORBCTL     0x4C    /* CORB control */
#define HDA_REG_CORBST      0x4D    /* CORB status */
#define HDA_REG_CORBSIZE    0x4E    /* CORB size */
#define HDA_REG_RIRBLBASE   0x50    /* RIRB lower base */
#define HDA_REG_RIRBUBASE   0x54    /* RIRB upper base */
#define HDA_REG_RIRBWP      0x58    /* RIRB write pointer */
#define HDA_REG_RINTCNT     0x5A
#define HDA_REG_RIRBCTL     0x5C    /* RIRB control */
#define HDA_REG_RIRBSTS     0x5D    /* RIRB status */
#define HDA_REG_RIRBSIZE    0x5E    /* RIRB size */
#define HDA_REG_IC          0x60    /* Immediate Command */
#define HDA_REG_IR          0x64    /* Immediate Response */
#define HDA_REG_IRS         0x68    /* Immediate Command Status */
#define HDA_REG_DPLBASE     0x70    /* DMA Position Lower Base */
#define HDA_REG_DPUBASE     0x74    /* DMA Position Upper Base */

/* Stream registers offset from SD_BASE */
#define HDA_REG_SD_CTL      0x00
#define HDA_REG_SD_STS      0x03
#define HDA_REG_SD_LPIB     0x04
#define HDA_REG_SD_CBL      0x08
#define HDA_REG_SD_LVI      0x0C
#define HDA_REG_SD_FIFOW    0x0E
#define HDA_REG_SD_FIFOS    0x10
#define HDA_REG_SD_FMT      0x12
#define HDA_REG_SD_BDLPL    0x18
#define HDA_REG_SD_BDLPU    0x1C

#define HDA_GCTL_CRST       0x00000001

#define HDA_CORBCTL_RUN     0x02
#define HDA_RIRBCTL_RUN     0x02
#define HDA_RIRBCTL_RINTCTL 0x01

#define HDA_SD_CTL_SRST     0x00000001
#define HDA_SD_CTL_RUN      0x00000002
#define HDA_SD_CTL_IOCE     0x00000004
#define HDA_SD_CTL_FEIE     0x00000008
#define HDA_SD_CTL_DEIE     0x00000010
#define HDA_SD_CTL_STRIPE   0x00C00000
#define HDA_SD_CTL_TP       0x01000000
#define HDA_SD_CTL_DIR      0x02000000
#define HDA_SD_CTL_STREAM_SHIFT   20

#define HDA_SD_FMT_BASE_44_1    0x0
#define HDA_SD_FMT_BASE_48      0x1
#define HDA_SD_FMT_MULT_1X      0x0
#define HDA_SD_FMT_DIV_1X       0x0
#define HDA_SD_FMT_BITS_16      0x1
#define HDA_SD_FMT_BITS_32      0x4

/* Codec command */
#define HDA_VERB_GET_PARAMETER      0xF00
#define HDA_VERB_GET_CONFIG_DEFAULT 0xF1C
#define HDA_VERB_SET_STREAM_CHANNEL 0x706
