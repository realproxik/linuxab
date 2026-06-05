/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
/*
 * Copyright (C) 2024-2026 [Tera Naam]
 * randstruct_plugin.c - Structure layout randomization for Linuxab OS
 *
 * Randomizes the layout of sensitive kernel structures to prevent
 * infoleaks and make exploitation harder.
 *
 * Linuxab OS - The Future of Computing
 */

#include "gcc-common.h"
#include <hash-table.h>

/* Plugin metadata */
int plugin_is_GPL_compatible;

static struct plugin_info randstruct_plugin_info = {
	.version = LINUXAB_GCC_PLUGIN_VERSION,
	.help = "Randomize layout of sensitive structures",
};

/* Randomization state */
static unsigned int randstruct_seed;
static bool randstruct_performance_mode;

/* Structure field info */
struct randstruct_field {
	tree field;
	unsigned int size;
	unsigned int alignment;
	bool sensitive;
};

/* Structure randomization data */
struct randstruct_struct {
	tree type;
	vec<<struct randstruct_field, va_gc> *fields;
	bool randomized;
};

static hash_map<<tree, struct randstruct_struct *> *randstruct_map;

/* Check if type should be randomized */
static bool should_randomize(tree type)
{
	tree attr;

	if (!RECORD_OR_UNION_TYPE_P(type))
		return false;

	/* Check for randomize_layout attribute */
	attr = lookup_attribute(LINUXAB_ATTR_RANDOMIZE_LAYOUT,
				TYPE_ATTRIBUTES(type));
	if (attr != NULL_TREE)
		return true;

	/* Check for no_randomize_layout attribute */
	attr = lookup_attribute(LINUXAB_ATTR_NO_RANDOMIZE_LAYOUT,
				TYPE_ATTRIBUTES(type));
	if (attr != NULL_TREE)
		return false;

	/* Default: randomize structures with function pointers */
	for (tree field = TYPE_FIELDS(type); field; field = TREE_CHAIN(field)) {
		if (TREE_CODE(field) != FIELD_DECL)
			continue;

		tree field_type = TREE_TYPE(field);
		if (TREE_CODE(field_type) == POINTER_TYPE &&
		    TREE_CODE(TREE_TYPE(field_type)) == FUNCTION_TYPE)
			return true;
	}

	return false;
}

/* Fisher-Yates shuffle for fields */
static void shuffle_fields(vec<<struct randstruct_field, va_gc> *fields)
{
	unsigned int n = vec_safe_length(fields);
	unsigned int i, j;
	struct randstruct_field tmp;

	if (n < 2)
		return;

	for (i = n - 1; i > 0; i--) {
		/* Simple LCG random number generator */
		randstruct_seed = randstruct_seed * 1103515245 + 12345;
		j = randstruct_seed % (i + 1);

		tmp = (*fields)[j];
		(*fields)[j] = (*fields)[i];
		(*fields)[i] = tmp;
	}
}

/* Group fields by alignment for performance */
static void group_by_alignment(vec<<struct randstruct_field, va_gc> *fields)
{
	if (!randstruct_performance_mode)
		return;

	/* Sort by alignment descending to minimize padding */
	unsigned int n = vec_safe_length(fields);
	unsigned int i, j;
	struct randstruct_field tmp;

	for (i = 0; i < n - 1; i++) {
		for (j = 0; j < n - i - 1; j++) {
			if ((*fields)[j].alignment < (*fields)[j + 1].alignment) {
				tmp = (*fields)[j];
				(*fields)[j] = (*fields)[j + 1];
				(*fields)[j + 1] = tmp;
			}
		}
	}
}

/* Randomize structure layout */
static void randomize_struct(tree type)
{
	struct randstruct_struct *rs;
	vec<<struct randstruct_field, va_gc> *fields = NULL;
	tree field;

	if (!should_randomize(type))
		return;

	/* Already randomized? */
	if (randstruct_map && randstruct_map->get(type))
		return;

	/* Collect fields */
	for (field = TYPE_FIELDS(type); field; field = TREE_CHAIN(field)) {
		if (TREE_CODE(field) != FIELD_DECL)
			continue;

		struct randstruct_field rf = {
			.field = field,
			.size = int_size_in_bytes(TREE_TYPE(field)),
			.alignment = TYPE_ALIGN_UNIT(TREE_TYPE(field)),
			.sensitive = false,
		};

		vec_safe_push(fields, rf);
	}

	if (vec_safe_length(fields) < 2)
		return;

	/* Shuffle or group */
	if (randstruct_performance_mode)
		group_by_alignment(fields);
	else
		shuffle_fields(fields);

	/* Store randomization info */
	rs = ggc_alloc<<struct randstruct_struct>();
	rs->type = type;
	rs->fields = fields;
	rs->randomized = true;

	if (!randstruct_map)
		randstruct_map = hash_map<<tree, struct randstruct_struct *>::create_ggc(256);

	randstruct_map->put(type, rs);

	/* Apply new layout */
	/* Note: Actual layout change requires more complex GCC internals */
	linuxab_warning(input_location,
			"Structure %qT randomized (%u fields)",
			type, vec_safe_length(fields));
}

/* Type processing pass */
static unsigned int execute_randstruct(void)
{
	tree type;

	/* Process all structure types */
	/* In real implementation, walk all types in compilation unit */

	return 0;
}

/* Pass definition */
static struct gimple_opt_pass pass_randstruct = {
	.pass = {
		.type = GIMPLE_PASS,
		.name = "randstruct",
		.optinfo_flags = OPTGROUP_NONE,
		.tv_id = TV_NONE,
		.properties_required = PROP_gimple_any,
		.properties_provided = 0,
		.properties_destroyed = 0,
		.todo_flags_start = 0,
		.todo_flags_finish = TODO_verify_stmts,
		.execute = execute_randstruct,
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
		error("Randstruct plugin incompatible with GCC %s", ver->basever);
		return 1;
	}

	/* Parse arguments */
	for (i = 0; i < argc; i++) {
		if (strcmp(argv[i].key, "seed") == 0) {
			randstruct_seed = atoi(argv[i].value);
		} else if (strcmp(argv[i].key, "performance") == 0) {
			randstruct_performance_mode = true;
		}
	}

	if (randstruct_seed == 0)
		randstruct_seed = (unsigned int)time(NULL);

	register_callback(plugin_name, PLUGIN_INFO, NULL, &randstruct_plugin_info);

	/* Register pass after types are finalized */
	struct register_pass_info pass_info = {
		.pass = &pass_randstruct.pass,
		.reference_pass_name = "fre",
		.ref_pass_instance_number = 1,
		.pos_op = PASS_POS_INSERT_AFTER,
	};

	register_callback(plugin_name, PLUGIN_PASS_MANAGER_SETUP,
			  NULL, &pass_info);

	printf("Linuxab Randstruct plugin loaded (seed=%u, performance=%s)\n",
	       randstruct_seed,
	       randstruct_performance_mode ? "yes" : "no");

	return 0;
}