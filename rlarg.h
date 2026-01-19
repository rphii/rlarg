#ifndef RLARG_H

#include <rlso.h>
#include "rlarg/argx-hint.h"

/* rlarg/arg-core.c */
struct Arg *arg_new(void);
void arg_help(struct Arg *arg);

/* rlarg/arg-parse.c */
int arg_parse(struct Arg *arg, const int argc, const char **argv);

/* rlarg/argx.c */
struct Argx_Group *argx_group(struct Arg *arg, So name);
struct Argx *argx(struct Argx_Group *group, char c, So name, So desc);

/* rlarg/argx-type.c */

void argx_type_so(struct Argx *argx, So *val, So *ref);
void argx_type_bool(struct Argx *argx, bool *val, bool *ref);
void argx_type_int(struct Argx *argx, int *val, int *ref);
void argx_type_size(struct Argx *argx, ssize_t *val, ssize_t *ref);

void argx_hint_kind(struct Argx *argx, Argx_Hint_List id);
void argx_hint_text(struct Argx *argx, So text);

#define RLARG_H
#endif /* RLARG_H */

