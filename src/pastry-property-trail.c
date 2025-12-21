/* pastry-property-trail.c
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
 * PastryPropertyTrail:
 *
 * References properties of properties on GObject instances with an arbitrary
 * depth.
 */

#define G_LOG_DOMAIN "PASTRY::PROPERTY-TRAIL"

#include "config.h"

#include "pastry-property-trail.h"
#include "pastry-util.h"

enum
{
  PROP_0,

  PROP_OBJECT,
  PROP_TRAIL,

  LAST_PROP
};
static GParamSpec *props[LAST_PROP] = { 0 };

enum
{
  SIGNAL_CHANGED,

  LAST_SIGNAL,
};
static guint signals[LAST_SIGNAL];

struct _PastryPropertyTrail
{
  GObject parent_instance;

  GObject    *object;
  GListModel *trail;

  GPtrArray *objects;
};
G_DEFINE_FINAL_TYPE (PastryPropertyTrail, pastry_property_trail, G_TYPE_OBJECT)

static void
dig (PastryPropertyTrail *self);

static void
clear (PastryPropertyTrail *self,
       guint                from);

static void
property_changed_cb (PastryPropertyTrail *self,
                     GParamSpec          *pspec,
                     GObject             *instance);

static void
dispose (GObject *object)
{
  PastryPropertyTrail *self = PASTRY_PROPERTY_TRAIL (object);

  clear (self, 0);

  pastry_clear_pointers (
      &self->object, g_object_unref,
      &self->trail, g_object_unref,
      NULL);

  G_OBJECT_CLASS (pastry_property_trail_parent_class)->dispose (object);
}

