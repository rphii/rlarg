#ifndef ARGX_SO_H

#include <rlso.h>
#include <rlc.h>

typedef struct Argx Argx;

typedef struct Argx_So {
    Argx *argx;
    So set_val;
    So set_ref;
    So hint;
    So hierarchy;
    bool val_visible;
    bool val_config;
    bool val_group;     /* if the set_val and set_ref are part of a group = require further expanding for a config */
    bool have_hint;
} Argx_So;

void argx_so_free(Argx_So *xso);
void argx_so_clear(Argx_So *xso);
void argx_so(Argx_So *xso, bool fx, Argx *argx);

#define ARGX_SO_H
#endif /* ARGX_SO_H */

