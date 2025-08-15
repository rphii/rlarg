
sources = [
  'rlarg.c',
  ]

headers = [
  ]

rlc_dep = dependency('rlc', fallback : ['rlc', 'rlc_dep'], default_options: ['default_library=static'])
rlso_dep = dependency('rlso', fallback : ['rlso', 'rlso_dep'], default_options: ['default_library=static'])

install_headers(headers, subdir: 'rlarg')
install_headers('rlarg.h')
librlarg = library('rlarg',
  sources,
  link_args: '-lm',
  dependencies: [rlc_dep, rlso_dep],
  install: true,
  )

pkg = import('pkgconfig')
pkg.generate(
  libraries: librlarg,
  version: meson.project_version(),
  name: 'rlarg',
  description: 'Argument parser for C.',
  )

rlarg_dep = declare_dependency(
  link_with: librlarg,
  include_directories: '.',
  )

