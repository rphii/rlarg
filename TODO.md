# priority

- sequence (e.g. \<int, string\>)
- IMMEDIATELY_PRE / POST
- custom hints for basic things
- parse arbitrary config hierarchy -> `arg-parse-config.c:30`
- compgen for, e.g. `prog --help<tab>` does not finish the word :(

# later

- arbitrary hierarchy = file

- min/max for numbers and...
- ..float/double
- ..time

- hot/reload
- better compgen for flags, e.g: +flag,-flag2 and show combinations..
- organize so_fmt_fx better..... (they all over the place)
- add option to set parser error... in arg_runtime?
- note that if URI wasn't found? optional enable/disable?

- don't output 'not set anywhere' .. if a group? what if callback? or just list sources of all sub-items?

- roff (man page) generation?

- add option to disable/enable function execution during compgen (why would I want/not want this...?)

