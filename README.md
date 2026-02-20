# rlarg

Argument parser for C.

## Dependencies

- [rphii/rlc](https://github.com/rphii/rlc) - core helpers
- [rphii/rlso](https://github.com/rphii/rlso) - string object

## Features

**Core**

- directly assign parsed values to your variables (no need to do a lookup on the argument parser)
- positional values, optional values and environmental values
- supports `bool`, `int`, `ssize_t`, `So` (string), `Color`, `enum`, `flags` (toggle), `group` (sub-options)
- supports arrays: `bool`, `int`, `ssize_t`, `So`, `Color`
- supports catch-all via: `rest` (array of `So`)
- disables all colors when piping (`isatty() == 0`)
- readable errors, I paid attention to short but descriptive messages
- tracks where values get set (stdin, config file + line, reference value, ...)

**Builtin**

- `argx_builtin_opt_help`: expansive help
- `argx_builtin_rice`: enable colors
- `argx_builtin_env_compgen`: auto completion support (see also [`bash/rlc`](bash/rlc))
- `argx_builtin_env_nocolor`: explicit `NOCOLOR` support 
- `argx_builtin_opt_so_fx`: string object effects (kind of like a custom type, it adds sub-options)
- `argx_builtin_opt_source`: config loading support (+ config can load other configs, and so on)
- `arg_enable_config_print`: config generation support

**Miscellaneous**

- `argx_callback`: provides support for custom callback (e.g. custom parser)
- `argx_attr_hide`: hides a value in help listing or config generation (e.g. API key)
- `argx_attr_configurable`: control if value can be configured via config file

**Runtime**

Used from within a callback (`argx_callback`):

- `arg_runtime_quit_early` quit the parser immediately (once control regained)
- `arg_runtime_quit_when_all_valid` quit the parser early after full validation

## Everything About Config Files

### Example

See [tests/readme.c](tests/readme.c)

Say that your program outputs something along the lines of this with `--help`:

```
positional:
default:
  -h  --help <rest>                     print this help
      --source <uri-array>              source config files =[
                                          "readme.conf"
                                        ]
      --int <int>                       an integer =0
      --vint <int-array>                an integer array
environment:
  NOCOLOR <bool>                        disable colors =false
  CONFIG_PRINT [default]                generate config of certain group
```

### Get a template configuration

```sh
$ CONFIG_PRINT=default ./build/tests/readme.c
[default]
source = [
  "readme.conf"
] # <uri-array>
int = 0 # <int>
# vint = <int-array>
```

### Syntax rules

The config file syntax resembles `TOML` if you squint really really hard. (it a lot different)

**Hierarchy**

To access any option, you can always use the full *hierarchy*. In the above
example, to access the `int` setting, you could do `default.int`. For the
`vint` -> `default.vint`.

**Sections**

If there are many configurations that belong to the same parent group, you can
make a line, that contains only `[the.section]` to make it easier for yourself.

**Basic Types:**

- Anything past a # is considered a comment and ignored *(except if it is
  wrapped inside a string, duh)*
- an array `[ with, something, inside ]`
- an automatic `file`
- a specified `file("with/a/path")`
- a string "that supports\nescape sequences"
- *any other thing on one line*

<details>
<summary>Setting your first int</summary>

In `readme.conf`, you could do this to set the int:

```
default.int = 123 # ignored comment
```

...or this...

```
[default] # ignored comment
int = 123
```

...or this...

```
[default.int]
= 123
```

</details>

<details>
<summary>Setting your first vector</summary>

In `readme.conf`, you could do this to set the int array:

```
default.vint = 123 # ignored comment
```

...or this...

```
default.vint = [123,456]
```

...or this...

```
default.vint = 

   [

        123

      ,

# ,,,ignored],,,comment,,,

     456

     ,

 ]

```

...or this...

```
[default]
int = 123
# ignored comment
int = 456
```

...or this...

```
[default.int]
= 123
= 456
```

</details>

<details>
<summary>Setting your first string</summary>

You can set a string by "quoting" it. This allows for precise control over your input data®.

- `\a \b \f \b \r \t \v \\ \' \" \e` more or less common escape sequences
- `\o123` -> `83` octal
- `\x42` -> `66` hex
- `\u03c6` -> `φ` unicode

- for more (test coverage) see [here](https://github.com/rphii/rlso/blob/main/tests/unescape.c)

Once the conversion is done, it is treated as... *any other type*. If your
option is an int, it is then converted to an int (it informs you if the
conversion failed). If your option happens to be a string, well, you're quite
lucky, aren't you. Because this is one way to make multiline strings!


</details>

<details>
<summary>Setting your first file</summary>

### Why does file exist?

The `file` and `file("with/path")` are special. As you may have noticed
already, there are no multiline strings. *(as of now.
\#all-my-homies-hate-multiline-strings)*.

To move around this issue, you can either use
"strings\nwith\nescape\nsequences\n" or... `file`!

### What does it do?

It load a file into the value, as the type, ... let's call it: *any other thing on MULTIPLE lines*.

- `file` searches, in the same folder as the configuration file, for one that matches the hierarchy\*
- `file("with/a/path")` searches for the specified file


\*example, say the config is `~/.config/readme/readme.conf`:

```
[default]
int = file
```

searches for a file called: `~/.config/readme/default.int`

### Special rules?

In the context if your *hierarchy* is an array vs. if your *file* is in an array:

- Hierarchy is array -> *file* is NOT in an array -> split on each (non-empty) line, generating a vector equivalent to the number of lines
- Hierarchy is array -> *file* is INSIDE an array -> dump 1:1

So these two differ:

```
[default]

# append an array of integers from the contents of default.vint file, spliting on each new line
vint = file         

# append an array 2 long, containing the int of the first and second file
vint = [ file("single int"), file("single int 2") ]
```

</details>


## Note to self

<details>
<summary>Adding an internal type</summary>

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
</details>

