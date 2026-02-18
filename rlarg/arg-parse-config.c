#include "arg-parse-config.h"
#include "arg-parse.h"

#define TODO_ERROR  \
    printff(F("TODO ERROR", FG_RD BOLD))

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

void arg_parse_config_ws_no_newline(Arg_Parse_Config *p, So *head) {
    ASSERT_ARG(p);
    while(arg_parse_config_any(p, head, " \t\v\r")) {}
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

void arg_parse_config_other(Arg_Parse_Config *p, So *head, So *val) {
    ASSERT_ARG(p);
    ASSERT_ARG(head);
    ASSERT_ARG(val);
    size_t len = so_find_ch(*head, '\n');
    if(len < so_len(*head)) {
        *val = so_split_ch(so_ll(head->str, len), '#', 0);
        so_shift(head, len);
    }
}

bool arg_parse_config_hierarchy(Arg_Parse_Config *p, So *head) {
    ASSERT_ARG(p);
    ASSERT_ARG(head);
    bool ok = true;
    arg_parse_config_ws(p, head);
    so_clear(&p->hierarchy);
    while(head->len) {
        arg_parse_config_ws(p, head);
        if(arg_parse_config_ch(p, head, '\n')) { ok = false; break; }
        if(arg_parse_config_ch(p, head, '=')) break;
        so_push(&p->hierarchy, so_at0(*head));
        so_shift(head, 1);
    }
    if(ok) {
        if(so_len(p->hierarchy)) {
            printff("HIERARCHY %.*s /// %.*s", SO_F(p->hierarchy), SO_F(*head));
        }
    } else {
        printff(F("TODO ERROR", FG_RD BOLD));
    }
    return ok;
}

bool arg_parse_config_string(Arg_Parse_Config *p, So *head) {
    ASSERT_ARG(p);
    ASSERT_ARG(head);
    bool ok = true;
    So q = *head;
    So val = SO;
    if(!arg_parse_config_ch(p, &q, '"')) return false;
    if(so_fmt_unescape(&val, q, '"')) return false;
    so_shift(&q, so_len(val));
    if(!arg_parse_config_ch(p, &q, '"')) return false;
    printff("STRING %.*s", SO_F(val));
    so_free(&val);
    if(ok) {
        *head = q;
    }
    return ok;
}

bool arg_parse_config_file(Arg_Parse_Config *p, So *head) {
    ASSERT_ARG(p);
    ASSERT_ARG(head);
    bool ok = true;
    So q = *head;
    if(!arg_parse_config_ch(p, &q, 'f')) return false;
    if(!arg_parse_config_ch(p, &q, 'i')) return false;
    if(!arg_parse_config_ch(p, &q, 'l')) return false;
    if(!arg_parse_config_ch(p, &q, 'e')) return false;
    so_clear(&p->file);
    if(arg_parse_config_ch(p, &q, '(')) {
        while(q.len) {
            if(arg_parse_config_ch(p, &q, '\n')) { ok = false; TODO_ERROR; break; }
            if(arg_parse_config_ch(p, &q, ')')) break;
            so_push(&p->file, so_at0(q));
            so_shift(&q, 1);
        }
    }
    if(!q.len) { ok = false; TODO_ERROR; } // TODO: add these !head->len checks more throughout */
    if(ok) {
        *head = q;
        printff("FILE %.*s", SO_F(p->file));
    }
    return ok;
}

bool arg_parse_config_value(Arg_Parse_Config *p, So *head) {
    bool ok = false;
    So other = SO;
    So q = *head;
    arg_parse_config_ws(p, &q);
    if(arg_parse_config_file(p, &q)) {
        ok = true;
    } else if(arg_parse_config_string(p, &q)) {
        ok = true;
    } else {
        arg_parse_config_other(p, &q, &other);
        printff("OTHER %.*s", SO_F(other));
        ok = true;
    }
    if(ok) {
        *head = q;
    }
    return ok;
}

bool arg_parse_config_settings(Arg_Parse_Config *p, So *head) {
    ASSERT_ARG(p);
    ASSERT_ARG(head);
    bool ok = true;
    So inspect = SO;
    arg_parse_config_other(p, head, &inspect);
    if(!so_len(inspect)) return false;
    So inspected = inspect;
    if(!arg_parse_config_hierarchy(p, &inspected)) return false;

    /* start of inspected now points to where we want to continue! */
    so_shift(head, inspected.str - head->str);

    So val = SO;
    arg_parse_config_value(p, head);

    return ok;
}

bool arg_parse_config_section(Arg_Parse_Config *p, So *head) {
    ASSERT_ARG(p);
    ASSERT_ARG(head);
    bool ok = true;
    arg_parse_config_ws(p, head);
    if(!arg_parse_config_ch(p, head, '[')) return false;
    so_clear(&p->section);
    arg_parse_config_ws_no_newline(p, head);
    while(head->len) {
        arg_parse_config_ws_no_newline(p, head);
        if(arg_parse_config_ch(p, head, '\n')) { ok = false; break; }
        if(arg_parse_config_ch(p, head, ']')) { break; }
        so_push(&p->section, so_at0(*head));
        so_shift(head, 1);
    }
    arg_parse_config_ws_no_newline(p, head);
    if(!arg_parse_config_ch(p, head, '\n')) { ok = false; }
    if(ok) {
        printff("SECTION %.*s", SO_F(p->section));
    } else {
        printff(F("TODO ERROR", FG_RD BOLD));
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

    So comment = SO;
    So head = config;
    Arg_Parse_Config p = {
        .arg = arg,
    };

    while(head.len) {
        if(!arg_parse_config_section(&p, &head)) {
            if(!arg_parse_config_settings(&p, &head)) {
                arg_parse_config_other(&p, &head, &comment);
            }
        }
    }

    arg_stream_free(&stream_config);
    so_free(&sanitized_file_path);
    so_free(&sanitized_string);
    argx_so_free(&xso);

    return status;
}


