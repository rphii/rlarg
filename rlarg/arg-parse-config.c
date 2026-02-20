#include "arg-parse-config.h"
#include "arg-parse.h"

#define TODO_WARN  \
    printff(F("TODO WARN", FG_YL BOLD))

#define TODO_ERROR  \
    printff(F("TODO ERROR", FG_RD BOLD))

Argx *arg_parse_config_get_hierarchy(Arg_Parse_Config *p) {
    So *tmp = &p->tmp_full_hierarchy;
    so_clear(tmp);
    if(so_len(p->section)) {
        so_extend(tmp, p->section);
        so_push(tmp, '.');
    }
    so_extend(tmp, p->hierarchy);
    p->stream.error_id = 0;
    Argx *argx = arg_parse_hierarchy(p->arg, &p->stream, *tmp, 0);
    //printff("FULL HIER: %.*s -> %p", SO_F(p->tmp_full_hierarchy), argx);
    return argx;
}

int arg_parse_config_assign_file_named(Arg_Parse_Config *p, So path, bool in_array) {
    if(!p->argx) {
        //TODO_WARN;
        return -1;
    }
    /* construct path */
    So content = SO;
    /* TODO: the tmp_file_path is lost to the operator of the arg parser (it is technically a source..) */
    so_path_join(&p->tmp_file_path, so_get_dir(p->stream.source.path), path);
    /* read file */
    //printff("FILE NAMED %.*s", SO_F(p->tmp_file_path));
    if(so_file_read(p->tmp_file_path, &content)) {
        TODO_ERROR;
        return -1;
    }
    /* now parse */
    vso_push(&p->arg->builtin.sources_content, content);
    if(in_array) {
        p->stream.carg = content;
        arg_parse_argx(p->arg, &p->stream, p->argx, content);
    } else {
        for(So line = SO; so_splice(content, &line, '\n'); ) {
            if(so_is_zero(line)) continue;
            line = so_trim(line);
            if(!so_len(line)) continue;
            p->stream.carg = line;
            arg_parse_argx(p->arg, &p->stream, p->argx, line);
        }
    }
    return 0;
}

int arg_parse_config_assign_file_auto(Arg_Parse_Config *p, bool in_array) {
    arg_parse_config_assign_file_named(p, p->tmp_full_hierarchy, in_array);
    return 0;
}

int arg_parse_config_assign_string(Arg_Parse_Config *p, So val, bool in_array) {
    if(!p->argx) {
        //TODO_WARN;
        return -1;
    }
    //printff("STRING %.*s", SO_F(val));
    p->stream.carg = val;
    arg_parse_argx(p->arg, &p->stream, p->argx, so_clone(val));
    return 0;
}

int arg_parse_config_assign_other(Arg_Parse_Config *p, So val, bool in_array) {
    if(!p->argx) {
        //TODO_WARN;
        return -1;
    }
    //printff("OTHER %.*s", SO_F(val));
    p->stream.carg = val;
    arg_parse_argx(p->arg, &p->stream, p->argx, val);
    return 0;
}

void arg_parse_config_shift(Arg_Parse_Config *p, Arg_Parse_Config_Head *head, size_t n) {
    size_t len = so_len(head->so);
    if(len < n) {
        ABORT(ERR_UNREACHABLE("can't shift this much"));
    }
    size_t n_newline = so_find_ch(so_iE(head->so, n), '\n');
    if(n_newline < n) ++p->stream.source.line_number;
    so_shift(&head->so, n);
}

/* return true on match */
bool arg_parse_config_ch(Arg_Parse_Config *p, Arg_Parse_Config_Head *head, char c) {
    ASSERT_ARG(p);
    if(!head->so.len) return false;
    bool result = (bool)(*head->so.str == c);
    if(result) {
        arg_parse_config_shift(p, head, 1);
    }
    return result;
}

/* return true on match */
bool arg_parse_config_any(Arg_Parse_Config *p, Arg_Parse_Config_Head *head, char *s) {
    ASSERT_ARG(p);
    if(!head->so.len) return false;
    char *result = strchr(s, *head->so.str);
    if(result && *result) {
        arg_parse_config_shift(p, head, 1);
        return true;
    }
    return false;
}

void arg_parse_config_ws(Arg_Parse_Config *p, Arg_Parse_Config_Head *head) {
    ASSERT_ARG(p);
    while(arg_parse_config_any(p, head, " \t\v\n\r")) {}
}

void arg_parse_config_ws_no_newline(Arg_Parse_Config *p, Arg_Parse_Config_Head *head) {
    ASSERT_ARG(p);
    while(arg_parse_config_any(p, head, " \t\v\r")) {}
}

