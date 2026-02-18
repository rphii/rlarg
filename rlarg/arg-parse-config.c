#include "arg-parse-config.h"
#include "arg-parse.h"

/* return true on match */
bool arg_parse_config_ch(Arg_Parse_Config *p, So *head, char c) {
    ASSERT_ARG(p);
    if(!head->len) return false;
    bool result = (bool)(*head->str == c);
    if(result) {
        so_shift(head, 1);
    }
    return result;
}

/* return true on match */
bool arg_parse_config_any(Arg_Parse_Config *p, So *head, char *s) {
    ASSERT_ARG(p);
    if(!head->len) return false;
    char *result = strchr(s, *head->str);
    if(result && *result) {
        so_shift(head, 1);
        return true;
    }
    return false;
}

void arg_parse_config_ws(Arg_Parse_Config *p, So *head) {
    ASSERT_ARG(p);
    while(arg_parse_config_any(p, head, " \t\v\n\r")) {}
}

/* return true on successful parse */
bool arg_parse_config_value_string(Arg_Parse_Config *p, So *head, So *val) {
    ASSERT_ARG(p);
    ASSERT_ARG(val);
    So q = *head;
    if(!arg_parse_config_ch(p, &q, '"')) goto invalid;
    if(so_fmt_unescape(val, q, '"')) goto invalid;
    if(!arg_parse_config_ch(p, &q, '"')) goto invalid;
    *head = q;
    return true;
invalid:
    return false;
}

#if 0
/* return true on successful parse */
bool arg_parse_config_value_file(Arg_Parse_Config *p, So *head, So *val) {
    ASSERT_ARG(p);
    ASSERT_ARG(val);
    bool ok = true;
    So q = *head;
    if(!arg_parse_config_ch(p, &q, 'f')) { ok = false; goto invalid; }
    if(!arg_parse_config_ch(p, &q, 'i')) { ok = false; goto invalid; }
    if(!arg_parse_config_ch(p, &q, 'l')) { ok = false; goto invalid; }
    if(!arg_parse_config_ch(p, &q, 'e')) { ok = false; goto invalid; }
    So path = SO;
    Argx_So xso = {0};
    /* check if path is given */
    if(arg_parse_config_ch(p, &q, '(')) {
        bool found_end = false;
        path = *head;
        path.len = 0;
        while(head->len) {
            /* check for ending */
            found_end = arg_parse_config_ch(p, &q, ')');
            if(found_end) break;
            /* otherwise, collect to path */
            so_shift(head, 1);
            ++path.len;
        }
        if(!found_end) { ok = false; goto invalid; }
    } else {
        /* assume from argx */
        argx_so(&xso, p->argx, true, false);
        so_extend(&path, xso.hierarchy);
        so_extend(&path, p->argx->opt);
    }
    
    *head = q;
invalid:
    argx_so_free(&xso);
    so_free(&path);
    return ok;
}

/* return true on successful parse */
bool arg_parse_config_value_any(Arg_Parse_Config *p, So *head, So *val) {
    ASSERT_ARG(p);
    ASSERT_ARG(val);
    So q = *head;
    So to_line_end = so_split_ch(q, '\n', &q);
    *val = so_trim(so_split_ch(to_line_end, '#', 0));
    *head = q;
    return true;
}

bool arg_parse_config_value(Arg_Parse_Config *p, So *val);
#endif

bool arg_parse_config_section(Arg_Parse_Config *p, So *head) {
    ASSERT_ARG(p);
    ASSERT_ARG(head);
    bool ok = true;
    arg_parse_config_ws(p, head);
    if(!arg_parse_config_ch(p, head, '[')) return ok;
    arg_parse_config_ws(p, head);
    while(head->len) {
        arg_parse_config_ws(p, head);
        if(arg_parse_config_ch(p, head, '\n')) { ok = false; break; }
        if(arg_parse_config_ch(p, head, ']')) break;
        so_push(&p->section, so_at0(*head));
    }
    if(!ok) {
        printff(F("TODO ERROR", FG_RD BOLD));
    }
    return ok;
}

bool arg_parse_config_other(Arg_Parse_Config *p, So *head) {
    ASSERT_ARG(p);
    ASSERT_ARG(head);
    bool ok = true;
    size_t len = so_find_ch(*head, '\n');
    So val = SO;
    if(len < so_len(*head)) {
        val = so_split_ch(so_ll(head->str, len), '#', 0);
        if(val.len) {
            printff("OTHER %.*s", SO_F(val));
        }
        so_shift(head, len);
    }
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

    So head = config;
    Arg_Parse_Config p = {
        .arg = arg,
    };

    while(head.len) {
        arg_parse_config_section(&p, &head);
        arg_parse_config_ws(&p, &head);
        arg_parse_config_other(&p, &head);
    }

    arg_stream_free(&stream_config);
    so_free(&sanitized_file_path);
    so_free(&sanitized_string);
    argx_so_free(&xso);

    return status;
}


