/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
/*
 * Copyright (C) 2024-2026 [Tera Naam]
 * gcc-common.h - Common definitions for Linuxab GCC plugins
 *
 * Linuxab OS - The Future of Computing
 */

#ifndef _GCC_COMMON_H
#define _GCC_COMMON_H

#include <gcc-plugin.h>
#include <plugin-version.h>
#include <tree.h>
#include <gimple.h>
#include <gimple-iterator.h>
#include <gimple-pretty-print.h>
#include <tree-iterator.h>
#include <stringpool.h>
#include <cgraph.h>
#include <ipa-ref.h>
#include <print-tree.h>
#include <diagnostic.h>
#include <context.h>
#include <tree-pass.h>
#include <pass_manager.h>
#include <attribs.h>
#include <function.h>
#include <basic-block.h>
#include <coretypes.h>
#include <langhooks.h>
#include <tree-pass.h>
#include <intl.h>
#include <params.h>
#include <options.h>
#include <emit-rtl.h>
#include <memmodel.h>
#include <tm_p.h>
#include <hash-set.h>
#include <vec.h>
#include <machmode.h>
#include <hash-map.h>
#include <input.h>
#include <is-a.h>
#include <plugin-api.h>

/* GCC version compatibility */
#if BUILDING_GCC_VERSION >= 4008
#include <predict.h>
#endif

#if BUILDING_GCC_VERSION >= 5000
#include <gimple-expr.h>
#endif

#if BUILDING_GCC_VERSION >= 6000
#include <tree-cfgcleanup.h>
#include <tree-into-ssa.h>
#endif

#if BUILDING_GCC_VERSION >= 7000
#include <gimple-fold.h>
#include <tree-vrp.h>
#endif

#if BUILDING_GCC_VERSION >= 8000
#include <gimple-ssa.h>
#include <tree-dfa.h>
#endif

#if BUILDING_GCC_VERSION >= 9000
#include <gimple-range.h>
#endif

#if BUILDING_GCC_VERSION >= 10000
#include <gimple-predict.h>
#endif

#if BUILDING_GCC_VERSION >= 11000
#include <gimple-match.h>
#endif

#if BUILDING_GCC_VERSION >= 12000
#include <gimple-ssa-warn-access.h>
#endif

#if BUILDING_GCC_VERSION >= 13000
#include <gimple-ssa-sprintf.h>
#endif

/* Linuxab plugin version */
#define LINUXAB_GCC_PLUGIN_VERSION "1.0.0"

/* Plugin helpers */
#define LINUXAB_PLUGIN_NAME "linuxab-gcc-plugin"
#define LINUXAB_PLUGIN_HELP "Linuxab OS security and analysis plugins"

/* Attribute helpers */
#define LINUXAB_ATTR_RANDOMIZE_LAYOUT "randomize_layout"
#define LINUXAB_ATTR_NO_RANDOMIZE_LAYOUT "no_randomize_layout"
#define LINUXAB_ATTR_STACKLEAK_TRACK "stackleak_track"
#define LINUXAB_ATTR_LATENT_ENTROPY "latent_entropy"

/* Function helpers */
static inline bool is_function(struct tree_node *node)
{
	return node && TREE_CODE(node) == FUNCTION_DECL;
}

static inline const char *get_function_name(struct tree_node *node)
{
	if (!is_function(node))
		return NULL;
	return IDENTIFIER_POINTER(DECL_NAME(node));
}

/* GIMPLE helpers */
static inline bool gimple_is_call(gimple *stmt)
{
	return gimple_code(stmt) == GIMPLE_CALL;
}

static inline bool gimple_is_return(gimple *stmt)
{
	return gimple_code(stmt) == GIMPLE_RETURN;
}

/* Location helpers */
static inline location_t get_location(gimple *stmt)
{
	return gimple_location(stmt);
}

/* Diagnostic helpers */
#define linuxab_warning(loc, fmt, ...) \
	warning_at(loc, 0, "Linuxab: " fmt, ##__VA_ARGS__)

#define linuxab_error(loc, fmt, ...) \
	error_at(loc, "Linuxab: " fmt, ##__VA_ARGS__)

/* Plugin registration macro */
#define LINUXAB_GCC_PLUGIN(plugin_name, plugin_version, plugin_help) \
	int plugin_is_GPL_compatible; \
	static struct plugin_info linuxab_plugin_info = { \
		.version = plugin_version, \
		.help = plugin_help, \
	}; \
	int plugin_init(struct plugin_name_args *info, \
			struct plugin_gcc_version *ver) \
	{ \
		if (!plugin_default_version_check(ver, &gcc_version)) { \
			error("Linuxab plugin %s incompatible with GCC %s", \
			      plugin_name, ver->basever); \
			return 1; \
		} \
		register_callback(plugin_name, PLUGIN_INFO, \
				NULL, &linuxab_plugin_info);
        }
#endif /* _GCC_COMMON_H */