
sources = [
  'arg.c',
  ]

headers = [
  'arg.h',
  ]

dep_rlc = dependency('rlc', required: true)
dep_rlso = dependency('rlso', required: true)

install_headers(headers, subdir: 'rl')
rlarg = library('rlarg',
  sources,
  link_args: '-lm',
  dependencies: [dep_rlc, dep_rlso],
  install: true,
  )

