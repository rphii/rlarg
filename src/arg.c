#include "arg.h"
#undef SSIZE_MAX
#include <sys/ioctl.h>
#include <unistd.h>
#include <rl/so.h>
#include <rl/array.h>

/* ENUMS, STRUCTS, VECTORS {{{ */

typedef enum {
    ARG_NONE,
    ARG_BOOL,
    ARG_FLAG,
    ARG_SSZ,
    ARG_INT,
    ARG_FLOAT,
    ARG_STRING,
    ARG_COLOR,
    ARG_VECTOR,
    //ARG_EXOTIC,
    ARG_OPTION,
    ARG_FLAGS,
    ARG_HELP,
    ARG_TRY_OPT,
    /* above */
    ARG__COUNT
} ArgList;

const char *arglist_str(ArgList id) {
    switch(id) {
        case ARG_NONE: return "none";
        case ARG_BOOL: return "bool";
        case ARG_FLAG: return "flag";
        case ARG_INT: return "int";
        case ARG_SSZ: return "int";
        case ARG_FLOAT: return "double";
        case ARG_STRING: return "string";
        case ARG_COLOR: return "color";
        case ARG_OPTION: return "option";
        case ARG_HELP: return "help";
        case ARG_FLAGS: return "flags";
        case ARG_VECTOR: return "string-vec";
        case ARG_TRY_OPT: return "args";
        //case ARG_EXOTIC: return "<exotic>";
        case ARG__COUNT: return "(internal:count)";
    }
    return "(internal:invalid)";
}

typedef struct ArgBase {
    So program;           // program name
    So desc;              // description of program
    So epilog;            // text below argument help
    unsigned char prefix;   // default: -
    unsigned char flag_sep; // default: ,
    bool show_help;         // display help if no arguments provided
    bool compgen_wordlist; // generate compgen wordlist
    bool source_check;     // display config errors or not
    VSo *rest_vec;
    So rest_desc;
} ArgBase;

typedef union ArgXVal {
    So *s;            // string
    ssize_t *z;         // number
    int *i;             // number
    double *f;          // double
    bool *b;            // bool
    void *x;            // exotic
    int e;              // enum
    VSo *v;           // vector
    Color *c;           // color
} ArgXVal;

typedef struct ArgXCallback {
    ArgFunction func;
    void *data;
    bool quit_early;
    bool allow_compgen;
    ssize_t priority;
} ArgXCallback;

typedef union ArgXNumber {
    ssize_t z;
    float f;
    int i;
} ArgXNumber;

typedef struct ArgX { /*{{{*/

    ArgList id;
    size_t count; // number of times argx_parse successfully parsed
    ArgXVal val;
    ArgXVal ref;
    int e; // enum
    So type;
    struct ArgXTable *table; // table for below
    struct ArgXGroup *o; // options, flags
    struct ArgXGroup *group; // required for options / parent group
    struct {
        ArgXNumber min;
        ArgXNumber max;
        ArgXCallback callback;
        bool hide_value;
        bool is_env;
        bool require_tf;
    } attr;
    struct {
        const unsigned char c;
        const So opt;
        const So desc;
    } info;

} ArgX; /*}}}*/

void argx_free(struct ArgX *argx);
void argx_group_free_array(struct ArgXGroup **group);

#include <rl/lut.h>
LUT_INCLUDE(TArgX, targx, So, BY_VAL, struct ArgX, BY_REF);
LUT_IMPLEMENT(TArgX, targx, So, BY_VAL, struct ArgX, BY_REF, so_hash, so_cmp, 0, argx_free);
//typedef struct ArgX *TArgX;

int argx_cmp_index(ArgX *a, ArgX *b) {
    ASSERT_ARG(a);
    ASSERT_ARG(b);
    return a->attr.callback.priority - b->attr.callback.priority;
}

#include <rl/vec.h>

VEC_INCLUDE(VArgX, vargx, ArgX *, BY_VAL, BASE);
VEC_INCLUDE(VArgX, vargx, ArgX *, BY_VAL, ERR);
VEC_INCLUDE(VArgX, vargx, ArgX *, BY_VAL, SORT);
VEC_IMPLEMENT(VArgX, vargx, ArgX *, BY_VAL, BASE, 0);
VEC_IMPLEMENT(VArgX, vargx, ArgX *, BY_VAL, ERR);
VEC_IMPLEMENT(VArgX, vargx, ArgX *, BY_VAL, SORT, argx_cmp_index);

typedef struct ArgXTable {
    So desc;
    TArgX lut;
} ArgXTable;

typedef struct ArgXGroup {
    So desc;
    ArgXTable *table;
    struct Arg *root; // required literally only for assigning short options
    struct ArgX *parent; // required for options
    struct ArgX **list;
    bool explicit_help;
} ArgXGroup;

VEC_INCLUDE(VArgXGroup, vargxgroup, ArgXGroup *, BY_REF, BASE);
VEC_INCLUDE(VArgXGroup, vargxgroup, ArgXGroup *, BY_REF, ERR);
VEC_INCLUDE(VArgXGroup, vargxgroup, ArgXGroup *, BY_REF, SORT);
VEC_IMPLEMENT(VArgXGroup, vargxgroup, ArgXGroup *, BY_REF, BASE, argx_group_free_array);
VEC_IMPLEMENT(VArgXGroup, vargxgroup, ArgXGroup *, BY_REF, ERR);
//VEC_IMPLEMENT(VArgXGroup, vargxgroup, ArgXGroup *, BY_REF, SORT, argx_group_free_array);

typedef struct ArgStream {
    VSo vals;
    size_t i;
    VArgX queue;
    bool force_done_parsing;
    bool try_parse;
    bool is_config;
} ArgStream;

typedef struct ArgParse {
    ArgBase *base;  // need the info of prefix...
    struct {
        bool get;
        bool get_explicit;
        ArgX *x;
        ArgXGroup *group;
        ArgX *helpx;
    } help;
    struct {
        VSo *vec;
        So desc;
        ArgXGroup *pos;
    } rest;
    VSo config;
    VSo config_files_expand;
    VSo config_files_base;
} ArgParse;

typedef struct ArgPrint {
    struct {
        int max;    // max width
        int desc;   // spacing until description
        int c;      // spacing until short option
        int opt;    // spacing until long option
    } bounds;
    So_Align_Cache p_al2;
    So_Align whitespace;
    bool compgen_nfirst;
} ArgPrint;

typedef struct ArgFmt {
    So_Fx group;
    So_Fx group_delim;
    So_Fx type;
    So_Fx type_delim;
    So_Fx one_of;
    So_Fx one_of_set;
    So_Fx one_of_delim;
    So_Fx flag;
    So_Fx flag_set;
    So_Fx flag_delim;
    So_Fx val;
    So_Fx val_delim;
    So_Fx c;
    So_Fx opt;
    So_Fx pos;
    So_Fx env;
    So_Fx desc;
    So_Fx program;
} ArgFmt;

typedef struct Arg {
    ArgStream instream;
    ArgBase base;
    struct {
        ArgXTable pos;
        ArgXTable opt;
    } tables;
    ArgX *opt_short[256];
    ArgXGroup pos;
    VArgXGroup groups;
    ArgFmt fmt;
    ArgParse parse;
    ArgPrint print;
    bool done_compgen;
    size_t n_pos_parsed;
} Arg;

/*}}}*/

/* FUNCTION PROTOTYPES {{{ */

#define ERR_arg_config_from_str(...) "failed loading config"
ErrDecl arg_config_from_str(struct Arg *arg, So text);

/*}}}*/

/* ~~~ creation of arguments ~~~ */

/* 0) SETTING UP {{{ */

void argstream_free(ArgStream *stream) {
    if(!stream) return;
    vargx_free(&stream->queue);
    vso_free(&stream->vals);
    memset(stream, 0, sizeof(*stream));
}

ATTR_NODISCARD struct Arg *arg_new(void) {
    Arg *result = 0;
    NEW(Arg, result);
    return result;
}
ATTR_NODISCARD struct ArgXGroup *wargx_new(void) {
    ArgXGroup *group = 0;
    NEW(ArgXGroup, group);
    ArgXTable *table = 0;
    NEW(ArgXTable, table);
    group->table = table;
    return group;
}

struct ArgXGroup *argx_group(struct Arg *arg, So desc, bool explicit_help) {
    size_t len = vargxgroup_length(arg->groups);
    (void)vargxgroup_resize(&arg->groups, len + 1);
    //array_resize(arg->groups, len + 1);
    ArgXGroup **result = vargxgroup_get_at(&arg->groups, len);
    *result = malloc(sizeof(**result));
    ASSERT_ARG(*result);
    memset(*result, 0, sizeof(**result));
    (*result)->explicit_help = explicit_help;
    (*result)->table = &arg->tables.opt;
    (*result)->root = arg;
    (*result)->desc = desc;
    return *result;
}

ArgXGroup *arg_pos(Arg *arg) {
    return &arg->pos;
}

void arg_init(struct Arg *arg, So program, So description, So epilog) {
    ASSERT_ARG(arg);
    arg->base.program = program;
    arg->base.desc = description;
    arg->base.epilog = epilog;
    arg->base.prefix = ARG_INIT_DEFAULT_PREFIX;
    arg->base.show_help = ARG_INIT_DEFAULT_SHOW_HELP;
    arg->base.flag_sep = ARG_INIT_DEFAULT_FLAG_SEP;
    arg->pos.desc = so("Usage");
    arg->pos.root = arg;
    arg->pos.table = &arg->tables.pos;
    arg->pos.desc = so("Usage");
    arg->parse.base = &arg->base;
    arg->print.bounds.c = 2;
    arg->print.bounds.opt = 6;
    arg_init_width(arg, 80, 45);
}

void arg_init_width(struct Arg *arg, int width, int percent) {
    ASSERT_ARG(arg);
    if(width == 0) {
        struct winsize w;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
        width = w.ws_col;
    }
    int desc = width * 45;
    if(desc % 200) desc += 200 - desc % 200;
    desc /= 100;
    arg->print.bounds.desc = desc;
    arg->print.bounds.max = width;
}

void arg_init_show_help(struct Arg *arg, bool show_help) {
    ASSERT_ARG(arg);
    arg->base.show_help = show_help;
}

