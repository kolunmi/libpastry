/* pastry-annotation-overlay.c
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
 * PastryAnnotationOverlay:
 *
 * Renders tooltip text over focused child widgets in a stylized way
 */

#define G_LOG_DOMAIN "PASTRY::ANNOTATION-OVERLAY"

#include "config.h"

#include "pastry-annotation-overlay.h"
#include "pastry-glassed.h"
#include "pastry-property-trail.h"
#include "pastry-util.h"

enum
{
  PROP_0,

  PROP_CHILD,

  LAST_PROP
};
static GParamSpec *props[LAST_PROP] = { 0 };

struct _PastryAnnotationOverlay
{
  GtkWidget parent_instance;

  GtkWidget *child;

  GtkWidget           *label;
  PastryPropertyTrail *focus_trail;
};

static void
glassed_iface_init (PastryGlassedInterface *iface);

G_DEFINE_FINAL_TYPE_WITH_CODE (
    PastryAnnotationOverlay,
    pastry_annotation_overlay,
    GTK_TYPE_WIDGET,
    G_IMPLEMENT_INTERFACE (PASTRY_TYPE_GLASSED, glassed_iface_init))

static void
focus_changed_cb (PastryAnnotationOverlay *self,
                  GtkWidget               *widget,
                  PastryPropertyTrail     *trail);

static void
dispose (GObject *object)
{
  PastryAnnotationOverlay *self = PASTRY_ANNOTATION_OVERLAY (object);

  g_signal_handlers_disconnect_by_func (
      self->focus_trail, focus_changed_cb, self);

  pastry_clear_pointers (
      &self->child, gtk_widget_unparent,
      &self->label, gtk_widget_unparent,
      &self->focus_trail, g_object_unref,
      NULL);

  G_OBJECT_CLASS (pastry_annotation_overlay_parent_class)->dispose (object);
}

