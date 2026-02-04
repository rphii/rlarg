#ifndef ARGX_CALLBACK_H

#include "../rlarg.h"

typedef struct Argx_Callback {
    Argx_Priority_List priority;
    Argx_Function func;
    void *user;
} Argx_Callback;

typedef struct Argx_Callback_Queue {
    struct Argx *argx;
    So so;
} Argx_Callback_Queue;

void argx_callback(struct Argx *argx, Argx_Function func, void *user, Argx_Priority_List priority);

#define ARGX_CALLBACK_H
#endif /* ARGX_CALLBACK_H */

