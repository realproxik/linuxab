# SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note
# Copyright (C) 2024-2026 [Tera Naam]
# Linuxab OS GCC Plugins Makefile

# GCC plugin build system
GCC_PLUGINS_DIR := $(srctree)/scripts/gcc-plugins
GCC_PLUGINS_CFLAGS := -I$(GCC_PLUGINS_DIR) -I$(GCC_PLUGINS_DIR)/include

# Plugin sources
gcc-plugin-y :=

# Core plugins
gcc-plugin-$(CONFIG_GCC_PLUGIN_CYC_COMPLEXITY) += cyc_complexity_plugin.so
gcc-plugin-$(CONFIG_GCC_PLUGIN_LATENT_ENTROPY) += latent_entropy_plugin.so
gcc-plugin-$(CONFIG_GCC_PLUGIN_RANDSTRUCT) += randstruct_plugin.so
gcc-plugin-$(CONFIG_GCC_PLUGIN_STACKLEAK) += stackleak_plugin.so
gcc-plugin-$(CONFIG_GCC_PLUGIN_STRUCTLEAK) += structleak_plugin.so
gcc-plugin-$(CONFIG_GCC_PLUGIN_ARM_SSP_PER_TASK) += arm_ssp_per_task_plugin.so

# Security plugins
gcc-plugin-$(CONFIG_GCC_PLUGIN_KASAN) += kasan_plugin.so
gcc-plugin-$(CONFIG_GCC_PLUGIN_UBSAN) += ubsan_plugin.so
gcc-plugin-$(CONFIG_GCC_PLUGIN_CFI) += cfi_plugin.so
gcc-plugin-$(CONFIG_GCC_PLUGIN_SANCOV) += sancov_plugin.so

# Host compiler settings
HOST_EXTRACFLAGS += -I$(GCC_PLUGINS_DIR)/include

# Plugin build rules
quiet_cmd_gcc_plugin = GCC-PLUGIN $@
      cmd_gcc_plugin = $(HOSTCC) $(HOST_EXTRACFLAGS) $(HOSTCFLAGS) \
                       -shared -fPIC -fno-rtti -O2 \
                       -o $@ $< $(GCC_PLUGINS_CFLAGS)

$(obj)/%.so: $(src)/%.c $(GCC_PLUGINS_DIR)/gcc-common.h
	$(call cmd,gcc_plugin)

# Always build plugins
always-y := $(gcc-plugin-y)

# Clean rules
clean-files := *.so *.o