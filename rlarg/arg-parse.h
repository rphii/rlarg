#ifndef ARG_PARSE_H

#include <rlso.h>
#include "arg-stream.h"
#include "arg.h"

typedef struct Arg_Parse {
} Arg_Parse;

int arg_parse_argx(struct Arg *arg, Arg_Stream *stream, Argx *argx, So so);

#define ARG_PARSE_H
#endif /* ARG_PARSE_H */

