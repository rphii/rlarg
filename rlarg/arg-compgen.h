#ifndef ARG_COMPGEN_H

struct Arg;
struct Argx;

void arg_compgen_global(struct Arg *arg);
void arg_compgen_argx(struct Arg *arg, struct Argx *argx);

#define ARG_COMPGEN_H
#endif /* ARG_COMPGEN_H */