void arg_init_prefix(struct Arg *arg, unsigned char prefix) {
    ASSERT_ARG(arg);
    arg->base.prefix = prefix;
}

void arg_init_rest(struct Arg *arg, So description, VSo *rest) {
    ASSERT_ARG(arg);
    arg->base.rest_vec = rest;
    arg->base.rest_desc = description;
}

void arg_init_pipe(struct Arg *arg, VSo *out, pthread_mutex_t *mutex) {
    ASSERT_ARG(arg);
    ASSERT_ARG(out);
    ASSERT_ARG(mutex);
}

void arg_init_fmt(struct Arg *arg) {
    ASSERT_ARG(arg);
    arg->fmt.program.fg = COLOR_WHITE;
    arg->fmt.program.bold = true;
    arg->fmt.group.fg = COLOR_WHITE;
    arg->fmt.group.bold = true;
    arg->fmt.group.underline = true;
    arg->fmt.opt.fg = COLOR_WHITE;
    arg->fmt.opt.bold = true;
    arg->fmt.pos.italic = true;
    arg->fmt.val_delim.fg = COLOR_GRAY;
    arg->fmt.val.fg = COLOR_GREEN;
    arg->fmt.flag_delim.fg = COLOR_GRAY;
    arg->fmt.flag.fg = COLOR_YELLOW;
    arg->fmt.flag_set.fg = COLOR_YELLOW;
    arg->fmt.flag_set.bold = true;
    arg->fmt.flag_set.underline = true;
    arg->fmt.flag_delim.fg = COLOR_GRAY;
    arg->fmt.c.fg = COLOR_WHITE;
    arg->fmt.c.bold = true;
    arg->fmt.env.fg = COLOR_WHITE;
    arg->fmt.env.bold = true;
    arg->fmt.one_of.fg = COLOR_BLUE;
    arg->fmt.one_of_set.fg = COLOR_BLUE;
    arg->fmt.one_of_set.bold = true;
    arg->fmt.one_of_set.underline = true;
    arg->fmt.one_of_delim.fg = COLOR_GRAY;
    arg->fmt.type_delim.fg = COLOR_GRAY;
    arg->fmt.type.fg = COLOR_GREEN;
    const size_t bc = arg->print.bounds.c;
    const size_t bo = arg->print.bounds.opt;
    const size_t bd = arg->print.bounds.desc;
    const size_t bm = arg->print.bounds.max;
    so_al_config(&arg->fmt.group.align,         0,    bc,   bm, 0, &arg->print.p_al2);
    so_al_config(&arg->fmt.group_delim.align,   0,    bc,   bm, 0, &arg->print.p_al2);
    so_al_config(&arg->fmt.type.align,          bo,   bo+4, bm, 0, &arg->print.p_al2);
    so_al_config(&arg->fmt.type_delim.align,    bo,   bo+4, bm, 0, &arg->print.p_al2);
    so_al_config(&arg->fmt.one_of.align,        bo,   bo+4, bm, 0, &arg->print.p_al2);
    so_al_config(&arg->fmt.one_of_set.align,    bo,   bo+4, bm, 0, &arg->print.p_al2);
    so_al_config(&arg->fmt.one_of_delim.align,  bo,   bo+4, bm, 0, &arg->print.p_al2);
    so_al_config(&arg->fmt.flag.align,          bo,   bo+4, bm, 0, &arg->print.p_al2);
    so_al_config(&arg->fmt.flag_set.align,      bo,   bo+4, bm, 0, &arg->print.p_al2);
    so_al_config(&arg->fmt.flag_delim.align,    bo,   bo+4, bm, 0, &arg->print.p_al2);
    so_al_config(&arg->fmt.val.align,           bo+8, bo+8, bm, 0, &arg->print.p_al2);
    so_al_config(&arg->fmt.val_delim.align,     bo+8, bo+8, bm, 0, &arg->print.p_al2);
    so_al_config(&arg->fmt.c.align,             bc,   bc,   bm, 0, &arg->print.p_al2);
    so_al_config(&arg->fmt.opt.align,           bo,   bo+2, bm, 0, &arg->print.p_al2);
    so_al_config(&arg->fmt.env.align,           bc,   bc+2, bm, 0, &arg->print.p_al2);
    so_al_config(&arg->fmt.desc.align,          bd,   bo+6, bm, 0, &arg->print.p_al2);
    so_al_config(&arg->fmt.pos.align,           bc,   bc+2, bm, 0, &arg->print.p_al2);
    so_al_config(&arg->fmt.program.align,       0,    0,    bm, 0, &arg->print.p_al2);
    so_al_config(&arg->print.whitespace,        0,    0,    bm, 0, &arg->print.p_al2);
    if(bm - bd >= 32) {
        arg->fmt.val_delim.align.i0 = bd;
        arg->fmt.val_delim.align.iNL = bd;
        arg->fmt.val.align.i0 = bd + 2;
        arg->fmt.val.align.iNL = bd + 2;
        arg->fmt.desc.align.i0 = bd;
        arg->fmt.desc.align.iNL = bd;
    }
}

#define ERR_argx_group_push(...) "failed adding argument x"
ErrDecl argx_group_push(ArgXGroup *group, ArgX *in, ArgX **out) {
    ASSERT_ARG(group);
    ASSERT_ARG(in);
    //ASSERT_ARG(group->table);
    TArgXKV *xkv = targx_once(&group->table->lut, in->info.opt, in);
    if(!xkv) THROW("option '%.*s' already exists!", SO_F(in->info.opt));
    array_push(group->list, xkv->val);
    //return xkv->val;
    if(out) *out = xkv->val;
    return 0;
error:
    //ABORT("critical error in " F("[%.*s]", BOLD) " -- aborting", STR_SO_F(group->desc));
    return -1;
}

struct ArgX *argx_init(struct ArgXGroup *group, const unsigned char c, So optX, So descX) {
    ASSERT_ARG(group);
    So opt = so_trim(optX);
    So desc = so_trim(descX);
    if(!so_len(opt)) ABORT("cannot add an empty long-opt argument");
    ArgX x = {
        .info = {c, opt, desc},
        .group = group,
        .table = group->table,
    };
    ArgX *argx = 0;
    TRYC(argx_group_push(group, &x, &argx));
    if(c) {
        if(!group->root) ABORT("cannot specify short option '%c' for '%.*s'", c, SO_F(opt));
        ArgX *xx = group->root->opt_short[c];
        if(xx) ABORT("already specified short option '%c' for '%.*s'; cannot set for '%.*s' as well.", c, SO_F(xx->info.opt), SO_F(opt));
        group->root->opt_short[c] = argx;
    }
    return argx;
error:
    ABORT("critical error in " F("[%.*s]", BOLD) " -- aborting", SO_F(group->desc));
    return 0;
}

/*}}}*/

/* 1) ASSIGN MAIN ID {{{ */

void argx_str(ArgX *x, So *val, So *ref) {
    ASSERT_ARG(x);
    ASSERT_ARG(val);
    x->id = ARG_STRING;
    x->val.s = val;
    x->ref.s = ref;
}
void argx_col(ArgX *x, Color *val, Color *ref) {
    ASSERT_ARG(x);
    ASSERT_ARG(val);
    x->id = ARG_COLOR;
    x->val.c = val;
    x->ref.c = ref;
}
void argx_ssz(ArgX *x, ssize_t *val, ssize_t *ref) {
    ASSERT_ARG(x);
    ASSERT_ARG(val);
    x->id = ARG_SSZ;
    x->val.z = val;
    x->ref.z = ref;
}
void argx_int(ArgX *x, int *val, int *ref) {
    ASSERT_ARG(x);
    ASSERT_ARG(val);
    x->id = ARG_INT;
    x->val.i = val;
    x->ref.i = ref;
}
void argx_dbl(ArgX *x, double *val, double *ref) {
    ASSERT_ARG(x);
    ASSERT_ARG(val);
    x->id = ARG_FLOAT;
    x->val.f = val;
    x->ref.f = ref;
}
void argx_bool(ArgX *x, bool *val, bool *ref) {
    ASSERT_ARG(x);
    ASSERT_ARG(val);
    x->id = ARG_BOOL;
    x->val.b = val;
    x->ref.b = ref;
}
void argx_bool_require_tf(ArgX *x, bool require_tf) {
    ASSERT_ARG(x);
    ASSERT_ARG(x->id == ARG_BOOL);
    x->attr.require_tf = true;
}
void argx_flag_set(ArgX *x, bool *val, bool *ref) {
    ASSERT_ARG(x);
    ASSERT_ARG(val);
    ASSERT(x->group && x->group->parent && x->group->parent->id == ARG_FLAGS, "expect flags");
    x->id = ARG_FLAG;
    x->val.b = val;
    x->ref.b = ref;
}
void argx_type(struct ArgX *x, So type) {
    ASSERT_ARG(x);
    x->type = type;
}
void argx_none(ArgX *x) {
    ASSERT_ARG(x);
    x->id = ARG_NONE;
}
void argx_vstr(struct ArgX *x, VSo *val, VSo *ref) {
    ASSERT_ARG(x);
    ASSERT_ARG(val);
    x->id = ARG_VECTOR;
    x->val.v = val;
    x->ref.v = ref;
}
void argx_try_parse(struct ArgX *x) {
    ASSERT_ARG(x);
    x->id = ARG_TRY_OPT;
}
struct ArgXGroup *argx_opt(ArgX *x, int *val, int *ref) {
    ASSERT_ARG(x);
    ArgXGroup *options = wargx_new();
    options->desc = x->info.opt;
    options->parent = x;
    x->id = ARG_OPTION;
    x->val.i = (int *)val;
    x->ref.i = (int *)ref;
    x->o = options;
    return options;
}

struct ArgXGroup *argx_flag(struct ArgX *x) {
    ArgXGroup *flags = wargx_new();
    flags->desc = x->info.opt;
    flags->parent = x;
    x->id = ARG_FLAGS;
    x->o = flags;
    return flags;
}

void argx_help(struct ArgX *x, struct Arg *arg) {
    ASSERT_ARG(x);
    x->id = ARG_HELP;
    x->attr.callback.func = (ArgFunction)arg_help;
    x->attr.callback.data = arg;
    x->attr.callback.quit_early = true;
    x->attr.callback.priority = -(ssize_t)1;
    arg->parse.help.helpx = x;
}

