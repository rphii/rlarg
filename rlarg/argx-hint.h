#ifndef RLARG_ARGX_HINT_H

#include "rlso.h"

typedef enum {
    ARGX_HINT_NONE,     /* [[[literally empty]]] */
    ARGX_HINT_REQUIRED, /* <type> */
    ARGX_HINT_ENUM,     /* (type) */
    ARGX_HINT_OPTION,   /* {type} */
    ARGX_HINT_FLAGS,    /* [type] */
    ARGX_HINT_SEQUENCE, /* |sequence| */
} Argx_Hint_List;

typedef struct Argx_Hint {
    So so;
    Argx_Hint_List id;
} Argx_Hint;

#define RLARG_ARGX_HINT_H
#endif /* RLARG_ARGX_HINT_H */

