/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
/*
 * Copyright (C) 2024-2026 [Tera Naam]
 * structleak_plugin.c - Variable initialization plugin for Linuxab OS
 *
 * Forces initialization of variables that might not be explicitly
 * initialized, preventing infoleaks and undefined behavior.
 *
 * Linuxab OS - The Future of Computing
 */

#include "gcc-common.h"

/* Plugin metadata */
int plugin_is_GPL_compatible;

static struct plugin_info structleak_plugin_info = {
	.version = LINUXAB_GCC_PLUGIN_VERSION,
	.help = "Force initialization of potentially uninitialized variables",
};

/* Initialization modes */
enum structleak_mode {
	STRUCTLEAK_NONE = 0,
	STRUCTLEAK_BYREF,      /* Initialize variables passed by reference */
	STRUCTLEAK_BYREF_ALL,  /* Initialize ALL variables passed by reference */
	STRUCTLEAK_ALL,        /* Initialize all variables */
};

static enum structleak_mode structleak_mode = STRUCTLEAK_BYREF;

/* Check if variable needs initialization */
static bool needs_initialization(tree var)
{
	tree type = TREE_TYPE(var);

	/* Already initialized? */
	if (DECL_INITIAL(var) != NULL_TREE)
		return false;

	/* Static/extern variables */
	if (TREE_STATIC(var) || DECL_EXTERNAL(var))
		return false;

	/* By-reference mode: only pointers and structures */
	if (structleak_mode == STRUCTLEAK_BYREF) {
		if (TREE_CODE(type) != POINTER_TYPE &&
		    !AGGREGATE_TYPE_P(type))
			return false;
	}

	/* Byref-all mode: all pointers and aggregates */
	if (structleak_mode == STRUCTLEAK_BYREF_ALL) {
		if (TREE_CODE(type) != POINTER_TYPE &&
		    !AGGREGATE_TYPE_P(type))
			return false;
	}

	return true;
}

/* Insert zero initialization */
static void insert_initialization(tree var, gimple_stmt_iterator *gsi)
{
	tree type = TREE_TYPE(var);
	gassign *init_stmt;

	/* Build zero initializer based on type */
	tree zero;

	switch (TREE_CODE(type)) {
	case INTEGER_TYPE:
		zero = build_int_cst(type, 0);
		break;
	case POINTER_TYPE:
		zero = build_int_cst(type, 0);
		break;
	case REAL_TYPE:
		zero = build_real_from_int_cst(type, build_int_cst(NULL_TREE, 0));
		break;
	case COMPLEX_TYPE:
		zero = build_complex(type,
				     build_int_cst(TREE_TYPE(type), 0),
				     build_int_cst(TREE_TYPE(type), 0));
		break;
	default:
		/* For aggregates, use memset */
		return;
	}

	init_stmt = gimple_build_assign(var, zero);
	gimple_set_location(init_stmt, DECL_SOURCE_LOCATION(var));
	gsi_insert_before(gsi, init_stmt, GSI_SAME_STMT);
}

/* Main pass */
static unsigned int execute_structleak(void)
{
	basic_block bb;
	gimple_stmt_iterator gsi;
	tree var;

	/* Process local variables */
	for (var = DECL_ARGUMENTS(current_function_decl); var; var = DECL_CHAIN(var)) {
		if (needs_initialization(var)) {
			/* Insert at function entry */
			bb = ENTRY_BLOCK_PTR_FOR_FN(cfun);
			if (bb && single_succ_p(bb)) {
				bb = single_succ(bb);
				gsi = gsi_start_bb(bb);
				insert_initialization(var, &gsi);
			}
		}
	}

	/* Process local declarations */
	for (var = gimple_seq_first_stmt(gsi_seq(ENTRY_BLOCK_PTR_FOR_FN(cfun)));
	     var; var = DECL_CHAIN(var)) {
		if (TREE_CODE(var) == VAR_DECL && needs_initialization(var)) {
			/* Find appropriate insertion point */
			FOR_EACH_BB_FN(bb, cfun) {
				for (gsi = gsi_start_bb(bb); !gsi_end_p(gsi); gsi_next(&gsi)) {
					gimple *stmt = gsi_stmt(gsi);
					if (gimple_code(stmt) == GIMPLE_BIND) {
						insert_initialization(var, &gsi);
					}
				}
			}
		}
	}

	return 0;
}

/* Pass definition */
static struct gimple_opt_pass pass_structleak = {
	.pass = {
		.type = GIMPLE_PASS,
		.name = "structleak",
		.optinfo_flags = OPTGROUP_NONE,
		.tv_id = TV_NONE,
		.properties_required = PROP_gimple_any,
		.properties_provided = 0,
		.properties_destroyed = 0,
		.todo_flags_start = 0,
		.todo_flags_finish = TODO_verify_stmts,
		.execute = execute_structleak,
	},
};

/* Plugin initialization */
int plugin_init(struct plugin_name_args *info,
		struct plugin_gcc_version *ver)
{
	const char *plugin_name = info->base_name;
	int argc = info->argc;
	struct plugin_argument *argv = info->argv;
	int i;

	if (!plugin_default_version_check(ver, &gcc_version)) {
		error("Structleak plugin incompatible with GCC %s", ver->basever);
		return 1;
	}

	/* Parse mode */
	for (i = 0; i < argc; i++) {
		if (strcmp(argv[i].key, "mode") == 0) {
			if (strcmp(argv[i].value, "byref") == 0)
				structleak_mode = STRUCTLEAK_BYREF;
			else if (strcmp(argv[i].value, "byref-all") == 0)
				structleak_mode = STRUCTLEAK_BYREF_ALL;
			else if (strcmp(argv[i].value, "all") == 0)
				structleak_mode = STRUCTLEAK_ALL;
		}
	}

	register_callback(plugin_name, PLUGIN_INFO, NULL, &structleak_plugin_info);

	/* Register pass after SSA */
	struct register_pass_info pass_info = {
		.pass = &pass_structleak.pass,
		.reference_pass_name = "ssa",
		.ref_pass_instance_number = 1,
		.pos_op = PASS_POS_INSERT_AFTER,
	};

	register_callback(plugin_name, PLUGIN_PASS_MANAGER_SETUP,
			  NULL, &pass_info);

	printf("Linuxab Structleak plugin loaded (mode=%d)\n", structleak_mode);

	return 0;
}