struct ArgX *argx_pos(struct Arg *arg, So opt, So desc) {
    ASSERT_ARG(arg);
    ArgX *x = argx_init(&arg->pos, 0, opt, desc);
    return x;
}

struct ArgX *argx_env(struct ArgXGroup *group, So opt, So desc, bool hide_value) {
    ASSERT_ARG(group);
    ArgX *x = argx_init(group, 0, opt, desc);
    x->attr.is_env = true;
    x->attr.hide_value = hide_value;
    return x;
}

struct ArgX *argx_get(struct ArgXGroup *group, So opt) {
    ASSERT_ARG(group);
    ASSERT_ARG(group->table);
    TArgXKV *kv = targx_get_kv(&group->table->lut, opt);
    return kv ? kv->val : 0;
}

/* }}} */

/* 2) ASSIGN SPECIFIC OPTIONS {{{ */

void argx_int_mm(ArgX *x, int min, int max) {
    ASSERT_ARG(x);
    if(x->id != ARG_INT) ABORT("wrong argx type in '%.*s' to set min/max: %s", SO_F(x->info.opt), arglist_str(x->id));
    x->attr.min.i = min;
    x->attr.max.i = max;
}

void argx_ssz_mm(ArgX *x, ssize_t min, ssize_t max) {
    ASSERT_ARG(x);
    if(x->id != ARG_SSZ) ABORT("wrong argx type in '%.*s' to set min/max: %s", SO_F(x->info.opt), arglist_str(x->id));
    x->attr.min.z = min;
    x->attr.max.z = max;
}

void argx_dbl_mm(ArgX *x, double min, double max) {
    ASSERT_ARG(x);
    if(x->id != ARG_FLOAT) ABORT("wrong argx type in '%.*s' to set min/max: %s", SO_F(x->info.opt), arglist_str(x->id));
    x->attr.min.f = min;
    x->attr.max.f = max;
}

void argx_func(struct ArgX *x, ssize_t priority, void *func, void *data, bool allow_compgen, bool quit_early) {
    ASSERT_ARG(x);
    ASSERT_ARG(func);
    x->attr.callback.priority = priority;
    x->attr.callback.func = func;
    x->attr.callback.data = data;
    x->attr.callback.allow_compgen = allow_compgen;
    x->attr.callback.quit_early = quit_early;
}
void argx_opt_enum(struct ArgX *x, int val) {
    ASSERT_ARG(x);
    /* TODO: order of this thing below (the || ... ) ? */
    if(!(x->group && x->group->parent) || x->group->parent->id != ARG_OPTION) {
        ABORT("can only set enums to child nodes of options " F("[%.*s]", BOLD), SO_F(x->info.opt));
    }
    if(!x->group->parent->val.i) {
        ABORT("parent " F("[%.*s]", BOLD) " has to be assigned to an enum", SO_F(x->group->parent->info.opt));
    }
    x->e = val;
    //printff("SET [%.*s] ENUM TO %i", SO_F(x->info.opt), val);
}
void argx_hide_value(struct ArgX *x, bool hide_value) {
    ASSERT_ARG(x);
    x->attr.hide_value = hide_value;
}

/* }}}*/

/* ~~~ implementing the argument parser to actually work ~~~ */

/* PRINTING FUNCTIONS {{{ */

void argx_fmt_type(So *out, Arg *arg, ArgX *argx) { /*{{{*/
    size_t i0 = arg->print.p_al2.progress + 1;
    switch(argx->id) {
        case ARG_COLOR:
        case ARG_STRING:
        case ARG_SSZ:
        case ARG_INT:
        case ARG_FLAG:
        case ARG_BOOL:
        case ARG_VECTOR:
        case ARG_TRY_OPT:
        case ARG_FLOAT: {
            so_fmt_fx(out, arg->fmt.type_delim, i0, "<");
            so_fmt_fx(out, arg->fmt.type, i0, "%.*s", SO_F(argx->type.len ? argx->type : so_l(arglist_str(argx->id))));
            so_fmt_fx(out, arg->fmt.type_delim, i0, ">");
        } break;
        case ARG_OPTION: {
            ArgXGroup *g = argx->o;
            if(array_len(g->list)) {
                so_fmt_fx(out, arg->fmt.one_of_delim, i0, "<");
                for(size_t i = 0; i < array_len(g->list); ++i) {
                    if(i) so_fmt_fx(out, arg->fmt.one_of_delim, i0, "|");
                    ArgX *x = array_at(g->list, i);
                    if(g && g->parent && g->parent->val.i && *g->parent->val.i == x->e) {
                        so_fmt_fx(out, arg->fmt.one_of_set, i0, "%.*s", SO_F(x->info.opt));
                    } else {
                        so_fmt_fx(out, arg->fmt.one_of, i0, "%.*s", SO_F(x->info.opt));
                    }
                }
                so_fmt_fx(out, arg->fmt.one_of_delim, i0, ">");
            }
        } break;
        case ARG_FLAGS: {
            ArgXGroup *g = argx->o;
            if(array_len(g->list)) {
                so_fmt_fx(out, arg->fmt.flag_delim, i0, "<"); /* TODO the space before < also gets format applied! this is wring. */
                for(size_t i = 0; i < array_len(g->list); ++i) {
                    if(i) so_fmt_fx(out, arg->fmt.flag_delim, i0, "|");
                    ArgX *x = array_at(g->list, i);
                    ASSERT_ARG(x->group);
                    ASSERT_ARG(x->group->parent);
                    ASSERT(x->id == ARG_FLAG, "the option [%.*s] in [--%.*s] should be set as a %s", SO_F(x->info.opt), SO_F(x->group->parent->info.opt), arglist_str(ARG_FLAG));
                    if(*x->val.b) {
                        so_fmt_fx(out, arg->fmt.flag_set, i0, "%.*s", SO_F(x->info.opt));
                    } else {
                        so_fmt_fx(out, arg->fmt.flag, i0, "%.*s", SO_F(x->info.opt));
                    }
                }
                so_fmt_fx(out, arg->fmt.flag_delim, i0, ">");
            }
        } break;
        case ARG_HELP: {
            so_fmt_fx(out, arg->fmt.type_delim, i0, " <");
            so_fmt_fx(out, arg->fmt.type, i0, "arg");
            so_fmt_fx(out, arg->fmt.type_delim, i0, ">");
        } break;
        case ARG_NONE:
        case ARG__COUNT: break;
    }
    //////so_push(out, ' ');
    //////TODO maybe set??? (above)
} /*}}}*/

void arg_help_set(struct Arg *arg, struct ArgX *x) {
    ASSERT_ARG(arg);
    ASSERT_ARG(x);
    if(!arg->parse.help.get) {
        arg->parse.help.get = true;
        arg->parse.help.x = x;
    }
}

bool argx_fmt_val(So *out, Arg *arg, ArgX *x, ArgXVal val, So prefix) {
    ASSERT_ARG(out);
    ASSERT_ARG(arg);
    ASSERT_ARG(x);
    bool did_fmt = false;
    if(x->attr.hide_value) return false;
    size_t i0 = arg->print.p_al2.progress + 1;
    switch(x->id) {
        case ARG_NONE: break;
        case ARG_OPTION: {} break;
        case ARG_FLAGS: {} break;
        case ARG_TRY_OPT: {} break;
        case ARG_HELP: {} break;
        case ARG_BOOL: {
            if(!val.b) break;
            so_fmt_fx(out, arg->fmt.val_delim, i0, "%.*s", SO_F(prefix));
            so_fmt_fx(out, arg->fmt.val, i0, "%s", *val.b ? "true" : "false");
            did_fmt = true;
        } break;
        case ARG_COLOR: {
            if(!val.c) break;
            so_fmt_fx(out, arg->fmt.val_delim, i0, "%.*s", SO_F(prefix));
            so_fmt_color(out, *val.c, SO_COLOR_RGB);
            did_fmt = true;
        } break;
        case ARG_FLOAT: {
            if(!val.f) break;
            so_fmt_fx(out, arg->fmt.val_delim, i0, "%.*s", SO_F(prefix));
            so_fmt_fx(out, arg->fmt.val, i0, "%f", *val.f);
            did_fmt = true;
        } break;
        case ARG_INT: {
            if(!val.i) break;
            so_fmt_fx(out, arg->fmt.val_delim, i0, "%.*s", SO_F(prefix));
            so_fmt_fx(out, arg->fmt.val, i0, "%i", *val.i);
            did_fmt = true;
        } break;
        case ARG_FLAG: {
            if(!val.b) break;
            so_fmt_fx(out, arg->fmt.val_delim, i0, "%.*s", SO_F(prefix));
            so_fmt_fx(out, arg->fmt.val, i0, "%s", *val.b ? "true" : "false");
            did_fmt = true;
        } break;
        case ARG_SSZ: {
            if(!val.z) break;
            so_fmt_fx(out, arg->fmt.val_delim, i0, "%.*s", SO_F(prefix));
            so_fmt_fx(out, arg->fmt.val, i0, "%zu", *val.z);
            did_fmt = true;
        } break;
        case ARG_STRING: {
            if(!val.s) break;
            if(!val.s->len) break;
            so_fmt_fx(out, arg->fmt.val_delim, i0, "%.*s", SO_F(prefix));
            so_fmt_fx(out, arg->fmt.val, i0, "%.*s", SO_F(*val.s));
            did_fmt = true;
        } break;
        case ARG_VECTOR: {
            if(!val.v) break;
            so_fmt_fx(out, arg->fmt.val_delim, i0, "%.*s", SO_F(prefix));
            so_fmt_fx(out, arg->fmt.val_delim, i0, "[");
            size_t len = array_len(*val.v);
            for(size_t i = 0; i < len; ++i) {
                if(i) so_fmt_fx(out, arg->fmt.val_delim, 0, ",");
                if(len > 1) so_al_nl(out, arg->print.whitespace, 1);
                So s = array_at(*val.v, i);
                so_fmt_fx(out, arg->fmt.val, 0, "%.*s", SO_F(s));
            }
            so_fmt_fx(out, arg->fmt.val_delim, 0, "]");
            did_fmt = true;
        } break;
        case ARG__COUNT:
        default: THROW("UKNOWN FMT, ID:%u", x->id);
    }
    return did_fmt;
error:
    return false;
}

