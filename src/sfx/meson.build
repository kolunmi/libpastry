libpastry_sfx_resources = gnome.compile_resources(
  'pastry-sfx-resources',
  'pastry-sfx.gresources.xml',

  source_dir: [
    # List in order of preference
    meson.current_build_dir(),
    meson.current_source_dir(),
  ],
  dependencies: pastry_theme_deps,
  c_name: 'pastry_sfx',
)
