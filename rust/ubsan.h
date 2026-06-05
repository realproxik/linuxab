// SPDX-License-Identifier: GPL-2.0
/*
 * linuxab UBSAN (Undefined Behavior Sanitizer)
 */

#ifndef _LINUXAB_UBSAN_H
#define _LINUXAB_UBSAN_H

#include <stdint.h>

/* UBSAN data types */
struct ubsan_source_location {
    const char *file;
    uint32_t line;
    uint32_t column;
};

struct ubsan_type_descriptor {
    uint16_t type_kind;
    uint16_t type_info;
    char type_name[];
};

struct ubsan_overflow_data {
    struct ubsan_source_location location;
    struct ubsan_type_descriptor *type;
};

struct ubsan_shift_out_of_bounds_data {
    struct ubsan_source_location location;
    struct ubsan_type_descriptor *lhs_type;
    struct ubsan_type_descriptor *rhs_type;
};

struct ubsan_invalid_value_data {
    struct ubsan_source_location location;
    struct ubsan_type_descriptor *type;
};

struct ubsan_array_out_of_bounds_data {
    struct ubsan_source_location location;
    struct ubsan_type_descriptor *array_type;
    struct ubsan_type_descriptor *index_type;
};

struct ubsan_nullability_arg_data {
    struct ubsan_source_location location;
    struct ubsan_type_descriptor *type;
    uint8_t arg_index;
};

struct ubsan_nullability_return_data {
    struct ubsan_source_location location;
    struct ubsan_type_descriptor *type;
};

struct ubsan_nonnull_return_data {
    struct ubsan_source_location attr_location;
};

struct ubsan_vla_bound_data {
    struct ubsan_source_location location;
    struct ubsan_type_descriptor *type;
};

struct ubsan_alignment_data {
    struct ubsan_source_location location;
    struct ubsan_type_descriptor *type;
    uint8_t alignment;
    uint8_t check_kind;
};

struct ubsan_object_size_data {
    struct ubsan_source_location location;
    struct ubsan_type_descriptor *type;
    uint8_t check_kind;
};

/* Handler prototypes called by compiler instrumentation */
void __ubsan_handle_add_overflow(struct ubsan_overflow_data *data, void *lhs, void *rhs);
void __ubsan_handle_sub_overflow(struct ubsan_overflow_data *data, void *lhs, void *rhs);
void __ubsan_handle_mul_overflow(struct ubsan_overflow_data *data, void *lhs, void *rhs);
void __ubsan_handle_divrem_overflow(struct ubsan_overflow_data *data, void *lhs, void *rhs);
void __ubsan_handle_negate_overflow(struct ubsan_overflow_data *data, void *old_val);
void __ubsan_handle_shift_out_of_bounds(struct ubsan_shift_out_of_bounds_data *data, void *lhs, void *rhs);
void __ubsan_handle_load_invalid_value(struct ubsan_invalid_value_data *data, void *val);
void __ubsan_handle_out_of_bounds(struct ubsan_array_out_of_bounds_data *data, void *index);
void __ubsan_handle_type_mismatch_v1(void *data, void *pointer, uint8_t log_alignment);
void __ubsan_handle_vla_bound_not_positive(struct ubsan_vla_bound_data *data, void *bound);
void __ubsan_handle_nonnull_return(struct ubsan_nonnull_return_data *data, void *pointer);
void __ubsan_handle_nullability_return(struct ubsan_nullability_return_data *data, void *pointer);
void __ubsan_handle_nonnull_arg(struct ubsan_nonnull_return_data *data);
void __ubsan_handle_nullability_arg(struct ubsan_nullability_arg_data *data, void *arg);
void __ubsan_handle_alignment_assumption(void *data, void *pointer, void *offset, void *mask);

void __ubsan_handle_builtin_unreachable(struct ubsan_source_location *location);
void __ubsan_handle_missing_return(struct ubsan_source_location *location);

#endif /* _LINUXAB_UBSAN_H */
