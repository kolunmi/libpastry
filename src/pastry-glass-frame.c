/* pastry-glass-frame.c
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
 * PastryGlassFrame:
 *
 * Draws a child widget above a frame. This arrangement allows blurring the
 * background of the child widget.
 */

#define G_LOG_DOMAIN "PASTRY::GLASS-FRAME"

#include "config.h"

#include "pastry-glass-frame.h"
#include "pastry-glassed.h"
#include "pastry-util.h"

enum
{
  PROP_0,

  PROP_CHILD,

  LAST_PROP
};
static GParamSpec *props[LAST_PROP] = { 0 };

struct _PastryGlassFrame
{
  GtkWidget parent_instance;

  GtkWidget *child;
};

static void
glassed_iface_init (PastryGlassedInterface *iface);

G_DEFINE_FINAL_TYPE_WITH_CODE (
    PastryGlassFrame,
    pastry_glass_frame,
    GTK_TYPE_WIDGET,
    G_IMPLEMENT_INTERFACE (PASTRY_TYPE_GLASSED, glassed_iface_init))

static void
dispose (GObject *object)
{
  PastryGlassFrame *self = PASTRY_GLASS_FRAME (object);

  pastry_clear_pointers (
      &self->child, gtk_widget_unparent,
      NULL);

  G_OBJECT_CLASS (pastry_glass_frame_parent_class)->dispose (object);
}

static void
get_property (GObject    *object,
              guint       prop_id,
              GValue     *value,
              GParamSpec *pspec)
{
  PastryGlassFrame *self = PASTRY_GLASS_FRAME (object);

  switch (prop_id)
    {
    case PROP_CHILD:
      g_value_set_object (value, pastry_glass_frame_get_child (self));
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
  PastryGlassFrame *self = PASTRY_GLASS_FRAME (object);

  switch (prop_id)
    {
    case PROP_CHILD:
      pastry_glass_frame_set_child (self, g_value_get_object (value));
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
  PastryGlassFrame *self = PASTRY_GLASS_FRAME (widget);

  if (self->child != NULL)
    gtk_widget_measure (
        self->child, orientation,
        for_size, minimum, natural,
        minimum_baseline, natural_baseline);
}

static void
size_allocate (GtkWidget *widget,
               int        width,
               int        height,
               int        baseline)
{
  PastryGlassFrame *self = PASTRY_GLASS_FRAME (widget);

  if (self->child != NULL && gtk_widget_should_layout (self->child))
    gtk_widget_allocate (self->child, width, height, baseline, NULL);
}

static void
snapshot (GtkWidget   *widget,
          GtkSnapshot *snapshot)
{
  return;
}

static void
pastry_glass_frame_class_init (PastryGlassFrameClass *klass)
{
  GObjectClass   *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->set_property = set_property;
  object_class->get_property = get_property;
  object_class->dispose      = dispose;

  /**
   * PastryGlassFrame:child:
   *
   * The child widget
   */
  props[PROP_CHILD] =
      g_param_spec_object (
          "child",
          NULL, NULL,
          GTK_TYPE_WIDGET,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  widget_class->measure       = measure;
  widget_class->size_allocate = size_allocate;
  widget_class->snapshot      = snapshot;

  gtk_widget_class_set_css_name (widget_class, "pastry-glass-frame");
}

static void
pastry_glass_frame_init (PastryGlassFrame *self)
{
}

static gboolean
place_glass (PastryGlassed  *self,
             GskRoundedRect *dest)
{
  GtkWidget *widget = GTK_WIDGET (self);
  double     width  = 0.0;
  double     height = 0.0;

  width  = gtk_widget_get_width (widget);
  height = gtk_widget_get_height (widget);

  dest->bounds    = GRAPHENE_RECT_INIT (0.0, 0.0, width, height);
  dest->corner[0] = GRAPHENE_SIZE_INIT (5.0, 5.0);
  dest->corner[1] = GRAPHENE_SIZE_INIT (5.0, 5.0);
  dest->corner[2] = GRAPHENE_SIZE_INIT (5.0, 5.0);
  dest->corner[3] = GRAPHENE_SIZE_INIT (5.0, 5.0);

  return TRUE;
}

static void
snapshot_overlay (PastryGlassed *glassed,
                  GtkSnapshot   *snapshot)
{
  PastryGlassFrame *self = PASTRY_GLASS_FRAME (glassed);

  if (self->child != NULL)
    gtk_widget_snapshot_child (GTK_WIDGET (self), self->child, snapshot);
}

static void
glassed_iface_init (PastryGlassedInterface *iface)
{
  iface->place_glass      = place_glass;
  iface->snapshot_overlay = snapshot_overlay;
}

/**
 * pastry_glass_frame_set_child:
 * @self: a `PastryGlassFrame`
 * @child: the child widget
 *
 * Sets the child widget
 */
void
pastry_glass_frame_set_child (PastryGlassFrame *self,
                              GtkWidget        *child)
{
  g_return_if_fail (PASTRY_IS_GLASS_FRAME (self));

  g_return_if_fail (child == NULL || GTK_IS_WIDGET (child));

  if (self->child == child)
    return;

  if (child != NULL)
    g_return_if_fail (gtk_widget_get_parent (child) == NULL);

  g_clear_pointer (&self->child, gtk_widget_unparent);
  self->child = child;

  if (child != NULL)
    gtk_widget_set_parent (child, GTK_WIDGET (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CHILD]);
}

/**
 * pastry_glass_frame_get_child
 * @self: a `PastryGlassFrame`
 *
 * Gets the child widget of @self.
 *
 * Returns: (nullable) (transfer none): the child widget of @self
 */
GtkWidget *
pastry_glass_frame_get_child (PastryGlassFrame *self)
{
  g_return_val_if_fail (PASTRY_IS_GLASS_FRAME (self), NULL);
  return self->child;
}
