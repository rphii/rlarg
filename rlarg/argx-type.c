#include "arg.h"
#include "argx.h"

/* TODO add check so that we will never add an invalid opt (e.g. has to be no format, can not have spaces, etc") */

void argx_type_so(struct Argx *argx, So *val, So *ref) {
    ASSERT_ARG(argx);
    ASSERT_ARG(val);
    argx->val = (Argx_Value_Union *)val;
    argx->ref = (Argx_Value_Union *)ref;
    argx->id = ARGX_STRING;
    argx->hint = (Argx_Hint){
        .id = ARGX_HINT_REQUIRED,
        .so = so("string"),
    };
}

void argx_type_bool(struct Argx *argx, bool *val, bool *ref) {
    ASSERT_ARG(argx);
    ASSERT_ARG(val);
    argx->val = (Argx_Value_Union *)val;
    argx->ref = (Argx_Value_Union *)ref;
    argx->id = ARGX_BOOL;
    argx->hint = (Argx_Hint){
        .id = ARGX_HINT_REQUIRED,
        .so = so("bool"),
    };
}

void argx_type_int(struct Argx *argx, int *val, int *ref) {
    ASSERT_ARG(argx);
    ASSERT_ARG(val);
    argx->val = (Argx_Value_Union *)val;
    argx->ref = (Argx_Value_Union *)ref;
    argx->id = ARGX_INT;
    argx->hint = (Argx_Hint){
        .id = ARGX_HINT_REQUIRED,
        .so = so("int"),
    };
}

void argx_type_size(struct Argx *argx, ssize_t *val, ssize_t *ref) {
    ASSERT_ARG(argx);
    ASSERT_ARG(val);
    argx->val = (Argx_Value_Union *)val;
    argx->ref = (Argx_Value_Union *)ref;
    argx->id = ARGX_SIZE;
    argx->hint = (Argx_Hint){
        .id = ARGX_HINT_REQUIRED,
        .so = so("size"),
    };
}


