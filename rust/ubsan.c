
// SPDX-License-Identifier: GPL-2.0
/*
 * linuxab UBSAN implementation
 */

#include "ubsan.h"
#include "printk.h"

static void ubsan_report(const char *reason, struct ubsan_source_location *loc)
{
    printk(KERN_EMERG "=================================================================\\n");
    printk(KERN_EMERG "UBSAN: %s in %s:%u:%u\\n", reason,
           loc->file ? loc->file : "<unknown>",
           loc->line, loc->column);
    printk(KERN_EMERG "=================================================================\\n");
    
    /* TODO: Print stack trace */
    /* For now, just warn and continue. Can be changed to panic. */
}

static void ubsan_report_val(const char *reason, struct ubsan_source_location *loc,
                             struct ubsan_type_descriptor *type, void *val)
{
    printk(KERN_EMERG "UBSAN: %s in %s:%u:%u\\n", reason,
           loc->file ? loc->file : "<unknown>", loc->line, loc->column);
    printk(KERN_EMERG "Type: %s\\n", type->type_name);
    /* TODO: Print value based on type_kind */
}

void __ubsan_handle_add_overflow(struct ubsan_overflow_data *data, void *lhs, void *rhs)
{
    (void)lhs; (void)rhs;
    ubsan_report("addition overflow", &data->location);
}

void __ubsan_handle_sub_overflow(struct ubsan_overflow_data *data, void *lhs, void *rhs)
{
    (void)lhs; (void)rhs;
    ubsan_report("subtraction overflow", &data->location);
}

void __ubsan_handle_mul_overflow(struct ubsan_overflow_data *data, void *lhs, void *rhs)
{
    (void)lhs; (void)rhs;
    ubsan_report("multiplication overflow", &data->location);
}

void __ubsan_handle_divrem_overflow(struct ubsan_overflow_data *data, void *lhs, void *rhs)
{
    (void)lhs; (void)rhs;
    ubsan_report("division remainder overflow", &data->location);
}

void __ubsan_handle_negate_overflow(struct ubsan_overflow_data *data, void *old_val)
{
    (void)old_val;
    ubsan_report("negation overflow", &data->location);
}

void __ubsan_handle_shift_out_of_bounds(struct ubsan_shift_out_of_bounds_data *data,
                                        void *lhs, void *rhs)
{
    (void)lhs; (void)rhs;
    ubsan_report("shift out of bounds", &data->location);
}

void __ubsan_handle_load_invalid_value(struct ubsan_invalid_value_data *data, void *val)
{
    (void)val;
    ubsan_report("load of invalid value", &data->location);
}

void __ubsan_handle_out_of_bounds(struct ubsan_array_out_of_bounds_data *data, void *index)
{
    (void)index;
    ubsan_report("array subscript out of bounds", &data->location);
}

void __ubsan_handle_type_mismatch_v1(void *data_raw, void *pointer, uint8_t log_alignment)
{
    struct {
        struct ubsan_source_location loc;
        struct ubsan_type_descriptor *type;
        uint8_t log_alignment;
        uint8_t type_check_kind;
    } *data = data_raw;
    
    (void)log_alignment;
    (void)pointer;
    const char *kind_str = "type mismatch";
    switch (data->type_check_kind) {
        case 0: kind_str = "load of null pointer"; break;
        case 1: kind_str = "store to null pointer"; break;
        case 2: kind_str = "reference binding to null"; break;
        case 3: kind_str = "member access on null pointer"; break;
        case 4: kind_str = "member call on null pointer"; break;
        case 5: kind_str = "constructor call on null pointer"; break;
        case 6: kind_str = "downcast on null pointer"; break;
        case 7: kind_str = "invalid vptr"; break;
    }
    ubsan_report(kind_str, &data->loc);
}

void __ubsan_handle_vla_bound_not_positive(struct ubsan_vla_bound_data *data, void *bound)
{
    (void)bound;
    ubsan_report("VLA bound not positive", &data->location);
}

void __ubsan_handle_nonnull_return(struct ubsan_nonnull_return_data *data, void *pointer)
{
    (void)pointer;
    ubsan_report("null pointer return", &data->attr_location);
}

void __ubsan_handle_nullability_return(struct ubsan_nullability_return_data *data, void *pointer)
{
    (void)pointer;
    ubsan_report("null pointer return (nullability)", &data->location);
}

void __ubsan_handle_nonnull_arg(struct ubsan_nonnull_return_data *data)
{
    ubsan_report("null pointer passed as argument", &data->attr_location);
}

void __ubsan_handle_nullability_arg(struct ubsan_nullability_arg_data *data, void *arg)
{
    (void)arg;
    ubsan_report("null pointer passed as argument (nullability)", &data->location);
}

void __ubsan_handle_alignment_assumption(void *data_raw, void *pointer, void *offset, void *mask)
{
    struct {
        struct ubsan_source_location loc;
        struct ubsan_type_descriptor *type;
        uint8_t alignment;
        uint8_t check_kind;
    } *data = data_raw;
    (void)pointer; (void)offset; (void)mask;
    ubsan_report("alignment assumption violated", &data->loc);
}

void __ubsan_handle_builtin_unreachable(struct ubsan_source_location *location)
{
    ubsan_report("unreachable code reached", location);
    while (1) __asm__ volatile ("cli; hlt");
}

void __ubsan_handle_missing_return(struct ubsan_source_location *location)
{
    ubsan_report("missing return statement", location);
    while (1) __asm__ volatile ("cli; hlt");
}