void argx_fmt(So *out, Arg *arg, ArgX *x, bool detailed) {
    ASSERT_ARG(out);
    ASSERT_ARG(arg);
    ASSERT_ARG(x);
    So tmp = {0};
    bool no_type = false;
    size_t i0 = arg->print.p_al2.progress + 1;
    if(x->group == &arg->pos) {
        /* format POSITIONAL values: full option */
        so_clear(&tmp);
        so_fmt_fx(out, arg->fmt.pos, 0, "%.*s", SO_F(x->info.opt));
        //////so_fmt_al(out, &arg->print.p_al2, arg->print.bounds.opt, arg->print.bounds.opt + 2, arg->print.bounds.max, "%.*s", SO_F(tmp));
    } else if(x->group->table == &arg->tables.opt && !x->attr.is_env) {
        /* format OPTIONAL value: short option + full option */
        if(x->info.c) {
            so_clear(&tmp);
            so_fmt_fx(out, arg->fmt.c, 0, "%c%c", arg->base.prefix, x->info.c);
            //////so_fmt_al(out, &arg->print.p_al2, arg->print.bounds.c, arg->print.bounds.c + 2, arg->print.bounds.max, "%.*s", SO_F(tmp));
        }
        so_clear(&tmp);
        so_fmt_fx(out, arg->fmt.opt, 0, "%c%c%.*s", arg->base.prefix, arg->base.prefix, SO_F(x->info.opt));
        //////so_fmt_al(out, &arg->print.p_al2, arg->print.bounds.opt, arg->print.bounds.opt + 2, arg->print.bounds.max, "%.*s", SO_F(tmp));
    } else {
        ////desc = false;
        //size_t i0 = x->group && x->group.parent ? 
        so_clear(&tmp);
        //So_Fx fmt = x->attr.is_env ? arg->fmt.env : arg->fmt.pos;
        So_Fx fmt = x->attr.is_env ? arg->fmt.env : arg->fmt.one_of;
        size_t i0 = x->attr.is_env ? arg->print.bounds.c : arg->print.bounds.opt + 2;
        so_fmt_fx(out, fmt, i0, "%.*s", SO_F(x->info.opt));
        //////so_fmt_al(out, &arg->print.p_al2, i0, arg->print.bounds.opt + 2, arg->print.bounds.max, "%.*s", SO_F(tmp));
    }
    if(x->info.desc.len) {
        i0 = arg->print.p_al2.progress + 1;
        so_clear(&tmp);
        argx_fmt_type(out, arg, x);
        //////so_fmt_al(out, &arg->print.p_al2, arg->print.p_al2.i0_prev, arg->print.bounds.opt + 2, arg->print.bounds.max, " %.*s", SO_F(tmp));
        so_clear(&tmp);
        so_fmt_fx(out, arg->fmt.desc, 0, "%.*s", SO_F(x->info.desc));
        //////so_fmt_al(out, &arg->print.p_al2, arg->print.bounds.desc, arg->print.bounds.opt + 4, arg->print.bounds.max, "%.*s", SO_F(tmp));
    }
    no_type = (detailed);
    if(!no_type) {
        so_clear(&tmp);
        argx_fmt_val(out, arg, x, x->val, so("="));
        //////so_fmt_al(out, &arg->print.p_al2, arg->print.bounds.desc, arg->print.bounds.opt + 4, arg->print.bounds.max, "%.*s", SO_F(tmp));
    }
    so_al_nl(out, arg->print.whitespace, 1);
    if(detailed) {
        if(x->id == ARG_OPTION && x->o) {
            for(size_t i = 0; i < array_len(x->o->list); ++i) {
                ArgX *argx = array_at(x->o->list, i);
                argx_fmt(out, arg, argx, false);
            }
        } else if(x->id == ARG_FLAGS) {
            ////////so_fmt_al(out, &arg->print.p_al2, 0, 0, arg->print.bounds.max, "\n");
            //for(size_t i = 0; i < vargx_length(x->o->vec); ++i) {
            //    ArgX *argx = vargx_get_at(&x->o->vec, i);
            ////////    so_fmt_al(out, &arg->print.p_al2, 0, 0, arg->print.bounds.max, "\n");
            //    argx_fmt(out, arg, argx, false);
            //}
        }
        so_clear(&tmp);
        if(argx_fmt_val(&tmp, arg, x, x->val, so("current value: "))) {
            so_fmt_al(out, arg->print.whitespace, 0, "\n%.*s", tmp);
            //////so_fmt_al(out, &arg->print.p_al2, 0, 0, arg->print.bounds.max, "\n");
            //////so_fmt_al(out, &arg->print.p_al2, 0, arg->print.bounds.opt + 2, arg->print.bounds.max, "%.*s", SO_F(tmp));
        }
        so_clear(&tmp);
        if(argx_fmt_val(&tmp, arg, x, x->ref, so("default value: "))) {
            //so_al_nl(out, arg->print.whitespace, 1);
            so_fmt_al(out, arg->print.whitespace, 0, "\n%.*s", tmp);
            //////so_fmt_al(out, &arg->print.p_al2, 0, 0, arg->print.bounds.max, "\n");
            //////so_fmt_al(out, &arg->print.p_al2, 0, arg->print.bounds.opt + 2, arg->print.bounds.max, "%.*s", SO_F(tmp));
        }
        so_al_nl(out, arg->print.whitespace, 1);
        //////so_fmt_al(out, &arg->print.p_al2, 0, 0, arg->print.bounds.max, "\n");
        /* done */
    }
    so_free(&tmp);
}

void argx_fmt_group(So *out, Arg *arg, ArgXGroup *group) {
    ASSERT_ARG(out);
    ASSERT_ARG(arg);
    ASSERT_ARG(group);
    /* empty groups we don't care about */
    So tmp = {0};
    if(!array_len(group->list) && !(group == &arg->pos)) {
        return;
    }
    /* group title */
    if(group->desc.len) {
        so_clear(&tmp);
        so_fmt_fx(out, arg->fmt.group, 0, "%.*s:", SO_F(group->desc));
        //////so_fmt_al(out, &arg->print.p_al2, 0, 0, arg->print.bounds.max, "%.*s", SO_F(tmp));
    }
    if(arg->parse.help.get && arg->parse.help.group) {
        if(arg->parse.help.group != group) {
            so_clear(&tmp);
            so_fmt_fx(out, arg->fmt.group_delim, 0, "<collapsed>", SO_F(group->desc));
            //////so_fmt_al(out, &arg->print.p_al2, 0, 0, arg->print.bounds.max, " %.*s\n", SO_F(tmp));
            so_free(&tmp);
            return;
        }
    } else if(group->explicit_help) {
        size_t i0 = arg->print.p_al2.progress + 1;
        so_clear(&tmp);
        so_fmt_fx(out, arg->fmt.group_delim, i0, "--help '%.*s'", SO_F(group->desc), SO_F(group->desc));
        so_al_nl(out, arg->print.whitespace, 1);
        //////so_fmt_al(out, &arg->print.p_al2, 0, 0, arg->print.bounds.max, " %.*s\n", SO_F(tmp));
        so_free(&tmp);
        return;
    }
    /* usage / group title */
    if(group != &arg->pos) {
        so_al_nl(out, arg->print.whitespace, 1);
    } else {
        so_clear(&tmp);
        so_fmt_fx(out, arg->fmt.program, 0, "%.*s ", SO_F(arg->base.program));
        //////so_fmt_al(out, &arg->print.p_al2, 0, 0, arg->print.bounds.max, "%.*s", SO_F(tmp));
    }
    /* each thing */
    for(size_t i = 0; i < array_len(group->list); ++i) {
        ArgX *x = array_at(group->list, i);
        so_clear(&tmp);
        argx_fmt(out, arg, x, false);
    }
#if 0
    /* rest */
    if(arg->parse.rest.vec) {
        so_clear(&tmp);
        so_fmt_fx(&tmp, arg->fmt.pos, "%.*s ", SO_F(arg->parse.rest.desc));
        //////so_fmt_al(out, &arg->print.p_al2, 0, 0, arg->print.bounds.max, "%.*s", SO_F(tmp));
    }
#endif
    so_free(&tmp);
}

void argx_fmt_specific(So *out, Arg *arg, ArgParse *parse, ArgX *x) { /*{{{*/
    So tmp = {0};
    if(x->group) {
        if(x->group->parent) {
            argx_fmt_specific(out, arg, parse, x->group->parent);
        } else {
            so_fmt_fx(out, arg->fmt.group, 0, "%.*s:", SO_F(x->group->desc));
            //////so_fmt_al(out, &arg->print.p_al2, 0, 0, arg->print.bounds.max, "%.*s\n", SO_F(tmp));
        }
    }
    argx_fmt(out, arg, x, (x == parse->help.x));
    so_free(&tmp);
    //argx_print(base, x, false);
} /*}}}*/

void argx_print_opt(bool *re, const char *fmt, ...) {
    va_list args;
    if(*re) printf("%c", 0);
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    *re = true;
}

void argx_compgen(struct Arg *arg, struct ArgX *x) {
    ASSERT_ARG(arg);
    if(!x) return;
    switch(x->id) {
        case ARG_OPTION:
        case ARG_FLAGS: {
            if(!x->o) break;
            for(size_t i = 0; i < array_len(x->o->list); ++i) {
                ArgX *argx = array_at(x->o->list, i);
                argx_print_opt(&arg->print.compgen_nfirst, "%.*s", SO_F(argx->info.opt));
            }
        } break;
        case ARG_BOOL: {
            printf("true%cfalse", 0);
        } break;
        case ARG_HELP: {
            for(size_t i = 0; i < vargxgroup_length(arg->groups); ++i) {
                ArgXGroup *group = *vargxgroup_get_at(&arg->groups, i);
                argx_print_opt(&arg->print.compgen_nfirst, "%.*s", SO_F(group->desc));
            }
        } break;
        default: break;
    }
}

