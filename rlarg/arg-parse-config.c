#include "arg-parse-config.h"
#include "arg-parse.h"

/* return true on match */
bool arg_parse_config_ch(Arg_Parse_Config *p, char c) {
    ASSERT_ARG(p);
    if(!p->head.len) return false;
    bool result = (bool)(*p->head.str == c);
    if(result) {
        so_shift(&p->head, 1);
    }
    return result;
}

/* return true on match */
bool arg_parse_config_any(Arg_Parse_Config *p, char *s) {
    ASSERT_ARG(p);
    if(!p->head.len) return false;
    char *result = strchr(s, *p->head.str);
    if(result && *result) {
        so_shift(&p->head, 1);
        return true;
    }
    return false;
}

void arg_parse_config_ws(Arg_Parse_Config *p) {
    ASSERT_ARG(p);
    while(arg_parse_config_any(p, " \t\v\n\r")) {}
}

/* return true on successful parse */
bool arg_parse_config_value_string(Arg_Parse_Config *p, So *val) {
    ASSERT_ARG(p);
    ASSERT_ARG(val);
    Arg_Parse_Config q = *p;
    if(!arg_parse_config_ch(&q, '"')) goto invalid;
    if(so_fmt_unescape(val, q.head, '"')) goto invalid;
    if(!arg_parse_config_ch(&q, '"')) goto invalid;
    p->head = q.head;
    return true;
invalid:
    return false;
}

/* return true on successful parse */
bool arg_parse_config_value_file(Arg_Parse_Config *p, So *val) {
    ASSERT_ARG(p);
    ASSERT_ARG(val);
    bool ok = true;
    Arg_Parse_Config q = *p;
    if(!arg_parse_config_ch(&q, 'f')) { ok = false; goto invalid; }
    if(!arg_parse_config_ch(&q, 'i')) { ok = false; goto invalid; }
    if(!arg_parse_config_ch(&q, 'l')) { ok = false; goto invalid; }
    if(!arg_parse_config_ch(&q, 'e')) { ok = false; goto invalid; }
    So path = SO;
    Argx_So xso = {0};
    /* check if path is given */
    if(arg_parse_config_ch(&q, '(')) {
        bool found_end = false;
        path = q.head;
        path.len = 0;
        while(q.head.len) {
            /* check for ending */
            found_end = arg_parse_config_ch(&q, ')');
            if(found_end) break;
            /* otherwise, collect to path */
            so_shift(&q.head, 1);
            ++path.len;
        }
        if(!found_end) { ok = false; goto invalid; }
    } else {
        /* assume from argx */
        argx_so(&xso, p->argx, true, false);
        so_extend(&path, xso.hierarchy);
        so_extend(&path, p->argx->opt);
    }
    
    p->head = q.head;
invalid:
    argx_so_free(&xso);
    so_free(&path);
    return ok;
}

/* return true on successful parse */
bool arg_parse_config_value_any(Arg_Parse_Config *p, So *val) {
    ASSERT_ARG(p);
    ASSERT_ARG(val);
    Arg_Parse_Config q = *p;
    So to_line_end = so_split_ch(q.head, '\n', &q.head);
    *val = so_trim(so_split_ch(to_line_end, '#', 0));
    p->head = q.head;
    return true;
}

bool arg_parse_config_value(Arg_Parse_Config *p, So *val);

/* return true on successful parse */
bool arg_parse_config_value_array(Arg_Parse_Config *p) {
    ASSERT_ARG(p);
    bool ok = true;
    Arg_Parse_Config q = *p;
    if(!arg_parse_config_ch(&q, '[')) { ok = false; goto invalid; }
    q.inside_array = true;
    bool have_end = false;
    bool have_value = false;
    So val = SO;
    while(q.head.len) {
        /* check for end */
        have_end = arg_parse_config_ch(&q, ']');
        if(have_end) break;
        /* parse anything */
        if(!arg_parse_config_value(&q, &val)) { ok = false; goto invalid; }
        bool is_comma = so_at0(val) == ',';
        if(is_comma) {
            if(!have_value) { ok = false; goto invalid; }
            else arg_parse_config_ch(&q, ',');
        }
        if(so_atE(val) == ']') {
            have_end = true;
            val = so_iE(val, so_len(val) - 1);
        }
        printff(">>> ARRAY VALUE: %.*s :: HAVE END %u", SO_F(val), have_end);
        getchar();
        so_free(&val);
        if(have_end) break;
    }
    if(!have_end) { ok = false; goto invalid; }
    p->head = q.head;
invalid:
    return ok;
}

