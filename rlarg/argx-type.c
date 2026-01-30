#include "arg.h"
#include "argx.h"

/* TODO add check so that we will never add an invalid opt (e.g. has to be no format, can not have spaces, etc") */

/* can provide val=0 to override ability from being allowed to set rest to not */
void argx_type_rest(struct Argx *argx, VSo *val) {
    ASSERT_ARG(argx);
    argx->val = (Argx_Value_Union *)val;
    argx->id = ARGX_TYPE_REST;
    argx->hint = (Argx_Hint){
        .id = ARGX_HINT_REQUIRED,
        .so = so("string"),
    };
    argx->is_array = true;
}

void argx_type_so(struct Argx *argx, So *val, So *ref) {
    ASSERT_ARG(argx);
    argx->val = (Argx_Value_Union *)val;
    argx->ref = (Argx_Value_Union *)ref;
    argx->id = ARGX_TYPE_STRING;
    argx->hint = (Argx_Hint){
        .id = ARGX_HINT_REQUIRED,
        .so = so("string"),
    };
}

void argx_type_uri(struct Argx *argx, So *val, So *ref) {
    ASSERT_ARG(argx);
    argx->val = (Argx_Value_Union *)val;
    argx->ref = (Argx_Value_Union *)ref;
    argx->id = ARGX_TYPE_STRING;
    argx->hint = (Argx_Hint){
        .id = ARGX_HINT_REQUIRED,
        .so = so("uri"),
    };
}

void argx_type_bool(struct Argx *argx, bool *val, bool *ref) {
    ASSERT_ARG(argx);
    argx->val = (Argx_Value_Union *)val;
    argx->ref = (Argx_Value_Union *)ref;
    argx->id = ARGX_TYPE_BOOL;
    argx->hint = (Argx_Hint){
        .id = ARGX_HINT_REQUIRED,
        .so = so("bool"),
    };
}

void argx_type_int(struct Argx *argx, int *val, int *ref) {
    ASSERT_ARG(argx);
    argx->val = (Argx_Value_Union *)val;
    argx->ref = (Argx_Value_Union *)ref;
    argx->id = ARGX_TYPE_INT;
    argx->hint = (Argx_Hint){
        .id = ARGX_HINT_REQUIRED,
        .so = so("int"),
    };
}

void argx_type_size(struct Argx *argx, ssize_t *val, ssize_t *ref) {
    ASSERT_ARG(argx);
    argx->val = (Argx_Value_Union *)val;
    argx->ref = (Argx_Value_Union *)ref;
    argx->id = ARGX_TYPE_SIZE;
    argx->hint = (Argx_Hint){
        .id = ARGX_HINT_REQUIRED,
        .so = so("size"),
    };
}

void argx_callback(struct Argx *argx, Argx_Function func, void *user, Argx_Priority_List priority) {
    ASSERT_ARG(argx);
    ASSERT_ARG(func);
    argx->callback.func = func;
    argx->callback.user = user;
    argx->callback.priority = priority;
}

void argx_type_array_so(struct Argx *argx, VSo *val, VSo *ref) {
    ASSERT_ARG(argx);
    argx->val = (Argx_Value_Union *)val;
    argx->ref = (Argx_Value_Union *)ref;
    argx->id = ARGX_TYPE_STRING;
    argx->hint = (Argx_Hint){
        .id = ARGX_HINT_REQUIRED,
        .so = so("string-array"),
    };
    argx->is_array = true;
}

void argx_type_array_uri(struct Argx *argx, VSo *val, VSo *ref) {
    ASSERT_ARG(argx);
    argx->val = (Argx_Value_Union *)val;
    argx->ref = (Argx_Value_Union *)ref;
    argx->id = ARGX_TYPE_URI;
    argx->hint = (Argx_Hint){
        .id = ARGX_HINT_REQUIRED,
        .so = so("uri-array"),
    };
    argx->is_array = true;
}

void argx_type_array_bool(struct Argx *argx, bool **val, bool **ref) {
    ASSERT_ARG(argx);
    argx->val = (Argx_Value_Union *)val;
    argx->ref = (Argx_Value_Union *)ref;
    argx->id = ARGX_TYPE_BOOL;
    argx->hint = (Argx_Hint){
        .id = ARGX_HINT_REQUIRED,
        .so = so("bool-array"),
    };
    argx->is_array = true;
}

void argx_type_array_int(struct Argx *argx, int **val, int **ref) {
    ASSERT_ARG(argx);
    argx->val = (Argx_Value_Union *)val;
    argx->ref = (Argx_Value_Union *)ref;
    argx->id = ARGX_TYPE_INT;
    argx->hint = (Argx_Hint){
        .id = ARGX_HINT_REQUIRED,
        .so = so("int-array"),
    };
    argx->is_array = true;
}

void argx_type_array_size(struct Argx *argx, ssize_t **val, ssize_t **ref) {
    ASSERT_ARG(argx);
    argx->val = (Argx_Value_Union *)val;
    argx->ref = (Argx_Value_Union *)ref;
    argx->id = ARGX_TYPE_SIZE;
    argx->hint = (Argx_Hint){
        .id = ARGX_HINT_REQUIRED,
        .so = so("size-array"),
    };
    argx->is_array = true;
}

struct Argx_Group *argx_group_enum(struct Argx *argx, int *val, int *ref) {
    ASSERT_ARG(argx);
    argx->val = (Argx_Value_Union *)val;
    argx->ref = (Argx_Value_Union *)ref;
    argx->id = ARGX_TYPE_GROUP;
    T_Argx *table;
    NEW(T_Argx, table);
    NEW(Argx_Group, argx->group_s);
    Argx_Group *group = argx->group_s;
    *group = argx_group_init(argx->group_p->arg, table, argx->opt, ARGX_GROUP_ENUM, argx);
    return group;
}

struct Argx_Group *argx_group_options(struct Argx *argx) {
    ASSERT_ARG(argx);
    argx->id = ARGX_TYPE_GROUP;
    T_Argx *table;
    NEW(T_Argx, table);
    NEW(Argx_Group, argx->group_s);
    Argx_Group *group = argx->group_s;
    argx->hint.id = ARGX_HINT_OPTION;
    *group = argx_group_init(argx->group_p->arg, table, argx->opt, ARGX_GROUP_OPTIONS, argx);
    return group;
}

struct Argx_Group *argx_group_flags(struct Argx *argx) {
    ASSERT_ARG(argx);
    argx->id = ARGX_TYPE_GROUP;
    T_Argx *table;
    NEW(T_Argx, table);
    NEW(Argx_Group, argx->group_s);
    Argx_Group *group = argx->group_s;
    argx->hint.id = ARGX_HINT_FLAGS;
    *group = argx_group_init(argx->group_p->arg, table, argx->opt, ARGX_GROUP_FLAGS, argx);
    return group;
}

struct Argx *argx_enum_bind(struct Argx_Group *group, int val, So name, So desc) {
    struct Argx *x = argx_opt(group, 0, name, desc);
    //printff("created '%.*s' on table %p",SO_F(name),group->table);
    x->id = ARGX_TYPE_ENUM;
    x->val_enum = val;
    return x;
}

struct Argx *argx_flag(struct Argx_Group *group, bool *val, bool *ref, So name, So desc) {
    struct Argx *x = argx_opt(group, 0, name, desc);
    argx_type_bool(x, val, ref);
    x->id = ARGX_TYPE_FLAG;
    return x;
}

