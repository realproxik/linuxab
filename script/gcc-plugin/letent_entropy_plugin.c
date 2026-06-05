/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
/*
 * Copyright (C) 2024-2026 [Tera Naam]
 * latent_entropy_plugin.c - Latent entropy extraction for Linuxab OS
 *
 * Extracts entropy from program execution flow for use in random seeds.
 * This makes each boot and execution unique.
 *
 * Linuxab OS - The Future of Computing
 */

#include "gcc-common.h"

/* Plugin metadata */
int plugin_is_GPL_compatible;

static struct plugin_info latent_entropy_plugin_info = {
	.version = LINUXAB_GCC_PLUGIN_VERSION,
	.help = "Extract latent entropy from execution flow",
};

/* Entropy accumulator */
static GTY(()) tree latent_entropy_var;
static unsigned long long latent_entropy_bits;

/* Attribute handling */
static tree latent_entropy_handle_attribute(tree *node, tree name,
					     tree args, int flags,
					     bool *no_add_attrs)
{
	if (TREE_CODE(*node) != VAR_DECL) {
		warning(*no_add_attrs ? OPT_Wattributes : 0,
			"%qE attribute only applies to variables", name);
		*no_add_attrs = true;
		return NULL_TREE;
	}

	return NULL_TREE;
}

static struct attribute_spec latent_entropy_attr = {
	.name = LINUXAB_ATTR_LATENT_ENTROPY,
	.min_length = 0,
	.max_length = 0,
	.decl_required = true,
	.type_required = false,
	.function_type_required = false,
	.handler = latent_entropy_handle_attribute,
	.affects_type_identity = false,
};

/* Register attribute */
static void register_latent_entropy_attribute(void)
{
	register_attribute(&latent_entropy_attr);
}

/* Inject entropy extraction at function entry */
static unsigned int execute_latent_entropy(void)
{
	basic_block entry_bb;
	gimple_stmt_iterator gsi;
	tree entropy_fn;
	gcall *call;

	/* Get entry block */
	entry_bb = ENTRY_BLOCK_PTR_FOR_FN(cfun);
	if (!entry_bb || !single_succ_p(entry_bb))
		return 0;

	entry_bb = single_succ(entry_bb);

	/* Insert entropy extraction at function entry */
	gsi = gsi_start_bb(entry_bb);

	/* Build: latent_entropy ^= hash(function_address, call_count) */
	call = gimple_build_call_internal(IFN_LATENT_ENTROPY, 0);
	gimple_set_location(call, DECL_SOURCE_LOCATION(current_function_decl));
	gsi_insert_before(&gsi, call, GSI_SAME_STMT);

	latent_entropy_bits++;

	return 0;
}

/* Pass definition */
static struct gimple_opt_pass pass_latent_entropy = {
	.pass = {
		.type = GIMPLE_PASS,
		.name = "latent_entropy",
		.optinfo_flags = OPTGROUP_NONE,
		.tv_id = TV_NONE,
		.properties_required = PROP_gimple_any,
		.properties_provided = 0,
		.properties_destroyed = 0,
		.todo_flags_start = 0,
		.todo_flags_finish = TODO_verify_stmts,
		.execute = execute_latent_entropy,
	},
};

/* Plugin initialization */
int plugin_init(struct plugin_name_args *info,
		struct plugin_gcc_version *ver)
{
	const char *plugin_name = info->base_name;

	if (!plugin_default_version_check(ver, &gcc_version)) {
		error("Latent entropy plugin incompatible with GCC %s", ver->basever);
		return 1;
	}

	register_callback(plugin_name, PLUGIN_INFO, NULL, &latent_entropy_plugin_info);
	register_callback(plugin_name, PLUGIN_ATTRIBUTES,
			  register_latent_entropy_attribute, NULL);

	/* Register pass at function entry */
	struct register_pass_info pass_info = {
		.pass = &pass_latent_entropy.pass,
		.reference_pass_name = "ssa",
		.ref_pass_instance_number = 1,
		.pos_op = PASS_POS_INSERT_AFTER,
	};

	register_callback(plugin_name, PLUGIN_PASS_MANAGER_SETUP,
			  NULL, &pass_info);

	printf("Linuxab Latent Entropy plugin loaded\n");

	return 0;
}