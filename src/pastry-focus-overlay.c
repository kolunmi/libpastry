/* pastry-focus-overlay.c
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
 * PastryFocusOverlay:
 *
 * Renders a special focus ring around child widgets
 */

#define G_LOG_DOMAIN "PASTRY::FOCUS-OVERLAY"

#include "config.h"

#include "pastry-animation.h"
#include "pastry-focus-overlay.h"
#include "pastry-property-trail.h"
#include "pastry-util.h"

enum
{
  PROP_0,

  PROP_CHILD,

  LAST_PROP
};
static GParamSpec *props[LAST_PROP] = { 0 };

struct _PastryFocusOverlay
{
  GtkWidget parent_instance;

  GtkWidget *child;

  GtkWidget           *frame;
  PastryPropertyTrail *focus_trail;
  PastryAnimation     *animation;

  GtkWidget      *focus_widget;
  graphene_rect_t frame_pos;
};

G_DEFINE_FINAL_TYPE (PastryFocusOverlay, pastry_focus_overlay, GTK_TYPE_WIDGET)

static void
focus_changed_cb (PastryFocusOverlay  *self,
                  GtkWidget           *widget,
                  PastryPropertyTrail *trail);

static void
animate (PastryFocusOverlay *self,
         const char         *key,
         double              value,
         gpointer            user_data);

static void
dispose (GObject *object)
{
  PastryFocusOverlay *self = PASTRY_FOCUS_OVERLAY (object);

  g_signal_handlers_disconnect_by_func (
      self->focus_trail, focus_changed_cb, self);

  pastry_clear_pointers (
      &self->child, gtk_widget_unparent,
      &self->frame, gtk_widget_unparent,
      &self->focus_trail, g_object_unref,
      &self->animation, g_object_unref,
      &self->focus_widget, g_object_unref,
      NULL);

  G_OBJECT_CLASS (pastry_focus_overlay_parent_class)->dispose (object);
}