static void
get_property (GObject    *object,
              guint       prop_id,
              GValue     *value,
              GParamSpec *pspec)
{
  PastryAnnotationOverlay *self = PASTRY_ANNOTATION_OVERLAY (object);

  switch (prop_id)
    {
    case PROP_CHILD:
      g_value_set_object (value, pastry_annotation_overlay_get_child (self));
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
  PastryAnnotationOverlay *self = PASTRY_ANNOTATION_OVERLAY (object);

  switch (prop_id)
    {
    case PROP_CHILD:
      pastry_annotation_overlay_set_child (self, g_value_get_object (value));
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
  PastryAnnotationOverlay *self = PASTRY_ANNOTATION_OVERLAY (widget);

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
  PastryAnnotationOverlay *self = PASTRY_ANNOTATION_OVERLAY (widget);

  if (self->child != NULL && gtk_widget_should_layout (self->child))
    gtk_widget_allocate (self->child, width, height, baseline, NULL);
}

static void
snapshot (GtkWidget   *widget,
          GtkSnapshot *snapshot)
{
  PastryAnnotationOverlay *self = PASTRY_ANNOTATION_OVERLAY (widget);

  if (self->child != NULL)
    gtk_widget_snapshot_child (widget, self->child, snapshot);
}

static void
pastry_annotation_overlay_class_init (PastryAnnotationOverlayClass *klass)
{
  GObjectClass   *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->set_property = set_property;
  object_class->get_property = get_property;
  object_class->dispose      = dispose;

  /**
   * PastryAnnotationOverlay:child:
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

  gtk_widget_class_set_css_name (widget_class, "pastry-annotation-overlay");
}

static void
pastry_annotation_overlay_init (PastryAnnotationOverlay *self)
{
  self->label = gtk_label_new (NULL);
  gtk_widget_set_halign (self->label, GTK_ALIGN_CENTER);
  gtk_widget_set_parent (self->label, GTK_WIDGET (self));

  self->focus_trail = pastry_property_trail_new (
      self, "root", "focus-widget", NULL);
  g_signal_connect_swapped (
      self->focus_trail, "changed",
      G_CALLBACK (focus_changed_cb), self);
}

static gboolean
place_glass (PastryGlassed  *glassed,
             GskRoundedRect *dest)
{
  PastryAnnotationOverlay *self      = PASTRY_ANNOTATION_OVERLAY (glassed);
  GtkWidget               *widget    = GTK_WIDGET (glassed);
  g_autoptr (GtkWidget) focused      = NULL;
  double           width             = 0.0;
  double           height            = 0.0;
  int              baseline          = -1;
  graphene_rect_t  focus_bounds      = { 0 };
  graphene_rect_t  label_bounds      = { 0 };
  gboolean         success           = FALSE;
  graphene_point_t tl                = { 0 };
  graphene_point_t tr                = { 0 };
  graphene_point_t bl                = { 0 };
  g_autoptr (GskTransform) transform = NULL;

  focused = pastry_property_trail_dup_resolved (self->focus_trail);
  if (focused == NULL)
    return FALSE;

  width    = gtk_widget_get_width (widget);
  height   = gtk_widget_get_height (widget);
  baseline = gtk_widget_get_baseline (widget);

  success = gtk_widget_compute_bounds (focused, widget, &focus_bounds);
  if (!success)
    return FALSE;
  graphene_rect_get_top_left (&focus_bounds, &tl);
  graphene_rect_get_top_right (&focus_bounds, &tr);
  graphene_rect_get_bottom_left (&focus_bounds, &bl);

#define MARGIN 5.0
  if (tl.y > height - bl.y)
    {
      label_bounds.origin.y    = 0;
      label_bounds.size.height = tl.y - MARGIN;
      gtk_widget_set_valign (self->label, GTK_ALIGN_END);
    }
  else
    {
      label_bounds.origin.y    = bl.y + MARGIN;
      label_bounds.size.height = height - bl.y - MARGIN;
      gtk_widget_set_valign (self->label, GTK_ALIGN_START);
    }
#undef MARGIN

  if (tl.x < width - tr.x)
    {
      label_bounds.size.width = tl.x + tr.x;
      label_bounds.origin.x   = 0.0;
    }
  else
    {
      label_bounds.size.width = (width - tl.x) + (width - tr.x);
      label_bounds.origin.x   = width - label_bounds.size.width;
    }

  transform = gsk_transform_translate (
      NULL, &GRAPHENE_POINT_INIT (label_bounds.origin.x, label_bounds.origin.y));
  gtk_widget_allocate (
      self->label,
      label_bounds.size.width,
      label_bounds.size.height,
      baseline,
      g_steal_pointer (&transform));

  g_assert (gtk_widget_compute_bounds (self->label, widget, &dest->bounds));
  return TRUE;
}

static void
snapshot_overlay (PastryGlassed *glassed,
                  GtkSnapshot   *snapshot)
{
  PastryAnnotationOverlay *self = PASTRY_ANNOTATION_OVERLAY (glassed);

  gtk_widget_snapshot_child (GTK_WIDGET (self), self->label, snapshot);
}

static void
glassed_iface_init (PastryGlassedInterface *iface)
{
  iface->place_glass      = place_glass;
  iface->snapshot_overlay = snapshot_overlay;
}

/**
 * pastry_annotation_overlay_set_child:
 * @self: a `PastryAnnotationOverlay`
 * @child: the child widget
 *
 * Sets the child widget
 */
void
pastry_annotation_overlay_set_child (PastryAnnotationOverlay *self,
                                     GtkWidget               *child)
{
  g_return_if_fail (PASTRY_IS_ANNOTATION_OVERLAY (self));

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
 * pastry_annotation_overlay_get_child
 * @self: a `PastryAnnotationOverlay`
 *
 * Gets the child widget of @self.
 *
 * Returns: (nullable) (transfer none): the child widget of @self
 */
GtkWidget *
pastry_annotation_overlay_get_child (PastryAnnotationOverlay *self)
{
  g_return_val_if_fail (PASTRY_IS_ANNOTATION_OVERLAY (self), NULL);
  return self->child;
}

static void
focus_changed_cb (PastryAnnotationOverlay *self,
                  GtkWidget               *widget,
                  PastryPropertyTrail     *trail)
{
  const char *old_label = NULL;
  const char *new_label = NULL;

  old_label = gtk_label_get_label (GTK_LABEL (self->label));
  if (GTK_IS_WIDGET (widget))
    new_label = gtk_widget_get_tooltip_text (widget);

  if (old_label == NULL &&
      new_label == NULL)
    return;

  gtk_label_set_label (GTK_LABEL (self->label), new_label);
  pastry_glassed_queue_draw (PASTRY_GLASSED (self));
}
