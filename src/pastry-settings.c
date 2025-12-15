/* pastry-settings.c
 *
 * Copyright 2025 Eva M
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

/**
 * PastrySettings:
 *
 * Manages the libpastry context.
 */

#define G_LOG_DOMAIN "PASTRY::SETTINGS"

#include "config.h"

#include "pastry-settings.h"
#include "pastry-util.h"

enum
{
  PROP_0,

  PROP_THEME,

  LAST_PROP
};
static GParamSpec *props[LAST_PROP] = { 0 };

struct _PastrySettings
{
  GObject parent_instance;

  PastryTheme *theme;

  gboolean is_default;
  struct
  {
    GdkDisplay *display;

    GtkCssProvider *light;
    GtkCssProvider *dark;
    GtkCssProvider *hc_light;
    GtkCssProvider *hc_dark;

    GDBusProxy *settings_portal;
    gboolean    portal_wants_dark;

    GtkCssProvider *applied;
  } default_state;
};
G_DEFINE_FINAL_TYPE (PastrySettings, pastry_settings, G_TYPE_OBJECT)

static PastrySettings *
init_default (void);

static void
init_css (PastrySettings *self);

/* Copied with modifications from libadwaita */
#define PORTAL_BUS_NAME           "org.freedesktop.portal.Desktop"
#define PORTAL_OBJECT_PATH        "/org/freedesktop/portal/desktop"
#define PORTAL_SETTINGS_INTERFACE "org.freedesktop.portal.Settings"
#define PORTAL_ERROR_NOT_FOUND    "org.freedesktop.portal.Error.NotFound"
static void
init_portal (PastrySettings *self);

/* Copied with modifications from libadwaita */
static void
portal_changed_cb (PastrySettings *self,
                   const char     *sender_name,
                   const char     *signal_name,
                   GVariant       *parameters,
                   GDBusProxy     *proxy);

/* Copied with modifications from libadwaita */
static gboolean
is_dark (GVariant *variant);

/* Copied with modifications from libadwaita */
static gboolean
read_setting (PastrySettings *self,
              const char     *schema,
              const char     *name,
              const char     *type,
              GVariant      **out);

static void
apply_css (PastrySettings *self);

static void
dispose (GObject *object)
{
  PastrySettings *self = PASTRY_SETTINGS (object);

  if (self->default_state.applied != NULL)
    gtk_style_context_remove_provider_for_display (
        self->default_state.display,
        GTK_STYLE_PROVIDER (self->default_state.applied));

  pastry_clear_pointers (
      &self->theme, g_object_unref,
      &self->default_state.display, g_object_unref,
      &self->default_state.light, g_object_unref,
      &self->default_state.dark, g_object_unref,
      &self->default_state.hc_light, g_object_unref,
      &self->default_state.hc_dark, g_object_unref,
      &self->default_state.settings_portal, g_object_unref,
      NULL);

  G_OBJECT_CLASS (pastry_settings_parent_class)->dispose (object);
}

