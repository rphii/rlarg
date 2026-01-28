#ifndef RLARG_STREAM_H

#include <rlso.h>
#include <stddef.h>

/* consider:
 *  --switch=value
 *  --switch value
 *  switch=value
 *  switch value
 *  toml? (configuration file)
 */

typedef struct Arg_Stream {
    const char **argv;
    int argc;
    int i;
    bool skip_flag_check;   /* set true once we encounter '--' */
    VSo *rest;              /* pointer to within an argx that allows to collect "rest" */
} Arg_Stream;

#define RLARG_STREAM_H
#endif /* RLARG_STREAM_H */

