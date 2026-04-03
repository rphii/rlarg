project('rlarg', 'c', meson_version: '>=1.1', version: '0.1.0')
add_project_arguments('-DRLARG_VERSION="'+meson.project_version()+'"', language: 'c')

sources = [
  'rlarg/arg-after.c',
  'rlarg/arg-compgen.c',
  'rlarg/arg-core.c',
  'rlarg/arg-parse-config.c',
  'rlarg/arg-parse.c',
  'rlarg/arg-runtime.c',
  'rlarg/arg-stream.c',
  'rlarg/argx-attr.c',
  'rlarg/argx-callback.c',
  'rlarg/argx-group.c',
  'rlarg/argx-hint.c',
  'rlarg/argx-so.c',
  'rlarg/argx-type.c',
  'rlarg/argx.c',
  ]

#headers = [
#  'rlarg.h',
#  ]

rlc_dep = dependency('rlc', fallback : ['rlc', 'rlc_dep'], default_options: ['default_library=static'])
rlso_dep = dependency('rlso', fallback : ['rlso', 'rlso_dep'], default_options: ['default_library=static'])

install_headers('rlarg.h')
install_data('bash/rlarg', install_dir: get_option('completion_dir'))

librlarg = library('rlarg',
  sources,
  dependencies: [rlc_dep, rlso_dep],
  install: true,
  )

pkg = import('pkgconfig')
pkg.generate(
  libraries: librlarg,
  version: '9999',
  name: 'rlarg',
  description: 'rl* argument parser',
  )


rlarg = declare_dependency(
  link_with: librlarg,
  include_directories: '.',
  )

subdir('tests')