bool arg_parse_config_value(Arg_Parse_Config *p, So *val) {
    bool ok = true;
    Arg_Parse_Config q = *p;
    if(!q.inside_array && arg_parse_config_value_array(&q)) {
    } else if(arg_parse_config_value_string(&q, val)) {
        printff(">>> STRING VALUE: %.*s", SO_F(*val));
    } else if(arg_parse_config_value_file(&q, val)) {
        printff(">>> FILE VALUE: %.*s", SO_F(*val));
    } else if(arg_parse_config_value_any(&q, val)) {
        printff(">>> ANY VALUE: %.*s", SO_F(*val));
    }
    p->head = q.head;
invalid:
    return ok;
}

int arg_parse_config(struct Arg *arg, So config, So path) {
    arg->help.error = 0;
    arg->help.last = 0;
    Arg_Stream stream_config = {
        .source.path = path,
        .is_config = true,
    };
    int line_number = 1;
    int status = 0;
    So sanitized_string = SO;
    So sanitized_file_path = SO;
    Argx *argx = 0;
    Argx_So xso = {0};
    //Arg_Parse_Config cfg = {
    //    .stream_config = &stream_config,
    //    .all_lines = config
    //};

#if 0
    So_Switches config_cases = So_Switches(
        ARG_CONFIG_FILE, so("file"),
        ARG_CONFIG_STRING, so("\""),
        ARG_CONFIG_ARRAY_OPEN, so("["),
        ARG_CONFIG_ARRAY_CLOSE, so("]"),
        );
#endif

    for(So line = SO; so_splice(config, &line, '\n'); ++line_number) {

        /* get one line and split on '=' */
        if(so_is_zero(line)) continue;

        line = so_trim(so_split_ch(line, '#', 0));
        if(!so_len(line)) continue;

        So value, hierarchy = so_trim(so_split_ch(line, '=', &value));
        value = so_trim(value);
        stream_config.carg = value; /* set early, to have context for potential erros */
        stream_config.source.line_number = line_number;

        so_clear(&sanitized_string);

        printff("LINE: [%.*s]",SO_F(line));

        /* get argx from hierarchy */
        argx = arg_parse_hierarchy(arg, &stream_config, hierarchy, 0);
        if(!argx) {
            Argx pseudo = { .opt = hierarchy };
            arg_parse_error(arg, &stream_config, ARG_PARSE_ERROR_HIERARCHY_OPTION_CONFIG, &pseudo);
            status = -1;
            continue;
        }

        Arg_Parse_Config p = {
            .argx = argx,
            .arg = arg,
            .head = so_ll(value.str, config.str - value.str),
        };
        So san = SO;
        if(!arg_parse_config_value(&p, &san)) {
            Argx pseudo = { .opt = hierarchy };
            arg_parse_error(arg, &stream_config, ARG_PARSE_ERROR_HIERARCHY_OPTION_CONFIG, &pseudo);
            status = -1;
            continue;
        }
        //line = so_ll(san.str + san.len, 0);
        so_free(&san);
        

        /* values:
         * ANY        -> single line, always
         * STRING '"' -> single line, always
         * FILE       -> single line, always
         * FILE(path) -> single line, always
         * [          -> sinlge or multi line, expect value, then comment comma, and some time later ]
         * */

#if 0
        if(so_at0(value) == '[') {
            // TODO
        }
        if(so_at0(value) == ']') {
            // TODO
        }

        for(;;) {

            value = so_trim(value);

            status |= arg_parse_config_value(arg, &cfg, argx, &value);
            printff(" *  SHIFTED VALUE [%.*s] SANITIZED [%.*s]", SO_F(value), SO_F(cfg.sanitized_string));

            if(cfg.id == ARG_PARSE_CONFIG_ARRAY) {

                //while(cfg.id == ARG_PARSE_CONFIG_ARRAY && so_len(value));

            } else {
                break;
            }

        } 
#endif

#if 0
        if(cfg.id == ARG_PARSE_CONFIG_ARRAY) {
            Argx pseudo = { .opt = line };
            arg_parse_error(arg, &stream_config, ARG_PARSE_ERROR_MISSING_ARRAY_DELIM, &pseudo);
            status = -1;
            continue;
        }
#endif

#if 0
        /* parse */
        if(request_parse) {
            //printff(">>> CARG [%.*s]",SO_F(stream_config.carg));
            stream_config.source.line_number = line_number;
            arg_parse_argx(arg, &stream_config, argx, stream_config.carg);
        }
        arg_stream_clear(&stream_config);
#endif

    }

    arg_stream_free(&stream_config);
    so_free(&sanitized_file_path);
    so_free(&sanitized_string);
    argx_so_free(&xso);

    return status;
}


