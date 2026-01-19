#ifndef RLARG_ARGX_TYPE_H

#include "rlso.h"

typedef enum {
    ARGX_HINT_REQUIRED, /* <type> */
    ARGX_HINT_OPTIONAL, /* (type) */
    ARGX_HINT_OPTION,   /* {type} */
    ARGX_HINT_FLAGS,    /* [type] */
} Argx_Hint_List;

typedef struct Argx_Hint {
    So so;
    Argx_Hint_List id;
} Argx_Hint;

#define RLARG_ARGX_TYPE_H
#endif /* RLARG_ARGX_TYPE_H */

