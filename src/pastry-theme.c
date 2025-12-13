/* pastry-theme.c
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

#define G_LOG_DOMAIN "PASTRY::THEME"

#include "config.h"

#include "pastry-theme.h"
#include "pastry-util.h"

enum
{
  PROP_0,

  PROP_ACCENT,
  PROP_NAME,

  LAST_PROP
};
static GParamSpec *props[LAST_PROP] = { 0 };

struct _PastryTheme
{
  GObject parent_instance;

  char *name;
  char *accent;

  GdkRGBA accent_rgba;
};
G_DEFINE_FINAL_TYPE (PastryTheme, pastry_theme, G_TYPE_OBJECT)

static void
dispose (GObject *object)
{
  PastryTheme *self = PASTRY_THEME (object);

  pastry_clear_pointers (
      &self->name, g_free,
      &self->accent, g_free,
      NULL);

  G_OBJECT_CLASS (pastry_theme_parent_class)->dispose (object);
}

static void
get_property (GObject    *object,
              guint       prop_id,
              GValue     *value,
              GParamSpec *pspec)
{
  PastryTheme *self = PASTRY_THEME (object);

  switch (prop_id)
    {
    case PROP_NAME:
      g_value_set_string (value, pastry_theme_get_name (self));
      break;
    case PROP_ACCENT:
      g_value_set_string (value, pastry_theme_get_accent (self));
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
  PastryTheme *self = PASTRY_THEME (object);

  switch (prop_id)
    {
    case PROP_NAME:
      pastry_theme_set_name (self, g_value_get_string (value));
      break;
    case PROP_ACCENT:
      pastry_theme_set_accent (self, g_value_get_string (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
pastry_theme_class_init (PastryThemeClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->set_property = set_property;
  object_class->get_property = get_property;
  object_class->dispose      = dispose;

  props[PROP_NAME] =
      g_param_spec_string (
          "name",
          NULL, NULL, NULL,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  props[PROP_ACCENT] =
      g_param_spec_string (
          "accent",
          NULL, NULL, NULL,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);
}

static void
pastry_theme_init (PastryTheme *self)
{
}

void
pastry_theme_set_name (PastryTheme *self,
                       const char  *name)
{
  g_return_if_fail (PASTRY_IS_THEME (self));

  if (name == self->name)
    return;
  g_clear_pointer (&self->name, g_free);
  if (name != NULL)
    self->name = g_strdup (name);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_NAME]);
}

const char *
pastry_theme_get_name (PastryTheme *self)
{
  g_return_val_if_fail (PASTRY_IS_THEME (self), NULL);
  return self->name;
}

void
pastry_theme_set_accent (PastryTheme *self,
                         const char  *accent)
{
  g_return_if_fail (PASTRY_IS_THEME (self));

  if (accent == self->accent)
    return;
  g_clear_pointer (&self->accent, g_free);
  if (accent != NULL)
    {
      if (gdk_rgba_parse (&self->accent_rgba, accent))
        self->accent = g_strdup (accent);
      else
        {
          g_critical ("\"%s\" is an invalid RGBA spec", accent);
          self->accent_rgba = (GdkRGBA) { 0 };
        }
    }
  else
    self->accent_rgba = (GdkRGBA) { 0 };

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ACCENT]);
}

const char *
pastry_theme_get_accent (PastryTheme *self)
{
  g_return_val_if_fail (PASTRY_IS_THEME (self), NULL);
  return self->accent;
}

void
pastry_theme_copy_accent_rgba (PastryTheme *self,
                               GdkRGBA     *rgba)
{
  g_return_if_fail (PASTRY_IS_THEME (self));
  g_return_if_fail (rgba != NULL);

  *rgba = self->accent_rgba;
}
