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
};
G_DEFINE_FINAL_TYPE (PastrySettings, pastry_settings, G_TYPE_OBJECT)

static void
dispose (GObject *object)
{
  PastrySettings *self = PASTRY_SETTINGS (object);

  pastry_clear_pointers (
      &self->theme, g_object_unref,
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
}

PastrySettings *
pastry_settings_get_default (void)
{
  static PastrySettings *settings = NULL;

  /* GTK APIs are not threadsafe */
  if (settings == NULL)
    {
      g_autoptr (PastryTheme) theme = NULL;

      theme = g_object_new (
          PASTRY_TYPE_THEME,
          "name", "default",
          "accent", "#dfdfe7",
          NULL);

      settings = g_object_new (
          PASTRY_TYPE_SETTINGS,
          "theme", theme,
          NULL);
    }

  return settings;
}

void
pastry_copy_accent_rgba (GdkRGBA *rgba)
{
  PastrySettings *settings = NULL;
  PastryTheme    *theme    = NULL;

  g_return_if_fail (rgba != NULL);

  settings = pastry_settings_get_default ();
  theme    = pastry_settings_get_theme (settings);

  if (theme != NULL)
    pastry_theme_copy_accent_rgba (theme, rgba);
  else
    *rgba = (GdkRGBA) { 0 };
}

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

PastryTheme *
pastry_settings_get_theme (PastrySettings *self)
{
  g_return_val_if_fail (PASTRY_IS_SETTINGS (self), NULL);
  return self->theme;
}
