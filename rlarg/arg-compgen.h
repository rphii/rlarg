#ifndef ARG_COMPGEN_H

#include <stddef.h>

struct Arg;
struct Argx;
struct Argx_Group;

void arg_compgen_global(struct Arg *arg);
void arg_compgen_argx(struct Arg *arg, struct Argx *argx);

void arg_compgen_help_argx(struct Arg *arg, struct Argx *argx);
void arg_compgen_help_group(struct Arg *arg, struct Argx_Group *group);
void arg_compgen_help_groups(struct Arg *arg);

#define ARG_COMPGEN_H
#endif /* ARG_COMPGEN_H */