void arg_parse_config_other(Arg_Parse_Config *p, Arg_Parse_Config_Head *head, So *val, bool in_array) {
    ASSERT_ARG(p);
    ASSERT_ARG(head);
    ASSERT_ARG(val);
    size_t len = so_find_ch(head->so, '\n');
    size_t len_to_comment = SIZE_MAX;
    size_t len_to_comma = SIZE_MAX;
    if(in_array && len) {
        So check_for_comma = so_iE(head->so, len);
        len_to_comma = so_find_ch(check_for_comma, ',');
    }
    if(len < so_len(head->so)) {
        len_to_comment = so_find_ch(head->so, '#');
        size_t less = len_to_comment < len_to_comma ? len_to_comment : len_to_comma;
        if(len < less) less = len;
        *val = so_trim(so_iE(head->so, less));
    }
    if(len_to_comment != SIZE_MAX && len_to_comma != SIZE_MAX && len_to_comma < len_to_comment) {
        arg_parse_config_shift(p, head, len_to_comma);
    } else {
        arg_parse_config_shift(p, head, len);
    }
}

bool arg_parse_config_hierarchy(Arg_Parse_Config *p, Arg_Parse_Config_Head *head) {
    ASSERT_ARG(p);
    ASSERT_ARG(head);
    bool ok = true;
    arg_parse_config_ws(p, head);
    so_clear(&p->hierarchy);
    while(head->so.len) {
        arg_parse_config_ws(p, head);
        if(arg_parse_config_ch(p, head, '\n')) { ok = false; break; }
        if(arg_parse_config_ch(p, head, '=')) break;
        so_push(&p->hierarchy, so_at0(head->so));
        arg_parse_config_shift(p, head, 1);
    }
    if(ok) {
        if(so_len(p->hierarchy)) {
            //printff("HIERARCHY %.*s", SO_F(p->hierarchy));
        }
    } else {
        printff(F("TODO ERROR", FG_RD BOLD));
    }
    return ok;
}

bool arg_parse_config_string(Arg_Parse_Config *p, Arg_Parse_Config_Head *head, bool in_array) {
    ASSERT_ARG(p);
    ASSERT_ARG(head);
    bool ok = true;
    Arg_Parse_Config_Head q = *head;
    So val = SO;
    if(!arg_parse_config_ch(p, &q, '"')) return false;
    size_t n_unescaped = 0;
    if(so_fmt_unescape(&val, q.so, '"', &n_unescaped)) return false;
    arg_parse_config_shift(p, &q, n_unescaped);
    if(!arg_parse_config_ch(p, &q, '"')) return false;
    arg_parse_config_assign_string(p, val, in_array);
    so_free(&val);
    if(ok) {
        *head = q;
    }
    return ok;
}

bool arg_parse_config_file(Arg_Parse_Config *p, Arg_Parse_Config_Head *head, bool in_array) {
    ASSERT_ARG(p);
    ASSERT_ARG(head);
    bool ok = true;
    Arg_Parse_Config_Head q = *head;
    if(!arg_parse_config_ch(p, &q, 'f')) return false;
    if(!arg_parse_config_ch(p, &q, 'i')) return false;
    if(!arg_parse_config_ch(p, &q, 'l')) return false;
    if(!arg_parse_config_ch(p, &q, 'e')) return false;
    so_clear(&p->file);
    bool used_path = false;
    if(arg_parse_config_ch(p, &q, '(')) {
        used_path = true;
        while(q.so.len) {
            if(arg_parse_config_ch(p, &q, '\n')) { ok = false; TODO_ERROR; break; }
            if(arg_parse_config_ch(p, &q, ')')) break;
            so_push(&p->file, so_at0(q.so));
            arg_parse_config_shift(p, &q, 1);
        }
    }
    if(!q.so.len) { ok = false; TODO_ERROR; } // TODO: add these !head->len checks more throughout */
    if(ok) {
        *head = q;
        if(used_path) arg_parse_config_assign_file_named(p, p->file, in_array);
        else arg_parse_config_assign_file_auto(p, in_array);
    }
    return ok;
}

bool arg_parse_config_value(Arg_Parse_Config *p, Arg_Parse_Config_Head *head, bool in_array) {
    bool ok = false;
    bool shift = false;
    So other = SO;
    Arg_Parse_Config_Head q = *head;
    arg_parse_config_ws(p, &q);
    if(arg_parse_config_file(p, &q, in_array)) {
        ok = true;
    } else if(arg_parse_config_string(p, &q, in_array)) {
        ok = true;
    } else {
        arg_parse_config_other(p, &q, &other, in_array);
        if(so_len(other)) {
            arg_parse_config_assign_other(p, other, in_array);
            ok = true;
        }
        shift = true;
    }
    arg_parse_config_ws(p, &q);
    if(ok || shift) {
        *head = q;
    }
    return ok;
}

