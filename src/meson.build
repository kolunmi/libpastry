api_version = '0.1'

libpastry_sources = [
  'libpastry.c',
  'pastry-animation.c',
  'pastry-annotation-overlay.c',
  'pastry-focus-overlay.c',
  'pastry-glass-frame.c',
  'pastry-glass-root.c',
  'pastry-glassed.c',
  'pastry-grid-spinner.c',
  'pastry-property-trail.c',
  'pastry-settings.c',
  'pastry-sound-theme.c',
  'pastry-spinner.c',
  'pastry-theme.c',
  'pastry-util.c',
  'pastry-visual-theme.c',
]

libpastry_headers = [
  'libpastry.h',
  'pastry-animation.h',
  'pastry-annotation-overlay.h',
  'pastry-focus-overlay.h',
  'pastry-glass-frame.h',
  'pastry-glass-root.h',
  'pastry-glassed.h',
  'pastry-grid-spinner.h',
  'pastry-property-trail.h',
  'pastry-settings.h',
  'pastry-sound-theme.h',
  'pastry-spinner.h',
  'pastry-theme.h',
  'pastry-visual-theme.h',
]

version_split = meson.project_version().split('.')
version_conf = configuration_data()
version_conf.set('VERSION', meson.project_version())
version_conf.set('MAJOR_VERSION', version_split[0])
version_conf.set('MINOR_VERSION', version_split[1])
version_conf.set('MICRO_VERSION', version_split[2])

configure_file(
          input: 'libpastry-version.h.in',
         output: 'libpastry-version.h',
  configuration: version_conf,
        install: true,
    install_dir: get_option('includedir') / 'libpastry'
)

libpastry_deps = [
  cc.find_library('m', required: true),
  dependency('gtk4', version: '>=4.20'),
]

subdir('stylesheet')
libpastry_sources += [libpastry_stylesheet_resources]

subdir('sfx')
libpastry_sources += [libpastry_sfx_resources]

libpastry_lib = shared_library(
  'libpastry-' + api_version,
  libpastry_sources,
  dependencies: libpastry_deps,
       install: true,
)

install_headers(
  libpastry_headers + files('libpastry-version-macros.h'),
  subdir: 'libpastry')

pkg = import('pkgconfig')
pkg.generate(
  description: 'A shared library for libpastry',
    libraries: libpastry_lib,
         name: 'libpastry',
     filebase: 'libpastry-' + api_version,
      version: meson.project_version(),
      subdirs: 'libpastry',
     requires: 'gtk4',
  install_dir: get_option('libdir') / 'pkgconfig'
)

gir_args = []
libpastry_gir = gnome.generate_gir(libpastry_lib,
              sources: [libpastry_sources, libpastry_headers],
               header: 'libpastry.h',
      export_packages: 'Pastry-' + api_version,
            nsversion: api_version,
            namespace: 'Pastry',
        symbol_prefix: 'pastry',
    identifier_prefix: 'Pastry',
           extra_args: gir_args,
             includes: ['Gtk-4.0'],
              install: true,
)
libpastry_gir_dep = declare_dependency(
  sources: [libpastry_gir],
)

libpastry_dep = declare_dependency(
  link_with: libpastry_lib,
  dependencies: [libpastry_deps, libpastry_gir_dep],
  include_directories : include_directories('..', '.'),
)
