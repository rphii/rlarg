#ifndef ARGX_SO_H

#include <rlso.h>
#include <rlc.h>

typedef struct Argx Argx;
typedef struct Arg_Rice Arg_Rice;
typedef struct Argx_Group Argx_Group;
typedef union Argx_Value_Union Argx_Value_Union;

typedef struct Argx_So_Options {
    bool force_nocolor;
    bool is_for_config;
    size_t array_max_items;
} Argx_So_Options;

bool argx_so_hint_visible(Argx *argx, Argx_Value_Union *val);
void argx_so_hint(So *out, Arg_Rice *rice, Argx *argx, Argx_Value_Union *val, Argx_So_Options *opts);
void argx_so_val(So *out, Arg_Rice *rice, Argx *argx, Argx_Value_Union *val, Argx_So_Options *opts);
bool argx_so_val_visible(Argx *argx, Argx_Value_Union *val);
bool argx_so_val_config(Argx *argx, Argx_Value_Union *val);
void argx_so_hierarchy(So *hierarchy, Arg_Rice *rice, Argx_Group *group);

#define ARGX_SO_H
#endif /* ARGX_SO_H */

