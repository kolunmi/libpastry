/* pastry-grid-spinner.c
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

#define G_LOG_DOMAIN "PASTRY::GRID-SPINNER"

#include "config.h"

#include "pastry-grid-spinner.h"
#include "pastry-property-trail.h"
#include "pastry-settings.h"
#include "pastry-util.h"

#define GAP_SIZE 0.5

enum
{
  PROP_0,

  PROP_SPEED,

  LAST_PROP
};
static GParamSpec *props[LAST_PROP] = { 0 };

struct _PastryGridSpinner
{
  GtkWidget parent_instance;

  double speed;

  double modulated;

  PastryPropertyTrail *accent_trail;
};
G_DEFINE_FINAL_TYPE (PastryGridSpinner, pastry_grid_spinner, GTK_TYPE_WIDGET)

static void
accent_changed_cb (PastryGridSpinner   *self,
                   PastryTheme         *theme,
                   PastryPropertyTrail *trail);

static void
dispose (GObject *object)
{
  PastryGridSpinner *self = PASTRY_GRID_SPINNER (object);

  g_signal_handlers_disconnect_by_func (
      self->accent_trail, accent_changed_cb, self);

  pastry_clear_pointers (
      &self->accent_trail, g_object_unref,
      NULL);

  G_OBJECT_CLASS (pastry_grid_spinner_parent_class)->dispose (object);
}

static void
get_property (GObject    *object,
              guint       prop_id,
              GValue     *value,
              GParamSpec *pspec)
{
  PastryGridSpinner *self = PASTRY_GRID_SPINNER (object);

  switch (prop_id)
    {
    case PROP_SPEED:
      g_value_set_double (value, pastry_grid_spinner_get_speed (self));
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
  PastryGridSpinner *self = PASTRY_GRID_SPINNER (object);

  switch (prop_id)
    {
    case PROP_SPEED:
      pastry_grid_spinner_set_speed (self, g_value_get_double (value));
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
  PastryGridSpinner *self = PASTRY_GRID_SPINNER (widget);

  (void) self;

  *minimum = 32;
  *natural = 64;
}

static void
snapshot (GtkWidget   *widget,
          GtkSnapshot *snapshot)
{
  static const graphene_rect_t rects[] = {
    GRAPHENE_RECT_INIT (-0.5, -1.5 - GAP_SIZE, 1.0, 1.0),
    GRAPHENE_RECT_INIT (0.5 + GAP_SIZE, -1.5 - GAP_SIZE, 1.0, 1.0),
    GRAPHENE_RECT_INIT (0.5 + GAP_SIZE, -0.5, 1.0, 1.0),
    GRAPHENE_RECT_INIT (0.5 + GAP_SIZE, 0.5 + GAP_SIZE, 1.0, 1.0),
    GRAPHENE_RECT_INIT (-0.5, 0.5 + GAP_SIZE, 1.0, 1.0),
    GRAPHENE_RECT_INIT (-1.5 - GAP_SIZE, 0.5 + GAP_SIZE, 1.0, 1.0),
    GRAPHENE_RECT_INIT (-1.5 - GAP_SIZE, -0.5, 1.0, 1.0),
    GRAPHENE_RECT_INIT (-1.5 - GAP_SIZE, -1.5 - GAP_SIZE, 1.0, 1.0),
  };

  PastryGridSpinner *self   = PASTRY_GRID_SPINNER (widget);
  int                width  = 0;
  int                height = 0;
  GdkRGBA            accent = { 0 };
  double             scale  = 0.0;

  width  = gtk_widget_get_width (GTK_WIDGET (self));
  height = gtk_widget_get_height (GTK_WIDGET (self));

  pastry_copy_accent_rgba (&accent);

  gtk_snapshot_save (snapshot);
  gtk_snapshot_translate (
      snapshot,
      &GRAPHENE_POINT_INIT ((int) (width / 2), (int) (height / 2)));

  scale = MIN (width, height) / (3.0 + GAP_SIZE * 4);
  gtk_snapshot_scale (snapshot, scale, scale);

  for (guint i = 0; i < G_N_ELEMENTS (rects); i++)
    {
      double circle_progress = 0.0;
      double alpha           = 0.0;

      circle_progress = self->modulated * (double) (int) G_N_ELEMENTS (rects);
      if (i == 0 && circle_progress > (double) (int) (G_N_ELEMENTS (rects) - 1))
        circle_progress -= (double) (int) G_N_ELEMENTS (rects);
      alpha = 1.0 - MIN (0.666, 0.666 * ABS ((double) i - circle_progress));

      gtk_snapshot_append_color (
          snapshot,
          &(GdkRGBA) { accent.red,
                       accent.green,
                       accent.blue,
                       accent.alpha * alpha },
          rects + i);
    }
  gtk_snapshot_append_color (
      snapshot,
      &accent,
      &GRAPHENE_RECT_INIT (-0.5, -0.5, 1.0, 1.0));

  gtk_snapshot_restore (snapshot);
}

static void
pastry_grid_spinner_class_init (PastryGridSpinnerClass *klass)
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

  widget_class->measure  = measure;
  widget_class->snapshot = snapshot;

  g_object_class_install_properties (object_class, LAST_PROP, props);
}

static gboolean
tick_cb (PastryGridSpinner *self,
         GdkFrameClock     *frame_clock,
         gpointer           user_data)
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
pastry_grid_spinner_init (PastryGridSpinner *self)
{
  self->speed = 1.0;

  gtk_widget_add_tick_callback (GTK_WIDGET (self), (GtkTickCallback) tick_cb, NULL, NULL);

  self->accent_trail = pastry_property_trail_new (
      pastry_settings_get_default (),
      "theme", "visual-theme", "accent", NULL);
  g_signal_connect_swapped (
      self->accent_trail, "changed",
      G_CALLBACK (accent_changed_cb), self);
}

void
pastry_grid_spinner_set_speed (PastryGridSpinner *self,
                               double             speed)
{
  g_return_if_fail (PASTRY_IS_GRID_SPINNER (self));

  if (speed == self->speed)
    return;
  self->speed = speed;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SPEED]);
}

double
pastry_grid_spinner_get_speed (PastryGridSpinner *self)
{
  g_return_val_if_fail (PASTRY_IS_GRID_SPINNER (self), 0.0);
  return self->speed;
}

static void
accent_changed_cb (PastryGridSpinner   *self,
                   PastryTheme         *theme,
                   PastryPropertyTrail *trail)
{
  gtk_widget_queue_draw (GTK_WIDGET (self));
}