static void
get_property (GObject    *object,
              guint       prop_id,
              GValue     *value,
              GParamSpec *pspec)
{
  PastryPropertyTrail *self = PASTRY_PROPERTY_TRAIL (object);

  switch (prop_id)
    {
    case PROP_OBJECT:
      g_value_set_object (value, pastry_property_trail_get_object (self));
      break;
    case PROP_TRAIL:
      g_value_set_object (value, pastry_property_trail_get_trail (self));
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
  PastryPropertyTrail *self = PASTRY_PROPERTY_TRAIL (object);

  switch (prop_id)
    {
    case PROP_OBJECT:
      pastry_property_trail_set_object (self, g_value_get_object (value));
      break;
    case PROP_TRAIL:
      pastry_property_trail_set_trail (self, g_value_get_object (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
pastry_property_trail_class_init (PastryPropertyTrailClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->set_property = set_property;
  object_class->get_property = get_property;
  object_class->dispose      = dispose;

  /**
   * PastryPropertyTrail:object:
   *
   * The root object to track from.
   */
  props[PROP_OBJECT] =
      g_param_spec_object (
          "object",
          NULL, NULL,
          G_TYPE_OBJECT,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * PastryPropertyTrail:trail:
   *
   * A `GListModel` of `GtkStringObject`s, where each string represents the
   * property at the depth of the item's index.
   */
  props[PROP_TRAIL] =
      g_param_spec_object (
          "trail",
          NULL, NULL,
          G_TYPE_LIST_MODEL,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  /**
   * PastryPropertyTrail::changed:
   * @trail: the object that received the signal
   * @object: the object with the greatest depth, or %NULL if it can't be resolved
   *
   * Emitted when any of the objects referenced by the property names along
   * [property@Pastry.PropertyTrail:trail] emit the "notify::name" signal
   */
  signals[SIGNAL_CHANGED] =
      g_signal_new (
          "changed",
          G_OBJECT_CLASS_TYPE (klass),
          G_SIGNAL_RUN_FIRST,
          0,
          NULL, NULL,
          g_cclosure_marshal_VOID__OBJECT,
          G_TYPE_NONE, 1,
          G_TYPE_OBJECT);
  g_signal_set_va_marshaller (
      signals[SIGNAL_CHANGED],
      G_TYPE_FROM_CLASS (klass),
      g_cclosure_marshal_VOID__OBJECTv);
}

static void
pastry_property_trail_init (PastryPropertyTrail *self)
{
  self->objects = g_ptr_array_new_with_free_func (g_object_unref);
}

/**
 * pastry_property_trail_new:
 * @object: The root object to track
 * @property: The first property
 * @...: optionally more properties, followed by %NULL
 *
 * Creates a new `PastryPropertyTrail` object.
 *
 * Returns: The newly created `PastryPropertyTrail` object.
 */
PastryPropertyTrail *
pastry_property_trail_new (gpointer    object,
                           const char *property,
                           ...)
{
  va_list var_args                = { 0 };
  g_autoptr (GtkStringList) trail = NULL;

  g_return_val_if_fail (G_IS_OBJECT (object), NULL);
  g_return_val_if_fail (property != NULL, NULL);

  trail = gtk_string_list_new (NULL);
  gtk_string_list_append (trail, property);

  va_start (var_args, property);
  for (;;)
    {
      const char *extra_property = NULL;

      extra_property = va_arg (var_args, const char *);
      if (extra_property != NULL)
        gtk_string_list_append (trail, extra_property);
      else
        break;
    }
  va_end (var_args);

  return g_object_new (
      PASTRY_TYPE_PROPERTY_TRAIL,
      "object", object,
      "trail", trail,
      NULL);
}

/**
 * pastry_property_trail_set_object:
 * @self: a `PastryPropertyTrail`
 * @object: a `GObject` instance to track from
 *
 * Sets the root object to track from.
 */
void
pastry_property_trail_set_object (PastryPropertyTrail *self,
                                  GObject             *object)
{
  g_return_if_fail (PASTRY_IS_PROPERTY_TRAIL (self));
  g_return_if_fail (object == NULL || G_IS_OBJECT (object));

  if (object == self->object)
    return;
  g_clear_object (&self->object);
  if (object != NULL)
    self->object = g_object_ref (object);

  clear (self, 0);
  dig (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_OBJECT]);
}

/**
 * pastry_property_trail_get_object:
 * @self: a `PastryPropertyTrail`
 *
 * Gets the root object being tracked by @self.
 *
 * Returns: (nullable) (transfer none): the root object being tracked by @self
 */
GObject *
pastry_property_trail_get_object (PastryPropertyTrail *self)
{
  g_return_val_if_fail (PASTRY_IS_PROPERTY_TRAIL (self), NULL);
  return self->object;
}

/**
 * pastry_property_trail_set_trail:
 * @self: a `PastryPropertyTrail`
 * @trail: a `GListModel` of `GtkStringObject`s
 *
 * Sets the trail to track on the root object.
 */
void
pastry_property_trail_set_trail (PastryPropertyTrail *self,
                                 GListModel          *trail)
{
  g_return_if_fail (PASTRY_IS_PROPERTY_TRAIL (self));
  g_return_if_fail (trail == NULL || G_IS_LIST_MODEL (trail));

  if (trail == self->trail)
    return;
  g_clear_object (&self->trail);
  if (trail != NULL)
    self->trail = g_object_ref (trail);

  clear (self, 0);
  dig (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TRAIL]);
}

/**
 * pastry_property_trail_get_trail:
 * @self: a `PastryPropertyTrail`
 *
 * Gets the trail model for @self.
 *
 * Returns: (nullable) (transfer none): the trail model for @self
 */
GListModel *
pastry_property_trail_get_trail (PastryPropertyTrail *self)
{
  g_return_val_if_fail (PASTRY_IS_PROPERTY_TRAIL (self), NULL);
  return self->trail;
}

static void
dig (PastryPropertyTrail *self)
{
  GType parent_prop_type     = G_TYPE_NONE;
  g_autoptr (GObject) object = NULL;
  guint n_items              = 0;

  if (self->object == NULL ||
      self->trail == NULL)
    {
      clear (self, 0);
      g_signal_emit (self, signals[SIGNAL_CHANGED], 0, NULL);
      return;
    }

  object = g_object_ref (self->object);

  n_items = g_list_model_get_n_items (self->trail);
  for (guint i = 0; i < n_items; i++)
    {
      g_autoptr (GtkStringObject) string = NULL;
      const char *property               = NULL;
      g_autoptr (GTypeClass) class       = NULL;
      GParamSpec *pspec                  = NULL;

      string   = g_list_model_get_item (self->trail, i);
      property = gtk_string_object_get_string (string);

      class = g_type_class_ref (G_OBJECT_TYPE (object));
      pspec = g_object_class_find_property (G_OBJECT_CLASS (class), property);
      if (pspec != NULL)
        {
          gboolean setup = TRUE;

          parent_prop_type = pspec->value_type;

          if (i < self->objects->len)
            {
              GObject *old = NULL;

              old = g_ptr_array_index (self->objects, i);
              if (object == old)
                setup = FALSE;
              else
                clear (self, i);
            }

          if (setup)
            {
              g_autofree char *spec = NULL;

              spec = g_strdup_printf ("notify::%s", property);
              g_signal_connect_swapped (object, spec, G_CALLBACK (property_changed_cb), self);

              g_ptr_array_add (self->objects, g_object_ref (object));
            }

          if (g_type_is_a (pspec->value_type, G_TYPE_OBJECT))
            {
              g_autoptr (GObject) tmp = NULL;

              g_object_get (object, property, &tmp, NULL);
              g_clear_object (&object);
              object = g_steal_pointer (&tmp);

              if (object == NULL)
                break;
            }
          else
            {
              if (i != n_items - 1)
                {
                  g_critical ("Property \"%s\" is not of an object gtype on class %s",
                              property, G_OBJECT_TYPE_NAME (object));
                  g_clear_object (&object);
                }
              break;
            }
        }
      else
        {
          if (!G_TYPE_IS_INTERFACE (parent_prop_type))
            /* Don't complain if the property was an interface, since the trail
               may be targeting a specific object type */
            g_critical ("Property \"%s\" doesn't exist on class %s",
                        property, G_OBJECT_TYPE_NAME (object));
          g_clear_object (&object);
          break;
        }
    }

  g_signal_emit (self, signals[SIGNAL_CHANGED], 0, object);
}

static void
clear (PastryPropertyTrail *self,
       guint                from)
{
  for (guint i = from; i < self->objects->len; i++)
    {
      GObject *object = NULL;

      object = g_ptr_array_index (self->objects, i);
      g_signal_handlers_disconnect_by_func (object, property_changed_cb, self);
    }

  g_ptr_array_set_size (self->objects, from);
}

static void
property_changed_cb (PastryPropertyTrail *self,
                     GParamSpec          *pspec,
                     GObject             *instance)
{
  dig (self);
}
