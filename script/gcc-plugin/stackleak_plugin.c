/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
/*
 * Copyright (C) 2024-2026 [Tera Naam]
 * stackleak_plugin.c - Stack erasure plugin for Linuxab OS
 *
 * Erases kernel stack before returning from syscalls to prevent
 * information leaks and uninitialized variable attacks.
 *
 * Linuxab OS - The Future of Computing
 */

#include "gcc-common.h"

/* Plugin metadata */
int plugin_is_GPL_compatible;

static struct plugin_info stackleak_plugin_info = {
	.version = LINUXAB_GCC_PLUGIN_VERSION,
	.help = "Erase kernel stack before return from syscalls",
};

/* Stack depth tracking */
static GTY(()) tree stackleak_track_decl;
static GTY(()) tree stackleak_erase_decl;
static GTY(()) tree stackleak_check_decl;

/* Attribute handling */
static tree stackleak_handle_attribute(tree *node, tree name,
					 tree args, int flags,
					 bool *no_add_attrs)
{
	if (TREE_CODE(*node) != FUNCTION_DECL) {
		warning(*no_add_attrs ? OPT_Wattributes : 0,
			"%qE attribute only applies to functions", name);
		*no_add_attrs = true;
		return NULL_TREE;
	}

	return NULL_TREE;
}

static struct attribute_spec stackleak_attr = {
	.name = LINUXAB_ATTR_STACKLEAK_TRACK,
	.min_length = 0,
	.max_length = 0,
	.decl_required = true,
	.type_required = false,
	.function_type_required = false,
	.handler = stackleak_handle_attribute,
	.affects_type_identity = false,
};

/* Register attribute */
static void register_stackleak_attribute(void)
{
	register_attribute(&stackleak_attr);
}

/* Stack erasure GIMPLE pass */
static unsigned int execute_stackleak_erase(void)
{
	basic_block bb;
	gimple_stmt_iterator gsi;
	tree erase_fn;

	/* Find return statements and insert stack erase */
	FOR_EACH_BB_FN(bb, cfun) {
		for (gsi = gsi_start_bb(bb); !gsi_end_p(gsi); gsi_next(&gsi)) {
			gimple *stmt = gsi_stmt(gsi);

			if (gimple_is_return(stmt)) {
				/* Insert stack erase before return */
				gcall *call = gimple_build_call(erase_fn, 0);
				gimple_set_location(call, gimple_location(stmt));
				gsi_insert_before(&gsi, call, GSI_SAME_STMT);
			}
		}
	}

	return 0;
}

/* Pass definition */
static struct gimple_opt_pass pass_stackleak_erase = {
	.pass = {
		.type = GIMPLE_PASS,
		.name = "stackleak_erase",
		.optinfo_flags = OPTGROUP_NONE,
		.tv_id = TV_NONE,
		.properties_required = PROP_gimple_any,
		.properties_provided = 0,
		.properties_destroyed = 0,
		.todo_flags_start = 0,
		.todo_flags_finish = TODO_verify_stmts,
		.execute = execute_stackleak_erase,
	},
};

/* Plugin initialization */
int plugin_init(struct plugin_name_args *info,
		struct plugin_gcc_version *ver)
{
	const char *plugin_name = info->base_name;

	if (!plugin_default_version_check(ver, &gcc_version)) {
		error("Stackleak plugin incompatible with GCC %s", ver->basever);
		return 1;
	}

	register_callback(plugin_name, PLUGIN_INFO, NULL, &stackleak_plugin_info);
	register_callback(plugin_name, PLUGIN_ATTRIBUTES,
			  register_stackleak_attribute, NULL);

	/* Register pass after stack frame allocation */
	struct register_pass_info pass_info = {
		.pass = &pass_stackleak_erase.pass,
		.reference_pass_name = "cfg",
		.ref_pass_instance_number = 1,
		.pos_op = PASS_POS_INSERT_AFTER,
	};

	register_callback(plugin_name, PLUGIN_PASS_MANAGER_SETUP,
			  NULL, &pass_info);

	printf("Linuxab Stackleak plugin loaded (GCC %s)\n", ver->basever);

	return 0;
}