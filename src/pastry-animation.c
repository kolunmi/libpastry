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

  PROP_WIDGET,

  LAST_PROP
};
static GParamSpec *props[LAST_PROP] = { 0 };

struct _PastryAnimation
{
  GObject parent_instance;

  GtkWidget *widget;
  GWeakRef   wr;

  guint       tag;
  GHashTable *data;
};
G_DEFINE_FINAL_TYPE (PastryAnimation, pastry_animation, G_TYPE_OBJECT)

typedef struct
{
  double                  from;
  double                  to;
  double                  damping_ratio;
  double                  mass;
  double                  stiffness;
  PastryAnimationCallback cb;
  gpointer                user_data;
  GDestroyNotify          destroy_data;
  GTimer                 *timer;
} SpringData;

static gboolean
tick_cb (GtkWidget     *widget,
         GdkFrameClock *frame_clock,
         GWeakRef      *wr);

static void
destroy_spring_data (gpointer ptr);

static void
destroy_wr (gpointer ptr);

static void
dispose (GObject *object)
{
  PastryAnimation *self        = PASTRY_ANIMATION (object);
  g_autoptr (GtkWidget) widget = NULL;

  widget = g_weak_ref_get (&self->wr);
  if (widget != NULL)
    {
      gtk_widget_remove_tick_callback (widget, self->tag);
      self->tag = 0;
    }
  g_clear_object (&widget);
  g_weak_ref_clear (&self->wr);

  pastry_clear_pointers (
      &self->widget, g_object_unref,
      &self->data, g_hash_table_unref,
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
    case PROP_WIDGET:
      g_value_take_object (value, pastry_animation_dup_widget (self));
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
    case PROP_WIDGET:
      g_clear_object (&self->widget);
      self->widget = g_value_dup_object (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
constructed (GObject *object)
{
  PastryAnimation *self = PASTRY_ANIMATION (object);

  if (GTK_IS_WIDGET (self->widget))
    {
      GWeakRef *wr = NULL;

      wr = g_new0 (typeof (*wr), 1);
      g_weak_ref_init (wr, self);

      self->tag = gtk_widget_add_tick_callback (
          self->widget,
          (GtkTickCallback) tick_cb,
          wr, destroy_wr);
    }
  g_weak_ref_init (&self->wr, self->widget);
  g_clear_object (&self->widget);
}

static void
pastry_animation_class_init (PastryAnimationClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed  = constructed;
  object_class->set_property = set_property;
  object_class->get_property = get_property;
  object_class->dispose      = dispose;

  props[PROP_WIDGET] =
      g_param_spec_object (
          "widget",
          NULL, NULL,
          GTK_TYPE_WIDGET,
          G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, LAST_PROP, props);
}

static void
pastry_animation_init (PastryAnimation *self)
{
  g_weak_ref_init (&self->wr, NULL);
  self->data = g_hash_table_new_full (
      g_str_hash, g_str_equal, g_free, destroy_spring_data);
}

PastryAnimation *
pastry_animation_new (GtkWidget *widget)
{
  g_return_val_if_fail (GTK_IS_WIDGET (widget), NULL);
  return g_object_new (
      PASTRY_TYPE_ANIMATION,
      "widget", widget,
      NULL);
}

GtkWidget *
pastry_animation_dup_widget (PastryAnimation *self)
{
  g_return_val_if_fail (PASTRY_IS_ANIMATION (self), NULL);
  return g_weak_ref_get (&self->wr);
}

void
pastry_animation_add_spring (PastryAnimation        *self,
                             const char             *key,
                             double                  from,
                             double                  to,
                             double                  damping_ratio,
                             double                  mass,
                             double                  stiffness,
                             PastryAnimationCallback cb,
                             gpointer                user_data,
                             GDestroyNotify          destroy_data)
{
  g_autoptr (GtkWidget) widget = NULL;

  g_return_if_fail (PASTRY_IS_ANIMATION (self));
  g_return_if_fail (key != NULL);
  g_return_if_fail (cb != NULL);

  widget = g_weak_ref_get (&self->wr);
  if (widget != NULL)
    {
      SpringData *data = NULL;

      data                = g_new0 (typeof (*data), 1);
      data->from          = from;
      data->to            = to;
      data->damping_ratio = damping_ratio;
      data->mass          = mass;
      data->stiffness     = stiffness;
      data->cb            = cb;
      data->user_data     = user_data;
      data->destroy_data  = destroy_data;
      /* We'll fill this in on the first iteration */
      data->timer = NULL;

      g_hash_table_replace (self->data, g_strdup (key), data);
      cb (widget, key, from, user_data);
    }
  else if (user_data != NULL &&
           destroy_data != NULL)
    destroy_data (user_data);
}

static gboolean
tick_cb (GtkWidget     *widget,
         GdkFrameClock *frame_clock,
         GWeakRef      *wr)
{
  g_autoptr (PastryAnimation) self = NULL;
  GHashTableIter iter              = { 0 };

  self = g_weak_ref_get (wr);
  if (self == NULL)
    return G_SOURCE_REMOVE;

  g_hash_table_iter_init (&iter, self->data);
  for (;;)
    {
      char       *key      = NULL;
      SpringData *data     = NULL;
      double      value    = 0.0;
      gboolean    finished = FALSE;

      if (!g_hash_table_iter_next (
              &iter,
              (gpointer *) &key,
              (gpointer *) &data))
        break;

      if (data->timer == NULL)
        {
          data->timer = g_timer_new ();
          value       = data->from;
        }
      else
        {
          double elapsed = 0.0;

          elapsed = g_timer_elapsed (data->timer, NULL);

          /* TODO: actually use the spring parameters */
          value = data->from + 15.0 * elapsed * (data->to - data->from);
        }

      finished = G_APPROX_VALUE (value, data->to, 0.0001) ||
                 (data->from > data->to && value < data->to) ||
                 (data->from < data->to && value > data->to);
      if (finished)
        value = data->to;

      data->cb (widget, key, value, data->user_data);

      if (finished)
        g_hash_table_iter_remove (&iter);
    }

  return G_SOURCE_CONTINUE;
}

static void
destroy_spring_data (gpointer ptr)
{
  SpringData *data = ptr;

  if (data->destroy_data != NULL &&
      data->user_data != NULL)
    data->destroy_data (data->user_data);
  g_clear_pointer (&data->timer, g_timer_destroy);
  g_free (ptr);
}

static void
destroy_wr (gpointer ptr)
{
  GWeakRef *wr = ptr;

  g_weak_ref_clear (wr);
  g_free (ptr);
}