bool arg_parse_config_section(Arg_Parse_Config *p, Arg_Parse_Config_Head *head) {
    ASSERT_ARG(p);
    ASSERT_ARG(head);
    bool ok = true;
    arg_parse_config_ws(p, head);
    if(!arg_parse_config_ch(p, head, '[')) return false;
    so_clear(&p->section);
    arg_parse_config_ws_no_newline(p, head);
    while(head->so.len) {
        arg_parse_config_ws_no_newline(p, head);
        if(arg_parse_config_ch(p, head, '\n')) { ok = false; break; }
        if(arg_parse_config_ch(p, head, ']')) { break; }
        so_push(&p->section, so_at0(head->so));
        arg_parse_config_shift(p, head, 1);
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

bool arg_parse_config_array(Arg_Parse_Config *p, Arg_Parse_Config_Head *head) {
    ASSERT_ARG(p);
    ASSERT_ARG(head);
    bool ok = true;
    Arg_Parse_Config_Head q = *head;
    if(!arg_parse_config_ch(p, &q, '[')) return false;
    printff("PARSE ARRAY vvvvvvvv");
    arg_parse_config_ws(p, &q);
    size_t values_parsed_old = 0;
    size_t values_parsed_now = 0;
    while(q.so.len) {
        /* can we expect an end ? */
        arg_parse_config_ws(p, &q);
        //printff("Q[%.*s]",SO_F(q));
        if(values_parsed_now > values_parsed_old) {
            if(!arg_parse_config_ch(p, &q, ',')) {
                if(arg_parse_config_ch(p, &q, ']')) break;
                else {
                    ok = false;
                    TODO_ERROR;
                    break;
                }
            } else {
                values_parsed_old = values_parsed_now;
            }
        } else {
            if(arg_parse_config_ch(p, &q, ']')) break;
            if(arg_parse_config_ch(p, &q, ',')) {
                ok = false;
                TODO_ERROR;
                break;
            }
            /* try parse a value */
            values_parsed_old = values_parsed_now;
            //printff("1EAD:%.*s",SO_F(q));
            if(arg_parse_config_value(p, &q, true)) {
                ++values_parsed_now;
            }
        }
        //printff("HEAD:%.*s",SO_F(q));
    }
    printff("PARSE ARRAY ^^^^^^^^ --- ok? %u", ok);
    if(ok) {
        *head = q;
    }
    //printff("REST:[%.*s]",SO_F(*head));
    return ok;
}

bool arg_parse_config_config(Arg_Parse_Config *p, Arg_Parse_Config_Head *head) {
    ASSERT_ARG(p);
    ASSERT_ARG(head);
    bool ok = true;
    Arg_Parse_Config_Head q = *head;
    arg_parse_config_ws(p, &q);
    if(arg_parse_config_array(p, &q)) {
    } else if(arg_parse_config_value(p, &q, false)) {
    } else {
        ok = false;
        TODO_ERROR;
    }
    arg_parse_config_ws(p, &q);
    if(ok) {
        *head = q;
    }
    return ok;
}

bool arg_parse_config_settings(Arg_Parse_Config *p, Arg_Parse_Config_Head *head) {
    ASSERT_ARG(p);
    ASSERT_ARG(head);
    bool ok = true;
    So inspect = SO;
    Arg_Parse_Config_Head q = *head;
    arg_parse_config_other(p, &q, &inspect, false);
    if(!so_len(inspect)) return false;
    Arg_Parse_Config_Head inspected = { .so = inspect, .line_number = head->line_number };
    if(!arg_parse_config_hierarchy(p, &inspected)) return false;

    /* now we have the full hierarchy, assign argx */
    //printff(">>>UPDATE HIERARCHY");
    p->argx = arg_parse_config_get_hierarchy(p);

    /* start of inspected now points to where we want to continue! */
    arg_parse_config_shift(p, head, inspected.so.str - head->so.str);

    arg_parse_config_config(p, head);

    return ok;
}

int arg_parse_config(struct Arg *arg, So config, So path) {
    arg->help.error = 0;
    arg->help.last = 0;

    int status = 0;

    So comment = SO;

    Arg_Parse_Config_Head head = {
        .so = config,
        .line_number = 1,
    };

    Arg_Parse_Config p = {
        .arg = arg,
        .stream.is_config = true,
        .stream.source.path = path,
        .stream.source.line_number = 1,
    };

    while(head.so.len) {
        if(!arg_parse_config_section(&p, &head)) {
            if(!arg_parse_config_settings(&p, &head)) {
                arg_parse_config_other(&p, &head, &comment, false);
            }
        }
    }

    arg_stream_free(&p.stream);

    so_free(&p.tmp_file_path);
    so_free(&p.tmp_full_hierarchy);
    so_free(&p.file);
    so_free(&p.hierarchy);
    so_free(&p.section);

    return status;
}


