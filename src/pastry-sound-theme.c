/* pastry-sound-theme.c
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
 * PastrySoundTheme:
 *
 * Defines a set of URIs to be played back in a libpastry UI when certain events
 * occur.
 */

#define G_LOG_DOMAIN "PASTRY::SOUND-THEME"

#include "config.h"

#include "pastry-sound-theme.h"
#include "pastry-util.h"

enum
{
  PROP_0,

  PROP_NAME,
  PROP_CONTROLLER_CONNECTED_URI,

  LAST_PROP
};
static GParamSpec *props[LAST_PROP] = { 0 };

struct _PastrySoundTheme
{
  GObject parent_instance;

  char *name;
  char *controller_connected_uri;
};
G_DEFINE_FINAL_TYPE (PastrySoundTheme, pastry_sound_theme, G_TYPE_OBJECT)

static void
dispose (GObject *object)
{
  PastrySoundTheme *self = PASTRY_SOUND_THEME (object);

  pastry_clear_pointers (
      &self->name, g_free,
      &self->controller_connected_uri, g_free,
      NULL);

  G_OBJECT_CLASS (pastry_sound_theme_parent_class)->dispose (object);
}

static void
get_property (GObject    *object,
              guint       prop_id,
              GValue     *value,
              GParamSpec *pspec)
{
  PastrySoundTheme *self = PASTRY_SOUND_THEME (object);

  switch (prop_id)
    {
    case PROP_NAME:
      g_value_set_string (value, pastry_sound_theme_get_name (self));
      break;
    case PROP_CONTROLLER_CONNECTED_URI:
      g_value_set_string (value, self->controller_connected_uri);
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
  PastrySoundTheme *self = PASTRY_SOUND_THEME (object);

  switch (prop_id)
    {
    case PROP_NAME:
      pastry_sound_theme_set_name (self, g_value_get_string (value));
      break;
    case PROP_CONTROLLER_CONNECTED_URI:
      g_clear_pointer (&self->controller_connected_uri, g_free);
      self->controller_connected_uri = g_value_dup_string (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
pastry_sound_theme_class_init (PastrySoundThemeClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->set_property = set_property;
  object_class->get_property = get_property;
  object_class->dispose      = dispose;

  /**
   * PastrySoundTheme:name:
   *
   * The name of this theme.
   */
  props[PROP_NAME] =
      g_param_spec_string (
          "name",
          NULL, NULL, NULL,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * PastrySoundTheme:controller-connected-uri:
   *
   * The uri pointing to a sound to play when a controller is connected.
   */
  props[PROP_CONTROLLER_CONNECTED_URI] =
      g_param_spec_string (
          "controller-connected-uri",
          NULL, NULL, NULL,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, LAST_PROP, props);
}

static void
pastry_sound_theme_init (PastrySoundTheme *self)
{
}

/**
 * pastry_sound_theme_set_name:
 * @self: a `PastrySoundTheme`
 * @name: a string representing the new name of the theme
 *
 * Sets the theme's name.
 */
void
pastry_sound_theme_set_name (PastrySoundTheme *self,
                             const char       *name)
{
  g_return_if_fail (PASTRY_IS_SOUND_THEME (self));

  if (name == self->name)
    return;
  g_clear_pointer (&self->name, g_free);
  if (name != NULL)
    self->name = g_strdup (name);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_NAME]);
}

/**
 * pastry_sound_theme_get_name:
 * @self: a `PastrySoundTheme`
 *
 * Gets the name string of @self.
 *
 * Returns: (nullable) (transfer none): the name of @self
 */
const char *
pastry_sound_theme_get_name (PastrySoundTheme *self)
{
  g_return_val_if_fail (PASTRY_IS_SOUND_THEME (self), NULL);
  return self->name;
}
