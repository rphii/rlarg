#ifndef ARGX_ATTR_H

#include <stdbool.h>
#include <rlso.h>

typedef struct Argx_Attr {
    bool is_array;
    bool is_hidden;
    bool is_unconfigurable;
    int val_enum;
} Argx_Attr;

#define ARGX_ATTR_H
#endif /* ARGX_ATTR_H */

