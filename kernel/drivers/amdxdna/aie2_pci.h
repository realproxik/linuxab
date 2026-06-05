/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2024-2026 Advanced Micro Devices, Inc.
 *
 * AMD XDNA AIE2 PCI header
 */

#ifndef __AIE2_PCI_H__
#define __AIE2_PCI_H__

#include "amdxdna_pci_drv.h"

/* AIE2 register offsets */
#define AIE2_MPNPU_PWAITMODE		0x100
#define AIE2_MPNPU_PSTATE		0x104
#define AIE2_MPNPU_DPM_STATUS		0x108
#define AIE2_MPNPU_FW_STATUS		0x10C

/* AIE2 firmware states */
enum aie2_fw_state {
	AIE2_FW_STATE_UNKNOWN = 0,
	AIE2_FW_STATE_OFF,
	AIE2_FW_STATE_BOOT,
	AIE2_FW_STATE_READY,
	AIE2_FW_STATE_RUNNING,
	AIE2_FW_STATE_SUSPENDED,
	AIE2_FW_STATE_ERROR,
};

/* Debug info for hardware context */
struct aie2_debug_info {
	u32	hwctx_id;
	u32	state;
	u32	cmd_count;
	u32	pending_cmds;
	u32	completed_cmds;
	u64	last_error;
	u32	fw_debug_val;
};

struct aie2_pci_dev {
	struct amdxdna_pci_dev		base;
	
	/* MMIO bases */
	void __iomem			*mpnpu_base;
	void __iomem			*apu_base;
	void __iomem			*psm_base;
	
	/* Firmware state */
	enum aie2_fw_state		fw_state;
	
	/* Subsystems */
	struct aie2_psp			*psp;
	struct aie2_smu			*smu;
	struct aie2_pm			*pm;
	struct aie2_error_info		*error;
	struct aie2_debug_info		*debug;
	
	/* Power management */
	u32				dpm_level;
	u32				dpm_level_saved;
	u32				dpm_table[8];
	u32				dpm_count;
	
	/* Hardware contexts */
	struct amdxdna_hwctx		*hwctx_table[AMDXDNA_MAX_HWCTX_PER_DEV];
	u32				hwctx_count;
	u32				max_hwctx;
	u32				max_opc;
};

int aie2_pci_init(struct amdxdna_dev *xdna);
void aie2_pci_fini(struct amdxdna_dev *xdna);

int aie2_request_fw_suspend(struct amdxdna_dev *xdna);
int aie2_request_fw_resume(struct amdxdna_dev *xdna);

/* Support retrieving hardware context debug information */
int aie2_get_hwctx_debug_info(struct amdxdna_dev *xdna, u32 hwctx_id,
			       struct aie2_debug_info *info);

int aie2_block_when_iommu_off(struct amdxdna_dev *xdna);

/* Internal helpers */
void aie2_hwctx_stop_all(struct aie2_pci_dev *aie2);

/* Device private data */
const struct amdxdna_dev_priv *aie2_get_dev_priv(u16 device_id);

#endif /* __AIE2_PCI_H__ */
