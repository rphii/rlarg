# rlarg

Argument parser for C.

## Dependencies

- [rphii/rlc](https://github.com/rphii/rlc) - core helpers
- [rphii/rlso](https://github.com/rphii/rlso) - string object

## Features

**Core**

- directly assign parsed values to your variables
- positional values, optional values and environmental values
- supports `bool`, `int`, `ssize_t`, `So` (string), `Color`, `enum`, `flags` (toggle), `group` (sub-options)
- supports vectors: `bool`, `int`, `ssize_t`, `So`, `Color`
- supports catching-all via: `rest` (vector of `So`)
- disables all colors when piping (`isatty() == 0`)

**Builtin**

- `argx_builtin_opt_help`: expansive help
- `argx_builtin_rice`: enable colors
- `argx_builtin_env_compgen`: auto completion support (see also [`bash/rlc`](bash/rlc))
- `argx_builtin_env_nocolor`: explicit `NOCOLOR` support 
- `argx_builtin_opt_source`: config loading support (+ config can load other configs, and so on)
- `arg_enable_config_print`: config generation support

**Miscellaneous**

- `argx_callback`: provides support for custom callback (e.g. custom parser)
- `argx_attr_hide`: hides a value in help listing (e.g. API key)
- `argx_attr_configurable`: control if value can be configured via config file

## Adding an internal type

1. [`argx.h`](rlarg/argx.h) extend enum and union:
    - `Argx_Type_List`
    - `Argx_Value_Union`
2. [`argx.c`](rlarg/argx.c) extend core: (expand switches)
    - `argx_free`
    - `argx_so`
3. [`arg-parse.c`](rlarg/arg-parse.c) specify conversion functions: (expand array of callbacks)
    - `static_parse_argx_single_cbs`
    - `static_parse_argx_vector_vals_cbs`
    - `static_parse_argx_vector_cbs`
4. [`arg-parse.c`](rlarg/arg-parse.c) fix setref support: (expand switches)
    - `arg_parse_setref_argx`
5. [`arg-compgen.c`](rlarg/arg-compgen.c) add compgen support: (expand switch)
    - `static_arg_compgen_argx`
6. [`argx-type.c`](rlarg/argx-type.c) add the actual code to specify your type: (create your function)
    - `argx_type_???`

You can alternatively view places you need to patch via:

```sh
$ grep ARGX_TYPE_STRING * -rn
```