static void
get_property (GObject    *object,
              guint       prop_id,
              GValue     *value,
              GParamSpec *pspec)
{
  PastrySettings *self = PASTRY_SETTINGS (object);

  switch (prop_id)
    {
    case PROP_THEME:
      g_value_set_object (value, pastry_settings_get_theme (self));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
set_property (GObject      *object,
              guint         prop_id,
              const GValue *value,
              GParamSpec   *pspec)
{
  PastrySettings *self = PASTRY_SETTINGS (object);

  switch (prop_id)
    {
    case PROP_THEME:
      pastry_settings_set_theme (self, g_value_get_object (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
pastry_settings_class_init (PastrySettingsClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->set_property = set_property;
  object_class->get_property = get_property;
  object_class->dispose      = dispose;

  /**
   * PastrySettings:theme:
   *
   * The active theme for the context.
   */
  props[PROP_THEME] =
      g_param_spec_object (
          "theme",
          NULL, NULL,
          PASTRY_TYPE_THEME,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);
}

static void
pastry_settings_init (PastrySettings *self)
{
  init_portal (self);
}

/**
 * pastry_settings_get_default:
 *
 * Gets the default global `PastrySettings`.
 *
 * Returns: (transfer none): a `PastrySettings`.
 */
PastrySettings *
pastry_settings_get_default (void)
{
  static PastrySettings *settings = NULL;

  /* GTK APIs are not threadsafe */
  if (settings == NULL)
    settings = init_default ();

  return settings;
}

/**
 * pastry_copy_accent_rgba:
 *
 * Convenience function to get the accent color of the main `PastryTheme` for
 * the default `PastrySettings`.
 */
void
pastry_copy_accent_rgba (GdkRGBA *rgba)
{
  PastrySettings    *settings = NULL;
  PastryVisualTheme *theme    = NULL;

  g_return_if_fail (rgba != NULL);

  settings = pastry_settings_get_default ();
  theme    = pastry_get_object (settings, "theme", "visual-theme", NULL);

  if (theme != NULL)
    pastry_visual_theme_copy_accent_rgba (theme, rgba);
  else
    *rgba = (GdkRGBA) { 0 };
}

/**
 * pastry_settings_set_theme:
 * @self: a `PastrySettings`
 * @theme: a `PastryTheme`
 *
 * Sets the theme.
 */
void
pastry_settings_set_theme (PastrySettings *self,
                           PastryTheme    *theme)
{
  g_return_if_fail (PASTRY_IS_SETTINGS (self));
  g_return_if_fail (theme == NULL || PASTRY_IS_THEME (theme));

  if (theme == self->theme)
    return;
  g_clear_object (&self->theme);
  if (theme != NULL)
    self->theme = g_object_ref (theme);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_THEME]);
}

/**
 * pastry_settings_get_theme:
 * @self: a `PastrySettings`
 *
 * Gets the theme object of @self.
 *
 * Returns: (nullable) (transfer none): the theme object of @self
 */
PastryTheme *
pastry_settings_get_theme (PastrySettings *self)
{
  g_return_val_if_fail (PASTRY_IS_SETTINGS (self), NULL);
  return self->theme;
}

static PastrySettings *
init_default (void)
{
  g_autoptr (PastrySoundTheme) sound_theme   = NULL;
  g_autoptr (PastryVisualTheme) visual_theme = NULL;
  g_autoptr (PastryTheme) theme              = NULL;
  g_autoptr (PastrySettings) settings        = NULL;

  visual_theme = g_object_new (
      PASTRY_TYPE_VISUAL_THEME,
      "name", "Default Visual Theme",
      "accent", "#dfdfe7",
      NULL);
  sound_theme = g_object_new (
      PASTRY_TYPE_SOUND_THEME,
      "name", "Default Sound Theme",
      NULL);
  theme = g_object_new (
      PASTRY_TYPE_THEME,
      "name", "Default Theme",
      "visual-theme", visual_theme,
      "sound-theme", sound_theme,
      NULL);

  settings = g_object_new (
      PASTRY_TYPE_SETTINGS,
      "theme", theme,
      NULL);
  settings->is_default = TRUE;

  init_css (settings);
  init_portal (settings);

  return g_steal_pointer (&settings);
}

static void
init_css (PastrySettings *self)
{
  GdkDisplay *display = NULL;

  /* TODO: listen to creation of displays */
  display = gdk_display_get_default ();

  self->default_state.display  = g_object_ref (display);
  self->default_state.light    = gtk_css_provider_new ();
  self->default_state.dark     = gtk_css_provider_new ();
  self->default_state.hc_light = gtk_css_provider_new ();
  self->default_state.hc_dark  = gtk_css_provider_new ();

  gtk_css_provider_load_from_resource (
      self->default_state.light,
      "/org/gtk/libgtk/theme/Pastry/gtk.css");
  gtk_css_provider_load_from_resource (
      self->default_state.dark,
      "/org/gtk/libgtk/theme/Pastry/gtk-dark.css");
  gtk_css_provider_load_from_resource (
      self->default_state.hc_light,
      "/org/gtk/libgtk/theme/Pastry/gtk-hc.css");
  gtk_css_provider_load_from_resource (
      self->default_state.hc_dark,
      "/org/gtk/libgtk/theme/Pastry/gtk-hc-dark.css");
  g_info ("Loaded stylesheets");

  apply_css (self);
}

static void
init_portal (PastrySettings *self)
{
  g_autoptr (GError) error     = NULL;
  g_autoptr (GVariant) variant = NULL;
  gboolean was_read            = FALSE;

  self->default_state.settings_portal = g_dbus_proxy_new_for_bus_sync (
      G_BUS_TYPE_SESSION,
      G_DBUS_PROXY_FLAGS_NONE,
      NULL,
      PORTAL_BUS_NAME,
      PORTAL_OBJECT_PATH,
      PORTAL_SETTINGS_INTERFACE,
      NULL,
      &error);

  if (self->default_state.settings_portal == NULL)
    {
      g_debug ("Settings portal not found: %s", error->message);
      return;
    }

  was_read = read_setting (self, "org.freedesktop.appearance",
                           "color-scheme", "u", &variant);
  if (was_read)
    self->default_state.portal_wants_dark = is_dark (variant);
  else
    g_debug ("Could not read color scheme info from portal");

  g_signal_connect_swapped (
      self->default_state.settings_portal, "g-signal",
      G_CALLBACK (portal_changed_cb), self);
}

static void
portal_changed_cb (PastrySettings *self,
                   const char     *sender_name,
                   const char     *signal_name,
                   GVariant       *parameters,
                   GDBusProxy     *proxy)
{
  const char *namespace;
  const char *name;
  g_autoptr (GVariant) value = NULL;

  if (g_strcmp0 (signal_name, "SettingChanged") != 0)
    return;

  g_variant_get (parameters, "(&s&sv)", &namespace, &name, &value);
  if (g_strcmp0 (namespace, "org.freedesktop.appearance") == 0 &&
      g_strcmp0 (name, "color-scheme") == 0)
    {
      self->default_state.portal_wants_dark = is_dark (value);
      apply_css (self);
    }
}

static gboolean
is_dark (GVariant *variant)
{
  guint32 value = 0;

  value = g_variant_get_uint32 (variant);
  switch (value)
    {
    case 0:
    case 2:
      return FALSE;
    case 1:
      return TRUE;
    default:
      g_warning ("Invalid colorscheme from portal");
      return FALSE;
    }
}

static gboolean
read_setting (PastrySettings *self,
              const char     *schema,
              const char     *name,
              const char     *type,
              GVariant      **out)
{
  g_autoptr (GError) local_error = NULL;
  g_autoptr (GVariant) ret       = NULL;
  g_autoptr (GVariant) child     = NULL;
  g_autoptr (GVariant) child2    = NULL;
  g_autoptr (GVariantType) out_type;

  ret = g_dbus_proxy_call_sync (
      self->default_state.settings_portal,
      "Read",
      g_variant_new ("(ss)", schema, name),
      G_DBUS_CALL_FLAGS_NONE,
      G_MAXINT,
      NULL,
      &local_error);

  if (local_error != NULL)
    {
      if (local_error->domain == G_DBUS_ERROR &&
          local_error->code == G_DBUS_ERROR_SERVICE_UNKNOWN)
        g_warning ("Portal not found: %s", local_error->message);
      else if (local_error->domain == G_DBUS_ERROR &&
               local_error->code == G_DBUS_ERROR_UNKNOWN_METHOD)
        g_warning ("Portal doesn't provide settings: %s", local_error->message);
      else if (g_dbus_error_is_remote_error (local_error))
        {
          g_autofree char *remote_error = g_dbus_error_get_remote_error (local_error);

          if (!g_strcmp0 (remote_error, PORTAL_ERROR_NOT_FOUND))
            g_warning ("Setting %s.%s of type %s not found", schema, name, type);
        }
      else
        g_warning ("Couldn't read the %s setting: %s", name, local_error->message);

      return FALSE;
    }

  g_variant_get (ret, "(v)", &child);
  g_variant_get (child, "v", &child2);

  out_type = g_variant_type_new (type);
  if (g_variant_type_equal (g_variant_get_type (child2), out_type))
    {
      *out = g_steal_pointer (&child2);
      return TRUE;
    }
  else
    {
      g_critical ("Invalid type for %s.%s: expected %s, got %s",
                  schema, name, type, g_variant_get_type_string (child2));
      return FALSE;
    }
}

static void
apply_css (PastrySettings *self)
{
  if (self->default_state.applied != NULL)
    gtk_style_context_remove_provider_for_display (
        self->default_state.display,
        GTK_STYLE_PROVIDER (self->default_state.applied));
  g_clear_object (&self->default_state.applied);

  if (self->default_state.portal_wants_dark)
    self->default_state.applied = g_object_ref (self->default_state.dark);
  else
    self->default_state.applied = g_object_ref (self->default_state.light);

  gtk_style_context_add_provider_for_display (
      self->default_state.display,
      GTK_STYLE_PROVIDER (self->default_state.applied),
      GTK_STYLE_PROVIDER_PRIORITY_USER);
}
