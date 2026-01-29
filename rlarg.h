#ifndef RLARG_H

#include <rlso.h>
#include "rlarg/argx-hint.h"

/* rlarg/arg-core.c */
struct Arg *arg_new(void);
void arg_help(struct Arg *arg);
void arg_config(struct Arg *arg);
void arg_free(struct Arg **arg);

/* rlarg/arg-parse.c */
int arg_parse(struct Arg *arg, const int argc, const char **argv);

/* rlarg/argx-group.c */
struct Argx_Group *argx_group(struct Arg *arg, So name);

/* rlarg/argx.c */
struct Argx *argx_opt(struct Argx_Group *group, char c, So name, So desc);
struct Argx *argx_pos(struct Arg *arg, So name, So desc);
struct Argx *argx_env(struct Arg *arg, So name, So desc);

void argx_builtin_env_compgen(struct Arg *arg);
void argx_builtin_opt_help(struct Argx_Group *group);
void argx_builtin_opt_source(struct Argx_Group *group, So uri);

/* rlarg/argx-type.c */

void argx_type_rest(struct Argx *argx, VSo *val);
void argx_type_so(struct Argx *argx, So *val, So *ref);
void argx_type_uri(struct Argx *argx, So *val, So *ref);
void argx_type_bool(struct Argx *argx, bool *val, bool *ref);
void argx_type_int(struct Argx *argx, int *val, int *ref);
void argx_type_size(struct Argx *argx, ssize_t *val, ssize_t *ref);

void argx_type_array_so(struct Argx *argx, VSo *val, VSo *ref);
void argx_type_array_uri(struct Argx *argx, VSo *val, VSo *ref);
void argx_type_array_bool(struct Argx *argx, bool **val, bool **ref);
void argx_type_array_int(struct Argx *argx, int **val, int **ref);
void argx_type_array_size(struct Argx *argx, ssize_t **val, ssize_t **ref);

struct Argx_Group *argx_group_enum(struct Argx *argx, int *val, int *ref);
struct Argx_Group *argx_group_options(struct Argx *argx);
struct Argx_Group *argx_group_flags(struct Argx *argx);

struct Argx *argx_enum_bind(struct Argx_Group *group, int val, So name, So desc);
struct Argx *argx_flag(struct Argx_Group *group, bool *val, bool *ref, So name, So desc);

void argx_hint_kind(struct Argx *argx, Argx_Hint_List id);
void argx_hint_text(struct Argx *argx, So text);

#define RLARG_H
#endif /* RLARG_H */

