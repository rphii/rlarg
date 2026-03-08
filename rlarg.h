#ifndef RLARG_H

#include <rlso.h>
#include "rlarg/argx-hint.h"

struct Argx;
struct Argx_Group;

/* rlarg/arg-core.c */
struct Arg *arg_new(void);
void arg_help(struct Arg *arg);
void arg_help_argx_group(struct Argx_Group *group);
void arg_help_argx(struct Argx *help);
void arg_config(struct Arg *arg);
void arg_free(struct Arg **arg);

void arg_enable_config_print(struct Arg *arg, bool enable);

/* rlarg/arg-runtime.c */
void arg_runtime_quit_early(struct Argx *argx, bool val);
void arg_runtime_quit_when_all_parsed(struct Argx *argx, bool val);

/* rlarg/arg-parse.c */
int arg_parse(struct Arg *arg, const int argc, const char **argv, bool *quit_early);
int arg_parse_config(struct Arg *arg, So config, So path);

/* rlarg/argx-group.c */
struct Argx_Group *argx_group(struct Arg *arg, So name);

/* rlarg/argx.c */
struct Argx *argx_opt(struct Argx_Group *group, char c, So name, So desc);
struct Argx *argx_pos(struct Arg *arg, So name, So desc);
struct Argx *argx_env(struct Arg *arg, So name, So desc);

void argx_builtin_env_compgen(struct Arg *arg);
void argx_builtin_opt_color(struct Argx_Group *group);
void argx_builtin_opt_help(struct Argx_Group *group);
void argx_builtin_opt_source(struct Argx_Group *group, So uri);
void argx_builtin_opt_so_fx(struct Argx *x, So_Fx *fmt, So_Fx *ref);
struct Argx_Group *argx_builtin_rice(struct Arg *arg);
void argx_builtin_usage_example(struct Argx *x);

typedef int (*Argx_Function)(struct Argx *argx, void *user, So so);

/* rlarg/argx-type.c */

typedef enum {
    ARGX_PRIORITY_IMMEDIATELY,
    ARGX_PRIORITY_WHEN_ALL_VALID,
} Argx_Priority_List;

void argx_type_rest(struct Argx *argx, VSo *val);
void argx_type_so(struct Argx *argx, So *val, So *ref);
void argx_type_uri(struct Argx *argx, So *val, So *ref);
void argx_type_bool(struct Argx *argx, bool *val, bool *ref);
void argx_type_int(struct Argx *argx, int *val, int *ref);
void argx_type_size(struct Argx *argx, ssize_t *val, ssize_t *ref);
void argx_type_color(struct Argx *argx, Color *val, Color *ref);
void argx_type_switch(struct Argx *argx);

void argx_type_array_so(struct Argx *argx, VSo *val, VSo *ref);
void argx_type_array_uri(struct Argx *argx, VSo *val, VSo *ref);
void argx_type_array_bool(struct Argx *argx, bool **val, bool **ref);
void argx_type_array_int(struct Argx *argx, int **val, int **ref);
void argx_type_array_size(struct Argx *argx, ssize_t **val, ssize_t **ref);
void argx_type_array_color(struct Argx *argx, Color **val, Color **ref);

void argx_switch_none(struct Argx *switch_argx, struct Argx *argx);
void argx_switch_so(struct Argx *switch_argx, struct Argx *argx, So val);
void argx_switch_uri(struct Argx *switch_argx, struct Argx *argx, So val);
void argx_switch_bool(struct Argx *switch_argx, struct Argx *argx, bool val);
void argx_switch_int(struct Argx *switch_argx, struct Argx *argx, int val);
void argx_switch_size(struct Argx *switch_argx, struct Argx *argx, ssize_t val);
void argx_switch_color(struct Argx *switch_argx, struct Argx *argx, Color val);
void argx_switch_flag(struct Argx *switch_argx, struct Argx *argx, bool value);

void argx_switch_array_so(struct Argx *switch_argx, struct Argx *argx, VSo val);
void argx_switch_array_uri(struct Argx *switch_argx, struct Argx *argx, VSo val);
void argx_switch_array_bool(struct Argx *switch_argx, struct Argx *argx, bool *val);
void argx_switch_array_int(struct Argx *switch_argx, struct Argx *argx, int *val);
void argx_switch_array_size(struct Argx *switch_argx, struct Argx *argx, ssize_t *val);
void argx_switch_array_color(struct Argx *switch_argx, struct Argx *argx, Color *val);

struct Argx_Group *argx_group_enum(struct Argx *argx, int *val, int *ref);
struct Argx_Group *argx_group_options(struct Argx *argx);
struct Argx_Group *argx_group_flags(struct Argx *argx);


struct Argx *argx_enum_bind(struct Argx_Group *group, int val, So name, So desc);

struct Argx *argx_flag(struct Argx_Group *group, bool *val, bool *ref, So name, So desc);

void argx_hint_kind(struct Argx *argx, Argx_Hint_List id);
void argx_hint_text(struct Argx *argx, So text);

/* rlarg/argx-callback.h */

void argx_callback(struct Argx *argx, Argx_Function func, void *user, Argx_Priority_List priority);

/* rlarg/argx-attr.c */

void argx_attr_hide(struct Argx *argx, bool hide);
void argx_attr_configurable(struct Argx *argx, bool configurable);

#define RLARG_H
#endif /* RLARG_H */

