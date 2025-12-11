/* pastry-animation.c
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

#define G_LOG_DOMAIN "PASTRY::ANIMATION"

#include "config.h"

#include "pastry-animation.h"
#include "pastry-util.h"

enum
{
  PROP_0,

  PROP_TARGET,
  PROP_PROPERTY_NAME,

  LAST_PROP
};
static GParamSpec *props[LAST_PROP] = { 0 };

struct _PastryAnimation
{
  GObject parent_instance;

  GObject *target;
  char    *property_name;
};
G_DEFINE_FINAL_TYPE (PastryAnimation, pastry_animation, G_TYPE_OBJECT)

static void
dispose (GObject *object)
{
  PastryAnimation *self = PASTRY_ANIMATION (object);

  pastry_clear_pointers (
      &self->target, g_object_unref,
      &self->property_name, g_free,
      NULL);

  G_OBJECT_CLASS (pastry_animation_parent_class)->dispose (object);
}

static void
get_property (GObject    *object,
              guint       prop_id,
              GValue     *value,
              GParamSpec *pspec)
{
  PastryAnimation *self = PASTRY_ANIMATION (object);

  switch (prop_id)
    {
    case PROP_TARGET:
      g_value_set_object (value, pastry_animation_get_target (self));
      break;
    case PROP_PROPERTY_NAME:
      g_value_set_string (value, pastry_animation_get_property_name (self));
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
  PastryAnimation *self = PASTRY_ANIMATION (object);

  switch (prop_id)
    {
    case PROP_TARGET:
      pastry_animation_set_target (self, g_value_get_object (value));
      break;
    case PROP_PROPERTY_NAME:
      pastry_animation_set_property_name (self, g_value_get_string (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
pastry_animation_class_init (PastryAnimationClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->set_property = set_property;
  object_class->get_property = get_property;
  object_class->dispose      = dispose;

  props[PROP_TARGET] =
      g_param_spec_object (
          "target",
          NULL, NULL,
          G_TYPE_OBJECT,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  props[PROP_PROPERTY_NAME] =
      g_param_spec_string (
          "property-name",
          NULL, NULL, NULL,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);
}

static void
pastry_animation_init (PastryAnimation *self)
{
}

void
pastry_animation_set_target (PastryAnimation *self,
                             GObject         *target)
{
  g_return_if_fail (PASTRY_IS_ANIMATION (self));
  g_return_if_fail (target == NULL || G_IS_OBJECT (self));

  if (target == self->target)
    return;
  g_clear_object (&self->target);
  if (target != NULL)
    self->target = g_object_ref (target);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TARGET]);
}

GObject *
pastry_animation_get_target (PastryAnimation *self)
{
  g_return_val_if_fail (PASTRY_IS_ANIMATION (self), NULL);
  return self->target;
}

void
pastry_animation_set_property_name (PastryAnimation *self,
                                    const char      *property_name)
{
  g_return_if_fail (PASTRY_IS_ANIMATION (self));

  if (property_name == self->property_name)
    return;
  g_clear_pointer (&self->property_name, g_free);
  if (property_name != NULL)
    self->property_name = g_strdup (property_name);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_PROPERTY_NAME]);
}

const char *
pastry_animation_get_property_name (PastryAnimation *self)
{
  g_return_val_if_fail (PASTRY_IS_ANIMATION (self), NULL);
  return self->property_name;
}
