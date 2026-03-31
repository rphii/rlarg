#ifndef ARGX_ATTR_H

#include <stdbool.h>
#include <rlso.h>

typedef struct Argx_Attr {
    bool is_array;
    bool is_hidden;
    bool is_unconfigurable;
    bool is_explicit_bool;
    bool callback_skip_compgen;
    int val_enum;
    So switch_arg;
} Argx_Attr;

#define ARGX_ATTR_H
#endif /* ARGX_ATTR_H */

