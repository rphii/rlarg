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
} Arg_Stream;

#define RLARG_STREAM_H
#endif /* RLARG_STREAM_H */

