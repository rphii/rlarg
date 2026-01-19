#ifndef RLARG_ARG_H

#include "../rlarg.h"

#include "arg-x.h"
#include "arg-stream.h"

#include <rlso.h>
#include <rlc.h>

typedef struct Arg {
    Argx *c[ARG_X_SHORT_COUNT]; /* short options */

    Argx_Groups groups; /* long options */

    T_Argx table; /* root of long options -> delve into groups */

    Arg_Stream stream; /* only stdin */

} Arg;

#define RLARG_ARG_H
#endif /* RLARG_ARG_H */

