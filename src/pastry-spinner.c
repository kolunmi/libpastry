/* pastry-spinner.c
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

#define G_LOG_DOMAIN "PASTRY::SPINNER"

#include "config.h"

#include "pastry-settings.h"
#include "pastry-spinner.h"

enum
{
  PROP_0,

  PROP_SPEED,
  PROP_N_DOTS,

  LAST_PROP
};
static GParamSpec *props[LAST_PROP] = { 0 };

struct _PastrySpinner
{
  GtkWidget parent_instance;

  double speed;
  int    n_dots;

  double modulated;
};
G_DEFINE_FINAL_TYPE (PastrySpinner, pastry_spinner, GTK_TYPE_WIDGET)

static void
dispose (GObject *object)
{
  PastrySpinner *self = PASTRY_SPINNER (object);

  (void) self;

  G_OBJECT_CLASS (pastry_spinner_parent_class)->dispose (object);
}

static void
get_property (GObject    *object,
              guint       prop_id,
              GValue     *value,
              GParamSpec *pspec)
{
  PastrySpinner *self = PASTRY_SPINNER (object);

  switch (prop_id)
    {
    case PROP_SPEED:
      g_value_set_double (value, pastry_spinner_get_speed (self));
      break;
    case PROP_N_DOTS:
      g_value_set_int (value, pastry_spinner_get_n_dots (self));
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
  PastrySpinner *self = PASTRY_SPINNER (object);

  switch (prop_id)
    {
    case PROP_SPEED:
      pastry_spinner_set_speed (self, g_value_get_double (value));
      break;
    case PROP_N_DOTS:
      pastry_spinner_set_n_dots (self, g_value_get_int (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
measure (GtkWidget     *widget,
         GtkOrientation orientation,
         int            for_size,
         int           *minimum,
         int           *natural,
         int           *minimum_baseline,
         int           *natural_baseline)
{
  PastrySpinner *self = PASTRY_SPINNER (widget);

  (void) self;

  *minimum = 64;
  *natural = 128;
}

static void
snapshot (GtkWidget   *widget,
          GtkSnapshot *snapshot)
{
  PastrySpinner *self   = PASTRY_SPINNER (widget);
  int            width  = 0;
  int            height = 0;
  GdkRGBA        accent = { 0 };

  width  = gtk_widget_get_width (GTK_WIDGET (self));
  height = gtk_widget_get_height (GTK_WIDGET (self));

  pastry_copy_accent_rgba (&accent);

  gtk_snapshot_save (snapshot);
  gtk_snapshot_translate (
      snapshot,
      &GRAPHENE_POINT_INIT ((int) (width / 2), (int) (height / 2)));

  for (guint i = 0; i < self->n_dots; i++)
    {
      graphene_rect_t  bounds          = { 0 };
      graphene_point_t center          = { 0 };
      GskColorStop     stops[3]        = { 0 };
      double           circle_progress = 0.0;

      bounds = GRAPHENE_RECT_INIT (-15.0, -(int) (height / 2), 30.0, 30.0);
      graphene_rect_get_center (&bounds, &center);

      stops[0].color  = accent;
      stops[0].offset = 0.0;
      stops[1].color  = accent;
      stops[1].offset = 0.15;
      stops[2].color  = (GdkRGBA) { 0.0, 0.0, 0.0, 0.0 };

      circle_progress = self->modulated * (double) self->n_dots;
      if (i == 0 && circle_progress > (double) (self->n_dots - 1))
        circle_progress -= (double) self->n_dots;
      stops[2].offset = 1.0 - MIN (0.75, 0.75 * ABS ((double) i - circle_progress));

      gtk_snapshot_append_radial_gradient (
          snapshot, &bounds, &center,
          bounds.size.width / 2.0, bounds.size.height / 2.0,
          0.0, 1.0,
          stops, G_N_ELEMENTS (stops));

      gtk_snapshot_rotate (snapshot, 360.0 / (double) self->n_dots);
    }

  gtk_snapshot_restore (snapshot);
}

static void
pastry_spinner_class_init (PastrySpinnerClass *klass)
{
  GObjectClass   *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->set_property = set_property;
  object_class->get_property = get_property;
  object_class->dispose      = dispose;

  props[PROP_SPEED] =
      g_param_spec_double (
          "speed",
          NULL, NULL,
          0.0, G_MAXDOUBLE, 1.0,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  props[PROP_N_DOTS] =
      g_param_spec_int (
          "n-dots",
          NULL, NULL,
          1, 16, 8,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  widget_class->measure  = measure;
  widget_class->snapshot = snapshot;

  g_object_class_install_properties (object_class, LAST_PROP, props);
}

static gboolean
tick_cb (PastrySpinner *self,
         GdkFrameClock *frame_clock,
         gpointer       user_data)
{
  gint64 frame_time   = 0;
  double linear_value = 0.0;

  frame_time   = gdk_frame_clock_get_frame_time (frame_clock);
  linear_value = fmod ((double) (frame_time % (gint64) G_MAXDOUBLE) *
                           0.000001 * self->speed,
                       1.0);

  self->modulated = linear_value;
  gtk_widget_queue_draw (GTK_WIDGET (self));

  return G_SOURCE_CONTINUE;
}

static void
pastry_spinner_init (PastrySpinner *self)
{
  self->speed  = 1.0;
  self->n_dots = 8;

  gtk_widget_add_tick_callback (GTK_WIDGET (self), (GtkTickCallback) tick_cb, NULL, NULL);
}

void
pastry_spinner_set_speed (PastrySpinner *self,
                          double         speed)
{
  g_return_if_fail (PASTRY_IS_SPINNER (self));

  if (speed == self->speed)
    return;
  self->speed = speed;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SPEED]);
}

double
pastry_spinner_get_speed (PastrySpinner *self)
{
  g_return_val_if_fail (PASTRY_IS_SPINNER (self), 0.0);
  return self->speed;
}

void
pastry_spinner_set_n_dots (PastrySpinner *self,
                           int            n_dots)
{
  g_return_if_fail (PASTRY_IS_SPINNER (self));

  if (n_dots == self->n_dots)
    return;
  self->n_dots = n_dots;

  gtk_widget_queue_draw (GTK_WIDGET (self));
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_N_DOTS]);
}

int
pastry_spinner_get_n_dots (PastrySpinner *self)
{
  g_return_val_if_fail (PASTRY_IS_SPINNER (self), 0.0);
  return self->n_dots;
}
