
sources = [
  'arg.c',
  ]

headers = [
  'arg.h',
  ]

dep_rlc = dependency('rlc', required: false)
if not dep_rlc.found()
  prj_rlc = subproject('rlc')
  dep_rlc = prj_rlc.get_variable('dep_rlc')
endif

dep_rlso = dependency('rlso', required: false)
if not dep_rlso.found()
  prj_rlso = subproject('rlso')
  dep_rlso = prj_rlso.get_variable('dep_rlso')
endif

install_headers(headers, subdir: 'rl')
rlarg = library('rlarg',
  sources,
  link_args: '-lm',
  dependencies: [dep_rlc, dep_rlso],
  install: true,
  )

