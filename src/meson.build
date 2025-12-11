api_version = '0.1'

libpastry_sources = [
  'libpastry.c',
  'pastry-util.c',
  'pastry-animation.c',
]

libpastry_headers = [
  'libpastry-version-macros.h',
  'libpastry.h',
  'pastry-util.h',
  'pastry-animation.h',
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
  dependency('gtk4'),
]

subdir('stylesheet')
libpastry_sources += [libpastry_stylesheet_resources]

libpastry_lib = shared_library(
  'libpastry-' + api_version,
  libpastry_sources,
  dependencies: libpastry_deps,
       install: true,
)
libpastry_dep = declare_dependency(
  link_with: libpastry_lib,
  dependencies: libpastry_deps,
  include_directories : include_directories('..', '.'),
)

install_headers(libpastry_headers, subdir: 'libpastry')

pkg = import('pkgconfig')
pkg.generate(
  description: 'A shared library for ...',
    libraries: libpastry_lib,
         name: 'libpastry',
     filebase: 'libpastry-' + api_version,
      version: meson.project_version(),
      subdirs: 'libpastry',
     requires: 'gio-2.0',
  install_dir: get_option('libdir') / 'pkgconfig'
)

gnome = import('gnome')
libpastry_gir = gnome.generate_gir(libpastry_lib,
              sources: [libpastry_sources, files('libpastry.h')],
               header: 'libpastry.h',
      export_packages: 'libpastry-' + api_version,
            nsversion: api_version,
            namespace: 'pastry',
        symbol_prefix: 'pastry',
    identifier_prefix: 'pastry',
             includes: ['Gtk-4.0'],
              install: true,
)
