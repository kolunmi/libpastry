stylesheet_deps = []

sassc = find_program('sassc', required: false)
if not sassc.found()
  subproject('sassc', default_options: ['warning_level=0', 'werror=false'])
  sassc = find_program('sassc', required: true)
endif

sassc_opts = [ '-a', '-M', '-t', 'compact' ]
pastry_scss_files = files([
  '_colors-public.scss',
  '_colors.scss',
  '_common.scss',
  '_drawing.scss',
  '_main.scss',
])

pastry_theme_variants = [
  'light',
  'dark',
  'hc',
  'hc-dark',
]
pastry_theme_deps = []
foreach variant: pastry_theme_variants
  pastry_theme_deps += custom_target('Pastry theme variant ' + variant,
    input: '@0@.scss'.format(variant),
    output: '@0@.css'.format(variant),
    command: [
      sassc, sassc_opts, '@INPUT@', '@OUTPUT@',
    ],
    depend_files: pastry_scss_files,
  )
endforeach

libpastry_stylesheet_resources = gnome.compile_resources(
  'pastry-stylesheet-resources',
  'pastry-stylesheet.gresources.xml',

  source_dir: [
    # List in order of preference
    meson.current_build_dir(),
    meson.current_source_dir(),
  ],
  dependencies: pastry_theme_deps,
  c_name: 'pastry_stylesheet',
)