void arg_compgen(struct Arg *arg) {
    ASSERT_ARG(arg);
    if(arg->done_compgen) return;
    arg->done_compgen = true;
    if(arg->parse.help.group) return;
    arg->done_compgen = true;
    /* optional help */
    TArgXKV **kv = 0;
    ArgXTable *opt_table = &arg->tables.opt;
    if(arg->parse.help.get) {
        ArgX *x = arg->parse.help.x;
        if(x) {
            opt_table = x->o ? x->o->table : 0;
            if(!opt_table && x->id != ARG_HELP) argx_compgen(arg, x);
        }
        if(arg->parse.help.get_explicit) {
            if(!x || (x && x->id != ARG_HELP)) {
                argx_compgen(arg, arg->parse.help.helpx);
            }
        }
    }
    if(opt_table) {
        while((kv = targx_iter_all(&opt_table->lut, kv))) {
            ArgX *x = (*kv)->val;
            if(x->attr.is_env) continue;
            if(x->group && x->group->parent) {
                argx_print_opt(&arg->print.compgen_nfirst, "%.*s", SO_F(x->info.opt));
            } else {
                int len = array_len(arg->instream.vals);
                if(len >= 1 && *arg->instream.vals[len - 1].str == '-') {
                    argx_print_opt(&arg->print.compgen_nfirst, "%c%c%.*s", arg->base.prefix, arg->base.prefix, SO_F(x->info.opt));
                }
            }
        }
    }
    if(!arg->parse.help.x) {
        /* positional help */
        size_t i = arg->n_pos_parsed;
        if(i < array_len(arg->pos.list)) {
            ArgX *x = array_at(arg->pos.list, i);
            //printff("X = %.*s", SO_F(x->info.opt));
            argx_compgen(arg, x);
        }
    }
    printf("\n");
}

int arg_help(struct Arg *arg) { /*{{{*/
    if(arg->base.compgen_wordlist) {
        arg_compgen(arg);
        return 0;
    }
    ASSERT_ARG(arg);
    So out = {0};
    /* if term width < min, adjust */
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    int width_restore = arg->print.bounds.max;
    int width = w.ws_col < arg->print.bounds.max ? w.ws_col : arg->print.bounds.max;
    arg->print.bounds.max = width;
    /* now print */
    if(arg->parse.help.x && arg->parse.help.get) {
        /* specific help */
        argx_fmt_specific(&out, arg, &arg->parse, arg->parse.help.x);
    } else {
        /* default help */
        so_fmt_fx(&out, arg->fmt.program, 0, "%.*s:", SO_F(arg->base.program));
        so_fmt_fx(&out, (So_Fx){0}, 0, " %.*s", SO_F(arg->base.desc));
        //////so_fmt_al(&out, &arg->print.p_al2, 0, 0, arg->print.bounds.max, "%.*s", SO_F(tmp));

        so_fmt_fx(&out, arg->fmt.group, 0, "%.*s:", SO_F(arg->pos.desc));
        //////so_fmt_al(&out, &arg->print.p_al2, 0, arg->print.bounds.c, arg->print.bounds.max, "%.*s\n", SO_F(tmp));
        so_fmt_fx(&out, arg->fmt.program, 0, "%.*s", SO_F(arg->base.program));
        //////so_fmt_al(&out, &arg->print.p_al2, arg->print.bounds.c, arg->print.bounds.c + 2, arg->print.bounds.max, "%.*s", SO_F(tmp));

        for(size_t i = 0; i < array_len(arg->pos.list); ++i) {
            ArgX *argx = array_at(arg->pos.list, i);
            so_fmt_fx(&out, arg->fmt.pos, 0, "%.*s", SO_F(argx->info.opt));
            //////so_fmt_al(&out, &arg->print.p_al2, 0, arg->print.bounds.c, arg->print.bounds.max, " %.*s", SO_F(tmp));
        }
        /*  */
        so_al_nl(&out, arg->print.whitespace, 1);
        for(size_t i = 0; i < array_len(arg->pos.list); ++i) {
            ArgX *argx = array_at(arg->pos.list, i);
            argx_fmt(&out, arg, argx, false);
        }

        if(arg->parse.help.get_explicit) {
            /* all other stuff */
            for(size_t i = 0; i < vargxgroup_length(arg->groups); ++i) {
                ArgXGroup **group = vargxgroup_get_at(&arg->groups, i);
                argx_fmt_group(&out, arg, *group);
            }
            if(so_len(arg->base.epilog)) {
                //arg_handle_print(arg, ARG_PRINT_NONE, "%.*s", SO_F(arg->base.epilog));
                /////so_al_nl(&out, arg->print.whitespace, 1);
                /////so_fmt_al(&out, arg->print.p_al2, "%.*s", SO_F(argx->info.opt));
                //////so_fmt_al(&out, &arg->print.p_al2, 0, 0, arg->print.bounds.max, "%.*s\n", SO_F(arg->base.epilog));
            }
        }
    }
    so_print(out);
    so_free(&out);
    arg->print.bounds.max = width_restore;
    return 0;
} /*}}}*/

void arg_config(struct Arg *arg, So conf) {
    ASSERT_ARG(arg);
    array_push(arg->parse.config, conf);
}

int arg_config_from_file(struct Arg *arg, So filename) {
    ASSERT_ARG(arg);
    So expanded = {0};
    so_extend_wordexp(&expanded, filename, true);
    if(!expanded.len) return 0;
    for(size_t i = 0; i < array_len(arg->parse.config_files_expand); ++i) {
        if(!so_cmp(array_at(arg->parse.config_files_expand, i), expanded)) {
            so_free(&expanded);
            return 0;
        }
    }
    array_push(arg->parse.config_files_expand, expanded);
    So text = {0};
    if(so_file_read(expanded, &text)) goto error;
    array_push(arg->parse.config, text);
    if(!text.len) {
        so_free(&text);
        return 0;
    }
    TRYC(arg_config_from_str(arg, text));
    return 0;
error:
    THROW_PRINT("failed reading file: '%.*s'\n", SO_F(expanded));
    return -1;
}

/* }}} */

