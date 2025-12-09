stylesheet_deps = []

sassc = find_program('sassc', required: false)
if not sassc.found()
  subproject('sassc', default_options: ['warning_level=0', 'werror=false'])
  sassc = find_program('sassc', required: true)
endif

sassc_opts = [ '-a', '-M', '-t', 'compact' ]
scss_deps = files([
  '_colors.scss',
  '_common.scss',
  '_compat-colors.scss',
  '_drawing.scss',
  '_palette.scss',
])

stylesheet_deps += custom_target(
  'gtk.scss',
  input: 'gtk.scss',
  output: 'gtk.css',
  command: [
    sassc, sassc_opts, '@INPUT@', '@OUTPUT@',
  ],
  depend_files: scss_deps,
)

libpastry_stylesheet_resources = gnome.compile_resources(
  'pastry-stylesheet-resources',
  'pastry-stylesheet.gresources.xml',

  source_dir: [
    # List in order of preference
    meson.current_build_dir(),
    meson.current_source_dir(),
  ],
  dependencies: stylesheet_deps,
  c_name: 'pastry_stylesheet',
)
