#ifndef ARGX_ATTR_H

#include <stdbool.h>
#include <rlso.h>

typedef struct Argx_Attr {
    bool is_array;
    bool is_hidden;
    bool is_unconfigurable;
    bool is_required;
    bool is_explicit_bool;
    bool is_fatal_config_error;
    bool callback_skip_compgen;
    int val_enum;
    So switch_arg;
} Argx_Attr;

struct Argx;

bool argx_attr_is_explicit_bool(struct Argx *argx);

#define ARGX_ATTR_H
#endif /* ARGX_ATTR_H */