#if 0
bool arg_group_info_opt(struct ArgXTable *g, void *x, RStr *found) {
    if(!g) return false;
    if(!x) return false;
    ASSERT_ARG(found);
    for(TArgXKV **kv = targx_iter_all(&g->lut, 0);
            kv;
            kv = targx_iter_all(&g->lut, kv)) {
        if((*kv)->val->
    }
}

RStr arg_info_opt(struct Arg *arg, void *x) {
    if(!x) return RSTR("");
    for(arg->parse
}
#endif

/* PARSING FUNCTIONS {{{ */

#define ERR_arg_parse_getopt_long(table, x, opt, ...) "nothing found for '" F("%.*s", FG_BL_B) "'", SO_F(opt)
ErrDecl arg_parse_getopt_long(ArgXTable *table, ArgX **x, So opt) {
    ASSERT_ARG(table);
    ASSERT_ARG(x);
    *x = targx_get(&table->lut, opt);
    if(!*x) return -1;
    return 0;
}

#define ERR_arg_parse_getopt_short(arg, ...) "failed getting short option"
ErrDecl arg_parse_getopt_short(Arg *arg, ArgX **x, const unsigned char c) {
    ASSERT_ARG(arg);
    ASSERT_ARG(x);
    *x = arg->opt_short[c];
    if(!*x) THROW("value " F("%c", FG_BL_B) " is not a valid short option", c);
    return 0;
error:
    return -1;
}

#define ERR_arg_parse_getv(...) "failed getting an argument"
ErrDecl arg_parse_getv(ArgParse *parse, ArgStream *stream, So *argV, bool *need_help) {
    ASSERT_ARG(parse);
    ASSERT_ARG(stream->vals);
    ASSERT_ARG(argV);
    bool pe = !stream->try_parse;
    /* parse->compgen? */
    unsigned int pfx = parse->base->prefix;
    So result;
repeat:
    if(stream->i < array_len(stream->vals)) {
        So argv = stream->vals[stream->i++];
        result = argv;
        if(!stream->force_done_parsing && so_len(result) == 2 && so_at(result, 0) == pfx && so_at(result, 1) == pfx) {
            stream->force_done_parsing = true;
            goto repeat;
        }
        *argV = result;
        //printff("GOT ARGUMENT %.*s", SO_F(*argV));
    } else {
        if(stream->force_done_parsing) {
        } else if(parse->base->compgen_wordlist) {
            *need_help = true;
        } else if(!parse->help.get) {
            THROW_P(pe, "no arguments left");
        } else {
            *need_help = true;
        }
    }
    return 0;
error:
    return -1;
}

void arg_parse_getv_undo(ArgParse *parse, ArgStream *stream) {
    ASSERT_ARG(parse);
    ASSERT_ARG(stream);
    ASSERT_ARG(stream->vals);
    ASSERT(stream->i, "nothing left to undo");
    --stream->i;
}

bool argx_parse_is_origin_from_pos(ArgParse *parse, ArgX *argx) {
    ASSERT_ARG(parse);
    ASSERT_ARG(parse->rest.pos);
    ASSERT_ARG(argx);
    if(!argx->group) return false;
    if(argx->group == parse->rest.pos) return true;
    if(!argx->group->parent) return false;
    return argx_parse_is_origin_from_pos(parse, argx->group->parent);
}

ArgXVal *argxval_to_set(ArgX *argx, ArgStream *stream) {
    ArgXVal *v = &argx->val;
    if(stream->is_config && argx->ref.x) v = &argx->ref;
    return v;
}

#define ERR_argx_parse(parse, stream, argx, ...) "failed parsing argument " F("[%.*s]", BOLD FG_WT_B) " " F("%s", ARG_TYPE_F), SO_F(argx->info.opt), arglist_str(argx->id)
ErrDecl argx_parse(ArgParse *parse, ArgStream *stream, ArgX *argx, bool *quit_early) {
    ASSERT_ARG(parse);
    ASSERT_ARG(argx);
    //printff("PARSE [%.*s]", SO_F(argx->info.opt));
    /* add to queue for post processing */
    bool pe = !stream->try_parse;
    TRYG(vargx_push_back(&stream->queue, argx));
    So argV = so("");
    /* check if we want to get help for this */
    if(parse->help.get && !argx->attr.is_env) {
        if(!parse->help.x || (argx->group ? parse->help.x == argx->group->parent : false)) {
            parse->help.x = argx;
        }
    }
    /* check enum / option */
    if(argx->group && argx->group->parent && argx->group->parent->id == ARG_OPTION) {
        if(argx->group->parent->val.i) {
            *argx->group->parent->val.i = argx->e;
        }
    }
    /* actually begin parsing */
    bool need_help = false;
    //printff("SETTING VALUE FOR %.*s ID %u", SO_F(argx->info.opt), argx->id);
    switch(argx->id) {
        case ARG_BOOL: { //printff("GET VALUE FOR BOOL");
            if(stream->i < array_len(stream->vals)) {
                TRYC_P(pe, arg_parse_getv(parse, stream, &argV, &need_help)); //printff("GOT VALUE [%.*s]", SO_F(argV));
                if(need_help) break;
                if(so_as_yes_or_no(argV, argxval_to_set(argx, stream)->b)) {
                    if(argx->attr.require_tf) {
                        THROW_P(pe, "failed parsing bool");
                    } else {
                        *argxval_to_set(argx, stream)->b = true;
                        arg_parse_getv_undo(parse, stream);
                    }
                }
            } else if(argx->attr.require_tf) {
                THROW_P(pe, "failed parsing bool");
            } else {
                *argxval_to_set(argx, stream)->b = true;
            }
            if(need_help) break;
            if(!stream->is_config) ++argx->count;
        } break;
        case ARG_FLAG: {
            *argx->val.b = true;
            if(!stream->is_config) ++argx->count;
        } break;
        case ARG_SSZ: {
            TRYC_P(pe, arg_parse_getv(parse, stream, &argV, &need_help));
            if(need_help) break;
            TRYC_P(pe, so_as_ssize(argV, argxval_to_set(argx, stream)->z, 0));
            if(!stream->is_config) ++argx->count;
        } break;
        case ARG_INT: {
            TRYC_P(pe, arg_parse_getv(parse, stream, &argV, &need_help));
            if(need_help) break;
            ssize_t z = 0;
            TRYC_P(pe, so_as_ssize(argV, &z, 0));
            *argxval_to_set(argx, stream)->i = (int)z;
            if(!stream->is_config) ++argx->count;
        } break;
        case ARG_FLOAT: {
            TRYC_P(pe, arg_parse_getv(parse, stream, &argV, &need_help));
            if(need_help) break;
            TRYC_P(pe, so_as_double(argV, argxval_to_set(argx, stream)->f));
            if(!stream->is_config) ++argx->count;
        } break;
        case ARG_COLOR: {
            TRYC_P(pe, arg_parse_getv(parse, stream, &argV, &need_help));
            if(need_help) break;
            TRYC_P(pe, so_as_color(argV, argxval_to_set(argx, stream)->c));
            if(!stream->is_config) ++argx->count;
        } break;
        case ARG_STRING: {
            TRYC_P(pe, arg_parse_getv(parse, stream, &argV, &need_help));
            if(need_help) break;
            *argxval_to_set(argx, stream)->s = argV;
            if(!stream->is_config) ++argx->count;
        } break;
        case ARG_VECTOR: {
            TRYC_P(pe, arg_parse_getv(parse, stream, &argV, &need_help));
            if(need_help) break;
            array_push(*argxval_to_set(argx, stream)->v, argV);
            if(!stream->is_config) ++argx->count;
            if(argx_parse_is_origin_from_pos(parse, argx)) {
                parse->rest.vec = argxval_to_set(argx, stream)->v;
                parse->rest.desc = argx->info.desc;
            }
        } break;
        case ARG_OPTION: {
            TRYC_P(pe, arg_parse_getv(parse, stream, &argV, &need_help));
            if(need_help) break;
            ArgX *x = 0;
            TRYC_P(pe, arg_parse_getopt_long(argx->o->table, &x, argV));
            TRYC_P(pe, argx_parse(parse, stream, x, quit_early));
            if(!stream->is_config) ++argx->count;
        } break;
        case ARG_FLAGS: {
            TRYC_P(pe, arg_parse_getv(parse, stream, &argV, &need_help));
            if(need_help) break;
            ASSERT_ARG(argx->o);
            if(!argx->count) {
                for(size_t i = 0; i < array_len(argx->o->list); ++i) {
                    ArgX *x = array_at(argx->o->list, i);
                    *x->val.b = false;
                }
            }
            for(So flag = {0}; so_splice(argV, &flag, parse->base->flag_sep); ) {
                if(!flag.str) continue;
                ArgX *x = 0;
                TRYC_P(pe, arg_parse_getopt_long(argx->o->table, &x, flag));
                TRYC_P(pe, argx_parse(parse, stream, x, quit_early));
                if(!stream->is_config) ++argx->count;
            }
        } break;
        case ARG_TRY_OPT: {
            /* get current parse state */
            if(stream->try_parse) break;
            //ArgParse p = *parse;
            bool discard_quit = false;
            ArgStream s = *stream;
            s.try_parse = true;
            /* get next value and parse */
            So argv = {0};
            if(arg_parse_getv(parse, &s, &argv, &discard_quit)) goto arg_try_opt_error;
            ArgX *x = 0;
            char pfx = parse->base->prefix;
            if(stream->is_config) {
                if(arg_parse_getopt_long(argx->table, &x, argv)) goto arg_try_opt_error;
            } else {
                if(!(argv.len > 2 && so_at(argv, 0) == pfx && so_at(argv, 1) == pfx)) goto arg_try_opt_error;
                if(arg_parse_getopt_long(argx->table, &x, so_i0(argv, 2))) goto arg_try_opt_error;
            }
            if(argx_parse(parse, &s, x, &discard_quit)) goto arg_try_opt_error;
            goto arg_try_opt_done;
arg_try_opt_error:
            if(stream->is_config) stream->i = array_len(stream->vals);
arg_try_opt_done:
            /* restore parse state */
            s.try_parse = false;
            *stream = s;
        } break;
        case ARG_HELP: {
            parse->help.get_explicit = true;
            parse->help.get = true;
        } break;
        /* above */
        case ARG__COUNT:
        case ARG_NONE: break;
    }
    /* TODO DRY */
    if(argx && argx->attr.callback.func && !argx->attr.callback.priority) {
        if(!(parse->base->compgen_wordlist && !argx->attr.callback.allow_compgen)) {
            if(argx->attr.callback.func(argx->attr.callback.data)) {
                if(!(pe || stream->is_config)) {
                    THROW_PRINT("failed executing function for " F("[%.*s]", BOLD) "\n", SO_F(argx->info.opt));
                }
                goto error_skip_help;
            }
            *quit_early = argx->attr.callback.quit_early;
            //if(*quit_early) break;
        }
    }
    if(!(parse->base->compgen_wordlist && need_help)) {
        return 0;
    }
error:
    if(!parse->help.get) {
        //printff("HELP GET X: %.*s", SO_F(argx->info.opt));
        parse->help.get = true;
        parse->help.x = argx;
    }
    if(parse->base->compgen_wordlist) {
        return 0;
    }
error_skip_help:
    return -1;
}

void arg_parse_setref_table(struct ArgXTable *table);

void arg_parse_setref_argx(struct ArgX *argx) {
    ASSERT_ARG(argx);
    switch(argx->id) {
        case ARG_FLAG:
        case ARG_BOOL: {
            if(argx->ref.b) *argx->val.b = *argx->ref.b;
        } break;
        case ARG_SSZ: {
            if(argx->ref.z) *argx->val.z = *argx->ref.z;
        } break;
        case ARG_INT: {
            if(argx->ref.i) *argx->val.i = *argx->ref.i;
        } break;
        case ARG_FLOAT: {
            if(argx->ref.f) *argx->val.f = *argx->ref.f;
        } break;
        case ARG_COLOR: {
            if(argx->ref.c) *argx->val.c = *argx->ref.c;
        } break;
        case ARG_STRING: {
            //printff("SETTING STR %.*s=%.*s", SO_F(argx->info.opt), SO_F(*argx->ref.s));
            if(argx->ref.s) *argx->val.s = *argx->ref.s;
        } break;
        case ARG_VECTOR: {
            if(argx->ref.v) *argx->val.v = *argx->ref.v;
        } break;
        case ARG_OPTION: {
            if(argx->ref.i) *argx->val.i = *argx->ref.i;
            if(argx->o) arg_parse_setref_table(argx->o->table);
        } break;
        case ARG_FLAGS: {
            if(argx->o) arg_parse_setref_table(argx->o->table);
        } break;
        case ARG_TRY_OPT:
        case ARG_HELP:
        case ARG_NONE:
        case ARG__COUNT: break;
    }
}

void arg_parse_setref_table(struct ArgXTable *table) {
    ASSERT_ARG(table);
    TArgXKV **kv = 0;
    while((kv = targx_iter_all(&table->lut, kv))) {
        ArgX *x = (*kv)->val;
        arg_parse_setref_argx(x);
    }
}

void arg_parse_setref(struct Arg *arg) {
    ASSERT_ARG(arg);
    /* first verify some things */
    /* finally assign */
    arg_parse_setref_table(&arg->tables.opt);
    arg_parse_setref_table(&arg->tables.pos);
    //arg_parse_setref_group(&arg->pos);
}

#define ERR_argstream_parse(...) "failed parsing argument stream"
int argstream_parse(struct Arg *arg, ArgStream *stream, bool *quit_early) {
    ASSERT_ARG(arg);
    ASSERT_ARG(stream);
    ASSERT_ARG(quit_early);
    int err = 0;
    int config_status = false;
    bool need_help = false;
    char pfx = arg->base.prefix;
    ArgX *argx = 0;
    ArgParse *parse = &arg->parse;
    /* check optional arguments */
    while(stream->i < array_len(stream->vals)) {
        So argV = so("");
        TRYC(arg_parse_getv(parse, stream, &argV, &need_help));
        if(need_help) break;
        if(*quit_early) goto quit_early;
        if(!so_len(argV)) continue;
        //printff(" [%.*s] %zu / %zu : config %u", SO_F(argV), stream->i, array_len(stream->vals), stream->is_config);
        if(!stream->force_done_parsing && (stream->is_config || (so_len(argV) >= 1 && so_at(argV, 0) == pfx))) {
            /* regular checking for options */
            if(stream->is_config || (so_len(argV) >= 2 && so_at(argV, 1) == pfx)) {
                So arg_query = stream->is_config ? argV : so_i0(argV, 2);
                /* long option */
                TRYC(arg_parse_getopt_long(&arg->tables.opt, &argx, arg_query));
                if(!argx->attr.is_env) {
                    TRYC(argx_parse(parse, stream, argx, quit_early));
                } else {
                    ASSERT_ARG(0);
                }
            } else {
                So arg_queries = so_i0(argV, 1);
                /* short option */
                for(size_t i = 0; i < so_len(arg_queries); ++i) {
                    const unsigned char query = so_at(arg_queries, i);
                    TRYC(arg_parse_getopt_short(arg, &argx, query));
                    if(!argx->attr.is_env) {
                        TRYC(argx_parse(parse, stream, argx, quit_early));
                    } else {
                        ASSERT_ARG(0);
                    }
                    //printff("SHORT OPTION! %.*s", SO_F(arg_queries));
                }
                //ArgX *argx = arg->opt_short[
            }
        } else if((arg->n_pos_parsed < array_len(arg->pos.list)) && !stream->is_config) {
            /* check for positional */
            arg_parse_getv_undo(parse, stream);
            ArgX *x = array_at(arg->pos.list, arg->n_pos_parsed);
            TRYC(argx_parse(parse, stream, x, quit_early));
            ++arg->n_pos_parsed;
        } else if(parse->rest.vec) {
            /* no argument, push rest */
            array_push(*parse->rest.vec, argV);
        }
        /* in case of trying to get help, also search pos and then env and then group */
        if(parse->help.get_explicit && stream->i < array_len(stream->vals)) {
            (void)arg_parse_getv(parse, stream, &argV, &need_help);
            ArgX *x = targx_get(&arg->tables.opt.lut, argV);
            //printff("GET HELP [%.*s]", SO_F(argV));
            if(argV.len) {
                for(size_t j = 0; j < vargxgroup_length(arg->groups); ++j) {
                    ArgXGroup **group = vargxgroup_get_at(&arg->groups, j);
                    if(!so_cmp(argV, (*group)->desc)) {
                        arg->parse.help.group = *group;
                    }
                }
            }
            if(x) {
                arg->parse.help.x = x;
            }
            if(!x && !arg->parse.help.group) {
                arg_parse_getv_undo(parse, stream);
            }
        }
    }
    //printff(" argstream done (%zu/%zu), forced? %u, quit? %u", stream->i, array_len(stream->vals), stream->force_done_parsing, *quit_early);
    if(config_status) {
        if(parse->help.get) {
            printf("\n");
        } else {
            goto error_skip_help;
        }
    }
    /* now go over the queue and do post processing */
    vargx_sort(&stream->queue);
    for(size_t i = 0; i < vargx_length(stream->queue); ++i) {
        ArgX *x = vargx_get_at(&stream->queue, i);
        if(!x->attr.callback.priority) continue;
        //printff("CHECK QUEUE [%.*s]", SO_F(x->info.opt));
        if(x && x->attr.callback.func) {
            if(parse->base->compgen_wordlist && !x->attr.callback.allow_compgen) continue;
            if(x->attr.callback.func(x->attr.callback.data)) {
                if(!(stream->is_config)) {
                    THROW_PRINT("failed executing function for " F("[%.*s]", BOLD) "\n", SO_F(x->info.opt));
                }
                goto error_skip_help;
            }
            *quit_early = x->attr.callback.quit_early;
            if(*quit_early) goto quit_early;
        }
    }
clean:
    return err;
quit_early:
    *quit_early = true;
    goto clean;
error_skip_help:
    return 0;
error:
    ERR_CLEAN;
}

ErrDecl arg_parse(struct Arg *arg, const unsigned int argc, const char **argv, bool *quit_early) {
    ASSERT_ARG(arg);
    ASSERT_ARG(arg->parse.base);
    ASSERT_ARG(quit_early);
    ASSERT_ARG(argv);
    ArgParse *parse = &arg->parse;
    for(size_t i = 1; i < argc; ++i) {
        array_push(arg->instream.vals, so_l(argv[i]));
    }
    parse->rest.vec = arg->base.rest_vec;
    parse->rest.desc = arg->base.rest_desc;
    parse->rest.pos = &arg->pos;
    int err = 0;
    /* prepare parsing */
    So env = SO;
    ArgStream tmpstream = {0};
    unsigned char pfx = arg->base.prefix;
    /* gather environment variables */
    TArgXKV **kv = 0;
    while((kv = targx_iter_all(&arg->tables.opt.lut, kv))) {
        ArgX *x = (*kv)->val;
        if(!x->attr.is_env) continue;
        if(so_env_get(&env, x->info.opt)) continue;
        vso_clear(&tmpstream.vals);
        argstream_free(&tmpstream);
        array_push(tmpstream.vals, env);
        TRYC(argx_parse(parse, &tmpstream, x, quit_early));
        //if(parse->help.get) goto error;
    }
    /* start parsing */
    int config_status = 0;
    for(size_t i = 0; i < array_len(arg->parse.config_files_base); ++i) {
        config_status |= arg_config_from_file(arg, array_at(arg->parse.config_files_base, i));
    }
    arg_parse_setref(arg);
    /* parse instream */
    arg->instream.is_config = false;
    TRYC(argstream_parse(arg, &arg->instream, quit_early));
    /* other stuff */
    if(arg->base.compgen_wordlist) {
        arg_help(arg);
        *quit_early = true;
        goto clean;
    }
    if(array_len(arg->instream.vals) < 1 && arg->base.show_help && !arg->parse.help.get_explicit) {
        arg_help(arg);
        *quit_early = true;
    } else if(!arg->parse.help.get && arg->n_pos_parsed < array_len(arg->pos.list)) {
        THROW("missing %zu positional arguments", array_len(arg->pos.list) - arg->n_pos_parsed);
    }
clean:
    argstream_free(&arg->instream);
    vso_free(&tmpstream.vals);
    /** NOTE: DO THIS OUTSIDE:
       if(*quit_early) {
           arg_free(&arg);
       }
    *************************/
    //printff("done argparse [%.*s]", SO_F(arg->base.program));
    return err;
error:
    //printff("error");
    arg_help(arg);
    ERR_CLEAN;
}

/*}}}*/

/* CONFIG FUNCTIONS {{{ */

void arg_config_error(struct Arg *arg, So line, size_t line_nb, So opt, ArgX *argx) {
    ASSERT_ARG(arg);
    if(arg->base.compgen_wordlist) return ; //0;
    if(line.str) {
        THROW_PRINT("config error on " F("line %zu", BOLD FG_MG_B) ":\n", line_nb + 1);
        if(!opt.str) {
            ERR_PRINTF("        %.*s:\n", SO_F(line));
            ERR_PRINTF("        ^");
        } else {
            So pre = so_ll(line.str, opt.str - line.str);
            So at = opt;
            So post = so_i0(line, so_len(pre) + so_len(at));
            ERR_PRINTF("        %.*s" F("%.*s", BOLD FG_RD_B) "%.*s\n", SO_F(pre), SO_F(at), SO_F(post));
            ERR_PRINTF("        %*s", (int)(opt.str - line.str), "");
            for(size_t i = 0; i < so_len(opt); ++i) {
            ERR_PRINTF(F("~", BOLD FG_RD_B));
            }
        }
        ERR_PRINTF("\n");
    }
    if(argx) {
        arg->parse.help.get = true;
        arg->parse.help.x = argx;
        arg_help(arg);
        arg->parse.help.get = false;
        arg->parse.help.x = 0;
    }
}

ErrDecl arg_config_from_str(struct Arg *arg, So text) {
    ASSERT_ARG(arg);
    int err = 0;
    size_t line_nb = 0;
    So line = {0}, opt = {0}, conf = text;
    bool pe = true;
    bool quit_early = false;
    if(!so_len(conf)) return 0;
    ArgX *argx = 0;
    ArgStream stream = {0};
    for(memset(&line, 0, sizeof(line)); so_splice(conf, &line, '\n'); ++line_nb) {
        if(!line.str) continue;
        line = so_trim(line);
        line = so_iE(line, so_find_ch(line, '#'));
        argx = 0;
        pe = true;
        if(!line.len) continue;
        //printff("LINE:%.*s", SO_F(line));
#if 1
        argstream_free(&stream);
        stream.is_config = true;
        for(memset(&opt, 0, sizeof(opt)); so_splice(line, &opt, '='); ) {
            array_push(stream.vals, so_trim(opt));
        }
        TRYC(argstream_parse(arg, &stream, &quit_early));
#else
        for(memset(&opt, 0, sizeof(opt)); so_splice(line, &opt, '='); ) {
            //printff(" OPT:%.*s, pe=%u",SO_F(opt), pe);
            if(!opt.str) continue;
            opt = so_trim(opt);
            if(!argx) {
                TRYC_P(pe, arg_parse_getopt_long(&arg->tables.opt, &argx, opt));
                if(argx->id == ARG_HELP) {
                    THROW_P(pe, "cannot configure help");
                } else if(argx->id == ARG_TRY_OPT) {
                    pe = false;
                    argx = 0;
                } else if(argx->attr.is_env) {
                    THROW_P(pe, "cannot configure env");
                } else if(argx->id == ARG_NONE) {
                    THROW_P(pe, "cannot configure non-value option");
                }
            } else {
#if 0
                    argx_parse(&parse, 
#else
                //printff("  setting value for [%.*s] : %.*s : id %u", SO_F(argx->info.opt), SO_F(opt), argx->id);
                switch(argx->id) {
                    case ARG_OPTION: {
                        ASSERT_ARG(argx->o);
                        ArgXTable *table = argx->o->table;
                        ArgX *x = 0;
                        TRYC_P(pe, arg_parse_getopt_long(table, &x, opt));
                        argx = x;
                    } break;
                    case ARG_BOOL: {
                        bool *b = argx->ref.b ? argx->ref.b : argx->val.b;
                        TRYC_P(pe, so_as_bool(opt, b));
                    } break;
                    case ARG_SSZ: {
                        ssize_t *z = argx->ref.z ? argx->ref.z : argx->val.z;
                        TRYC_P(pe, so_as_ssize(opt, z, 0));
                    } break;
                    case ARG_INT: {
                        int *i = argx->ref.i ? argx->ref.i : argx->val.i;
                        ssize_t z = 0;
                        TRYC_P(pe, so_as_ssize(opt, &z, 0));
                        *i = (int)z;
                    } break;
                    case ARG_FLOAT: {
                        double *f = argx->ref.f ? argx->ref.f : argx->val.f;
                        TRYC_P(pe, so_as_double(opt, f));
                    } break;
                    case ARG_COLOR: {
                        Color *c = argx->ref.c ? argx->ref.c : argx->val.c;
                        TRYC_P(pe, so_as_color(opt, c));
                    } break;
                    case ARG_STRING: {
                        So *s = argx->ref.s ? argx->ref.s : argx->val.s;
                        *s = opt;
                    } break;
                    case ARG_FLAG: {
                        bool *b = argx->ref.b ? argx->ref.b : argx->val.b;
                        *b = true;
                    } break;
                    case ARG_NONE: {
                        THROW_P(pe, "option cannot have a value");
                    } break;
                    case ARG_FLAGS: {
                        ASSERT_ARG(argx->o);
                        for(size_t i = 0; i < array_len(argx->o->list); ++i) {
                            ArgX *x = array_at(argx->o->list, i);
                            bool *b = x->ref.b ? x->ref.b : x->val.b;
                            *b = false;
                        }
                        for(So flag = {0}; so_splice(opt, &flag, arg->base.flag_sep); ) {
                            if(!flag.str) continue;
                            ArgX *x = 0;
                            TRYC_P(pe, arg_parse_getopt_long(argx->o->table, &x, flag));
                            bool *b = x->ref.b ? x->ref.b : x->val.b;
                            *b = true;
                        }
                    } break;
                    case ARG_VECTOR: {
                        VSo *v = argx->ref.v ? argx->ref.v : argx->val.v;
                        array_push(*v, opt);
                    } break;
                    case ARG_TRY_OPT:
                    case ARG_HELP:
                    case ARG__COUNT: ABORT(ERR_UNREACHABLE);
                }
#endif
                //printff(" GROUP %p", argx->group);
                //printff(" PARENT %p", argx->group ? argx->group->parent : 0);
                //printff(" ID %s", argx->group ? argx->group->parent ? arglist_str(argx->group->parent->id) : "" : 0);
            }
            /* check enum / option; TODO DRY */
            if(argx && argx->group && argx->group && argx->group->parent && argx->group->parent->id == ARG_OPTION) {
                if(argx->group->parent->ref.i) {
                    *argx->group->parent->ref.i = argx->e;
                } else if(argx->group->parent->val.i) {
                    *argx->group->parent->val.i = argx->e;
                }
            }
        }
#endif
parse_continue: ; /* semicolon to remove warning */
    }
    //arg_parse_setref(arg);
    return err;
error:
    if(pe) {
        err = -1;
        arg_config_error(arg, line, line_nb, opt, argx);
    }
    goto parse_continue;
}

/*}}}*/

/* FREEING FUNCTIONS {{{ */

void argx_table_free(ArgXTable *table) {
    ASSERT_ARG(table);
    targx_free(&table->lut);
    memset(table, 0, sizeof(*table));
}

void argx_group_free(ArgXGroup *group) {
    ASSERT_ARG(group);
    array_free(group->list);
    //if(group->parent) free(group->table);
    if(group->parent) {
        argx_table_free(group->table);
        free(group->table);
    }
    memset(group, 0, sizeof(*group));
}

void argx_group_free_array(ArgXGroup **group) {
    ASSERT_ARG(group);
    if(*group) {
        argx_group_free(*group);
        free(*group);
    }
    memset(group, 0, sizeof(*group));
}

void argx_free(ArgX *argx) {
    if(!argx) return;
    //if(argx->id == ARG_OPTION) printff("free [%.*s]", SO_F(argx->info.opt));
    ASSERT_ARG(argx);
    if((argx->id == ARG_OPTION || argx->id == ARG_FLAGS)) {
        argx_group_free(argx->o);
        free(argx->o);
    }
    if(argx->id == ARG_VECTOR) {
        array_free(*argx->val.v);
    }
    //if(argx->id == ARG_OPTION) printff("done [%.*s]", SO_F(argx->info.opt));
    memset(argx, 0, sizeof(*argx));
};


void arg_free(struct Arg **parg) {
    //printff("FREE ARGS");
    ASSERT_ARG(parg);
    Arg *arg = *parg;

    vargxgroup_free(&arg->groups);
    //array_free_set(arg->groups, ArgXGroup, (ArrayFree)argx_group_free_array);
    //array_free(arg->groups);
    argx_group_free(&arg->pos);

    argx_table_free(&arg->tables.opt);
    argx_table_free(&arg->tables.pos);

    vso_free(&arg->parse.config);
    vso_free(&arg->parse.config_files_base);
    vso_free(&arg->parse.config_files_expand);
    if(arg->base.rest_vec) vso_free(arg->base.rest_vec);
    free(*parg);
    *parg = 0;
    //printff("FREED ARGS");
}

/*}}}*/

void argx_builtin_env_compgen(ArgXGroup *group) {
    ASSERT_ARG(group);
    struct ArgX *x = argx_env(group, so("COMPGEN_WORDLIST"), so("Generate input for autocompletion"), false);
    argx_bool(x, &group->root->base.compgen_wordlist, 0);
    argx_bool_require_tf(x, true);
    //argx_func(x, 0, argx_callback_env_compgen, arg, true);
}

void argx_builtin_opt_help(ArgXGroup *group) {
    ASSERT_ARG(group);
    struct ArgX *x = argx_init(group, 'h', so("help"), so("print this help"));
    argx_help(x, group->root);
}

void argx_builtin_opt_fmtx(ArgX *x, So_Fx *fmt, So_Fx *ref) {
    struct ArgXGroup *g = 0;
    g=argx_opt(x, 0, 0);
      x=argx_init(g, 0, so("fg"), so("foreground"));
        argx_col(x, &fmt->fg, ref ? &ref->fg : 0);
      x=argx_init(g, 0, so("bg"), so("background"));
        argx_col(x, &fmt->bg, ref ? &ref->bg : 0);
      x=argx_init(g, 0, so("bold"), so("bold"));
        argx_bool(x, &fmt->bold, ref ? &ref->bold : 0);
      x=argx_init(g, 0, so("it"), so("italic"));
        argx_bool(x, &fmt->italic, ref ? &ref->italic : 0);
      x=argx_init(g, 0, so("ul"), so("underline"));
        argx_bool(x, &fmt->underline, ref ? &ref->underline : 0);
}

ArgXGroup *argx_builtin_opt_rice(ArgXGroup *group) {
    ASSERT_ARG(group);

    struct Arg *arg = group->root;
    struct ArgX *x = 0;
    struct ArgXGroup *g = 0, *o = 0;
    x=argx_init(group, 0, so("rice"), so("change the look & feel"));
      g=argx_opt(x, false, false);

    x=argx_init(g, 0, so("arg"), so("argument parser formatting"));
      o=argx_opt(x, false, false);

    x=argx_init(o, 0, so("program"), so("program name formatting"));
      argx_builtin_opt_fmtx(x, &arg->fmt.program, 0);

    x=argx_init(o, 0, so("group"), so("group formatting"));
      argx_builtin_opt_fmtx(x, &arg->fmt.group, 0);
    x=argx_init(o, 0, so("group-delim"), so("group delimiter formatting"));
      argx_builtin_opt_fmtx(x, &arg->fmt.group_delim, 0);

    x=argx_init(o, 0, so("pos"), so("positional formatting"));
      argx_builtin_opt_fmtx(x, &arg->fmt.pos, 0);
    x=argx_init(o, 0, so("short"), so("short option formatting"));
      argx_builtin_opt_fmtx(x, &arg->fmt.c, 0);
    x=argx_init(o, 0, so("long"), so("long option formatting"));
      argx_builtin_opt_fmtx(x, &arg->fmt.opt, 0);
    x=argx_init(o, 0, so("env"), so("environmental formatting"));
      argx_builtin_opt_fmtx(x, &arg->fmt.env, 0);
    x=argx_init(o, 0, so("desc"), so("description formatting"));
      argx_builtin_opt_fmtx(x, &arg->fmt.desc, 0);

    x=argx_init(o, 0, so("one"), so("one-of formatting"));
      argx_builtin_opt_fmtx(x, &arg->fmt.one_of, 0);
    x=argx_init(o, 0, so("one-set"), so("one-of set formatting"));
      argx_builtin_opt_fmtx(x, &arg->fmt.one_of_set, 0);
    x=argx_init(o, 0, so("one-delim"), so("one-of delimiter formatting"));
      argx_builtin_opt_fmtx(x, &arg->fmt.one_of_delim, 0);

    x=argx_init(o, 0, so("flag"), so("flag formatting"));
      argx_builtin_opt_fmtx(x, &arg->fmt.flag, 0);
    x=argx_init(o, 0, so("flag-set"), so("flag set formatting"));
      argx_builtin_opt_fmtx(x, &arg->fmt.flag_set, 0);
    x=argx_init(o, 0, so("flag-delim"), so("flag delimiter formatting"));
      argx_builtin_opt_fmtx(x, &arg->fmt.flag_delim, 0);

    x=argx_init(o, 0, so("type"), so("type formatting"));
      argx_builtin_opt_fmtx(x, &arg->fmt.type, 0);
    x=argx_init(o, 0, so("type-delim"), so("type delimiter formatting"));
      argx_builtin_opt_fmtx(x, &arg->fmt.type_delim, 0);

    x=argx_init(o, 0, so("val"), so("value formatting"));
      argx_builtin_opt_fmtx(x, &arg->fmt.val, 0);
    x=argx_init(o, 0, so("val-delim"), so("value delimiter formatting"));
      argx_builtin_opt_fmtx(x, &arg->fmt.val_delim, 0);

    return g;
}

void argx_builtin_opt_source(struct ArgXGroup *group, So source) {
    ASSERT_ARG(group);
    struct Arg *arg = group->root;
    array_push(arg->parse.config_files_base, source);
    struct ArgX *x = targx_get(&group->table->lut, so("source"));
    if(!x) {
        x=argx_init(group, 0, so("source"), so("source other config files"));
          argx_vstr(x, &arg->parse.config_files_base, 0);
        x=argx_init(group, 0, so("try-opt"), so("try parsing an option, especially useful for within sources, to suppress errors"));
          argx_try_parse(x);
    }
}