static void
get_property (GObject    *object,
              guint       prop_id,
              GValue     *value,
              GParamSpec *pspec)
{
  PastryFocusOverlay *self = PASTRY_FOCUS_OVERLAY (object);

  switch (prop_id)
    {
    case PROP_CHILD:
      g_value_set_object (value, pastry_focus_overlay_get_child (self));
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
  PastryFocusOverlay *self = PASTRY_FOCUS_OVERLAY (object);

  switch (prop_id)
    {
    case PROP_CHILD:
      pastry_focus_overlay_set_child (self, g_value_get_object (value));
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
  PastryFocusOverlay *self = PASTRY_FOCUS_OVERLAY (widget);

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
  PastryFocusOverlay *self    = PASTRY_FOCUS_OVERLAY (widget);
  g_autoptr (GtkWidget) focus = NULL;

  if (self->child != NULL && gtk_widget_should_layout (self->child))
    gtk_widget_allocate (self->child, width, height, baseline, NULL);

  focus = pastry_property_trail_dup_resolved (self->focus_trail);
  if (focus != NULL &&
      gtk_widget_is_ancestor (focus, widget))
    {
      graphene_rect_t bounds  = { 0 };
      gboolean        success = FALSE;

      success = gtk_widget_compute_bounds (focus, widget, &bounds);
      if (success)
        {
          bounds.origin.x += self->frame_pos.origin.x;
          bounds.origin.y += self->frame_pos.origin.y;
          bounds.size.width += self->frame_pos.size.width;
          bounds.size.height += self->frame_pos.size.height;
          graphene_rect_inset (&bounds, -5.0, -5.0);

          gtk_widget_set_visible (self->frame, TRUE);
          gtk_widget_allocate (
              self->frame,
              bounds.size.width, bounds.size.height,
              baseline,
              gsk_transform_translate (NULL, &bounds.origin));
        }
      else
        gtk_widget_set_visible (self->frame, FALSE);
    }
  else
    gtk_widget_set_visible (self->frame, FALSE);
}

static void
snapshot (GtkWidget   *widget,
          GtkSnapshot *snapshot)
{
  PastryFocusOverlay *self = PASTRY_FOCUS_OVERLAY (widget);

  if (self->child != NULL)
    gtk_widget_snapshot_child (widget, self->child, snapshot);

  if (gtk_widget_should_layout (self->frame))
    {
      GskRoundedRect rrect = { 0 };

      g_assert (gtk_widget_compute_bounds (self->frame, widget, &rrect.bounds));
      graphene_rect_inset (&rrect.bounds, 5.0, 5.0);
      for (guint i = 0; i < G_N_ELEMENTS (rrect.corner); i++)
        {
          rrect.corner[i].width  = 15.0;
          rrect.corner[i].height = 15.0;
        }

      gtk_snapshot_push_mask (snapshot, GSK_MASK_MODE_INVERTED_ALPHA);
      gtk_snapshot_push_rounded_clip (snapshot, &rrect);
      gtk_snapshot_append_color (
          snapshot,
          &(GdkRGBA) {
              .red   = 0.0,
              .green = 0.0,
              .blue  = 0.0,
              .alpha = 1.0,
          },
          &rrect.bounds);
      gtk_snapshot_pop (snapshot);
      gtk_snapshot_pop (snapshot);
      gtk_widget_snapshot_child (widget, self->frame, snapshot);
      gtk_snapshot_pop (snapshot);
    }
}

static void
pastry_focus_overlay_class_init (PastryFocusOverlayClass *klass)
{
  GObjectClass   *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->set_property = set_property;
  object_class->get_property = get_property;
  object_class->dispose      = dispose;

  /**
   * PastryFocusOverlay:child:
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

  gtk_widget_class_set_css_name (widget_class, "pastry-focus-overlay");
}

static void
pastry_focus_overlay_init (PastryFocusOverlay *self)
{
  self->frame = gtk_frame_new (NULL);
  gtk_widget_set_visible (self->frame, FALSE);
  gtk_widget_set_parent (self->frame, GTK_WIDGET (self));

  self->focus_trail = pastry_property_trail_new (
      self, "root", "focus-widget", NULL);
  g_signal_connect_swapped (
      self->focus_trail, "changed",
      G_CALLBACK (focus_changed_cb), self);

  self->animation = pastry_animation_new (GTK_WIDGET (self));
}

/**
 * pastry_focus_overlay_set_child:
 * @self: a `PastryFocusOverlay`
 * @child: the child widget
 *
 * Sets the child widget
 */
void
pastry_focus_overlay_set_child (PastryFocusOverlay *self,
                                GtkWidget          *child)
{
  g_return_if_fail (PASTRY_IS_FOCUS_OVERLAY (self));

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
 * pastry_focus_overlay_get_child
 * @self: a `PastryFocusOverlay`
 *
 * Gets the child widget of @self.
 *
 * Returns: (nullable) (transfer none): the child widget of @self
 */
GtkWidget *
pastry_focus_overlay_get_child (PastryFocusOverlay *self)
{
  g_return_val_if_fail (PASTRY_IS_FOCUS_OVERLAY (self), NULL);
  return self->child;
}

static void
focus_changed_cb (PastryFocusOverlay  *self,
                  GtkWidget           *widget,
                  PastryPropertyTrail *trail)
{
  gboolean        success      = FALSE;
  graphene_rect_t old_bounds   = { 0 };
  graphene_rect_t new_bounds   = { 0 };
  graphene_rect_t animate_from = { 0 };

  if (widget == NULL)
    {
      g_clear_object (&self->focus_widget);
      gtk_widget_queue_allocate (GTK_WIDGET (self));
      return;
    }

  if (self->focus_widget != NULL)
    {
      success = gtk_widget_compute_bounds (
          widget, GTK_WIDGET (self), &old_bounds);
      if (success)
        {
          success = gtk_widget_compute_bounds (
              self->focus_widget, GTK_WIDGET (self), &new_bounds);
          if (success)
            {
              animate_from.origin.x    = new_bounds.origin.x - (old_bounds.origin.x - self->frame_pos.origin.x);
              animate_from.origin.y    = new_bounds.origin.y - (old_bounds.origin.y - self->frame_pos.origin.y);
              animate_from.size.width  = new_bounds.size.width - (old_bounds.size.width - self->frame_pos.size.width);
              animate_from.size.height = new_bounds.size.height - (old_bounds.size.height - self->frame_pos.size.height);
            }
        }
    }
  g_clear_object (&self->focus_widget);
  self->focus_widget = g_object_ref (widget);

#define DAMPING_RATIO 1.0
#define MASS          1.0
#define STIFFNESS     0.25

  pastry_animation_add_spring (
      self->animation,
      "x",
      animate_from.origin.x, 0.0,
      DAMPING_RATIO, MASS, STIFFNESS,
      (PastryAnimationCallback) animate,
      NULL, NULL);
  pastry_animation_add_spring (
      self->animation,
      "y",
      animate_from.origin.y, 0.0,
      DAMPING_RATIO, MASS, STIFFNESS,
      (PastryAnimationCallback) animate,
      NULL, NULL);
  pastry_animation_add_spring (
      self->animation,
      "w",
      animate_from.size.width, 0.0,
      DAMPING_RATIO, MASS, STIFFNESS,
      (PastryAnimationCallback) animate,
      NULL, NULL);
  pastry_animation_add_spring (
      self->animation,
      "h",
      animate_from.size.height, 0.0,
      DAMPING_RATIO, MASS, STIFFNESS,
      (PastryAnimationCallback) animate,
      NULL, NULL);
}

static void
animate (PastryFocusOverlay *self,
         const char         *key,
         double              value,
         gpointer            user_data)
{
  if (g_strcmp0 (key, "x") == 0)
    self->frame_pos.origin.x = value;
  else if (g_strcmp0 (key, "y") == 0)
    self->frame_pos.origin.y = value;
  else if (g_strcmp0 (key, "w") == 0)
    self->frame_pos.size.width = value;
  else if (g_strcmp0 (key, "h") == 0)
    self->frame_pos.size.height = value;

  gtk_widget_queue_allocate (GTK_WIDGET (self));
}
