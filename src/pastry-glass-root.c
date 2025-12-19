/* pastry-glass-root.c
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
 * PastryGlassRoot:
 *
 * Recursively manages child `PastryGlassed` widgets.
 */

#define G_LOG_DOMAIN "PASTRY::GLASS-ROOT"

#define DEFAULT_N_GLASS_WIDGETS 8

#include "config.h"

#include "pastry-glass-root.h"
#include "pastry-glassed.h"
#include "pastry-util.h"

enum
{
  PROP_0,

  PROP_CHILD,
  PROP_CAPACITY,

  LAST_PROP
};
static GParamSpec *props[LAST_PROP] = { 0 };

struct _PastryGlassRoot
{
  GtkWidget parent_instance;

  GtkWidget *child;

  GPtrArray  *glass_widgets;
  GHashTable *rrects_cache;
};

G_DEFINE_FINAL_TYPE (PastryGlassRoot, pastry_glass_root, GTK_TYPE_WIDGET)

static gboolean
search_glass_allocate (PastryGlassRoot *self,
                       GtkWidget       *widget);

static void
fill_glass_widgets (PastryGlassRoot *self,
                    guint            from);

static void
dispose (GObject *object)
{
  PastryGlassRoot *self = PASTRY_GLASS_ROOT (object);

  pastry_clear_pointers (
      &self->child, gtk_widget_unparent,
      &self->glass_widgets, g_ptr_array_unref,
      &self->rrects_cache, g_hash_table_unref,
      NULL);

  G_OBJECT_CLASS (pastry_glass_root_parent_class)->dispose (object);
}

static void
get_property (GObject    *object,
              guint       prop_id,
              GValue     *value,
              GParamSpec *pspec)
{
  PastryGlassRoot *self = PASTRY_GLASS_ROOT (object);

  switch (prop_id)
    {
    case PROP_CHILD:
      g_value_set_object (value, pastry_glass_root_get_child (self));
      break;
    case PROP_CAPACITY:
      g_value_set_int (value, pastry_glass_root_get_capacity (self));
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
  PastryGlassRoot *self = PASTRY_GLASS_ROOT (object);

  switch (prop_id)
    {
    case PROP_CHILD:
      pastry_glass_root_set_child (self, g_value_get_object (value));
      break;
    case PROP_CAPACITY:
      pastry_glass_root_set_capacity (self, g_value_get_int (value));
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
  PastryGlassRoot *self = PASTRY_GLASS_ROOT (widget);

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
  PastryGlassRoot *self = PASTRY_GLASS_ROOT (widget);

  g_hash_table_remove_all (self->rrects_cache);

  if (self->child != NULL && gtk_widget_should_layout (self->child))
    {
      gtk_widget_allocate (self->child, width, height, baseline, NULL);
      search_glass_allocate (self, self->child);
    }
}

static void
snapshot (GtkWidget   *widget,
          GtkSnapshot *snapshot)
{
  PastryGlassRoot *self                = PASTRY_GLASS_ROOT (widget);
  g_autoptr (GtkSnapshot) tmp_snapshot = NULL;
  GHashTableIter iter                  = { 0 };
  g_autoptr (GskRenderNode) final_node = NULL;

  tmp_snapshot = gtk_snapshot_new ();
  if (self->child != NULL)
    gtk_widget_snapshot_child (widget, self->child, tmp_snapshot);

  g_hash_table_iter_init (&iter, self->rrects_cache);
  for (guint i = 0;; i++)
    {
      PastryGlassed  *glassed        = NULL;
      GskRoundedRect *rrect          = NULL;
      gboolean        valid          = FALSE;
      graphene_rect_t bounds         = { 0 };
      graphene_rect_t clip           = { 0 };
      g_autoptr (GskRenderNode) node = NULL;
      GtkWidget *glass_widget        = NULL;

      valid = g_hash_table_iter_next (
          &iter,
          (gpointer *) &glassed,
          (gpointer *) &rrect);
      if (!valid)
        break;

      valid = gtk_widget_compute_bounds (
          GTK_WIDGET (glassed), widget, &bounds);
      if (!valid)
        continue;

      clip = GRAPHENE_RECT_INIT (
          bounds.origin.x + rrect->bounds.origin.x,
          bounds.origin.y + rrect->bounds.origin.y,
          rrect->bounds.size.width,
          rrect->bounds.size.height);

      node = gtk_snapshot_to_node (tmp_snapshot);
      g_clear_object (&tmp_snapshot);

      tmp_snapshot = gtk_snapshot_new ();

      gtk_snapshot_push_mask (tmp_snapshot, GSK_MASK_MODE_INVERTED_ALPHA);
      gtk_snapshot_append_color (
          tmp_snapshot,
          &(const GdkRGBA) { .alpha = 1.0 },
          &clip);
      gtk_snapshot_pop (tmp_snapshot);
      gtk_snapshot_append_node (tmp_snapshot, node);
      gtk_snapshot_pop (tmp_snapshot);

      gtk_snapshot_push_clip (tmp_snapshot, &clip);
      gtk_snapshot_push_blur (tmp_snapshot, 32.0);
      gtk_snapshot_append_node (tmp_snapshot, node);
      gtk_snapshot_pop (tmp_snapshot);

      gtk_snapshot_save (tmp_snapshot);
      gtk_snapshot_translate (tmp_snapshot, &GRAPHENE_POINT_INIT (bounds.origin.x, bounds.origin.y));

      g_assert (i < self->glass_widgets->len);
      glass_widget = g_ptr_array_index (self->glass_widgets, i);
      gtk_widget_snapshot_child (widget, glass_widget, tmp_snapshot);
      pastry_glassed_snapshot_overlay (glassed, tmp_snapshot);

      gtk_snapshot_restore (tmp_snapshot);
      gtk_snapshot_pop (tmp_snapshot);
    }

  final_node = gtk_snapshot_to_node (tmp_snapshot);
  gtk_snapshot_append_node (snapshot, final_node);
}

static void
pastry_glass_root_class_init (PastryGlassRootClass *klass)
{
  GObjectClass   *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->set_property = set_property;
  object_class->get_property = get_property;
  object_class->dispose      = dispose;

  /**
   * PastryGlassRoot:child:
   *
   * The child widget
   */
  props[PROP_CHILD] =
      g_param_spec_object (
          "child",
          NULL, NULL,
          GTK_TYPE_WIDGET,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * PastryGlassRoot:capacity:
   *
   * The maximum number of `PastryGlassed` child widgets supported
   */
  props[PROP_CAPACITY] =
      g_param_spec_int (
          "capacity",
          NULL, NULL,
          1, 16, DEFAULT_N_GLASS_WIDGETS,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  widget_class->measure       = measure;
  widget_class->size_allocate = size_allocate;
  widget_class->snapshot      = snapshot;

  gtk_widget_class_set_css_name (widget_class, "pastry-glass-root");
}

static void
pastry_glass_root_init (PastryGlassRoot *self)
{
  self->glass_widgets = g_ptr_array_new_with_free_func (
      (GDestroyNotify) gtk_widget_unparent);
  g_ptr_array_set_size (self->glass_widgets, DEFAULT_N_GLASS_WIDGETS);
  fill_glass_widgets (self, 0);

  self->rrects_cache = g_hash_table_new_full (
      g_direct_hash, g_direct_equal, g_object_unref, g_free);
}

/**
 * pastry_glass_root_set_child:
 * @self: a `PastryGlassRoot`
 * @child: the child widget
 *
 * Sets the child widget
 */
void
pastry_glass_root_set_child (PastryGlassRoot *self,
                             GtkWidget       *child)
{
  g_return_if_fail (PASTRY_IS_GLASS_ROOT (self));

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
 * pastry_glass_root_get_child
 * @self: a `PastryGlassRoot`
 *
 * Gets the child widget of @self.
 *
 * Returns: (nullable) (transfer none): the child widget of @self
 */
GtkWidget *
pastry_glass_root_get_child (PastryGlassRoot *self)
{
  g_return_val_if_fail (PASTRY_IS_GLASS_ROOT (self), NULL);
  return self->child;
}

/**
 * pastry_glass_root_set_capacity:
 * @self: a `PastryGlassRoot`
 * @capacity: the maximum number of `PastryGlassed` child widgets to support
 *
 * Sets the maximum number of `PastryGlassed` child widgets to support
 */
void
pastry_glass_root_set_capacity (PastryGlassRoot *self,
                                int              capacity)
{
  guint old_len = 0;

  g_return_if_fail (PASTRY_IS_GLASS_ROOT (self));
  g_return_if_fail (capacity > 0 && capacity <= 64);

  if (capacity == self->glass_widgets->len)
    return;

  old_len = self->glass_widgets->len;
  g_ptr_array_set_size (self->glass_widgets, capacity);
  fill_glass_widgets (self, old_len);

  gtk_widget_queue_draw (GTK_WIDGET (self));
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CAPACITY]);
}

/**
 * pastry_glass_root_get_capacity
 * @self: a `PastryGlassRoot`
 *
 * Gets the maximum number of `PastryGlassed` child widgets supported by @self
 *
 * Returns: the maximum number of `PastryGlassed` child widgets supported by @self
 */
int
pastry_glass_root_get_capacity (PastryGlassRoot *self)
{
  g_return_val_if_fail (PASTRY_IS_GLASS_ROOT (self), 0.0);
  return self->glass_widgets->len;
}

static gboolean
search_glass_allocate (PastryGlassRoot *self,
                       GtkWidget       *widget)
{
  for (GtkWidget *child = gtk_widget_get_first_child (widget);
       child != NULL;
       child = gtk_widget_get_next_sibling (child))
    {
      if (PASTRY_IS_GLASSED (child))
        {
          g_autofree GskRoundedRect *rrect    = NULL;
          gboolean                   do_glass = FALSE;

          rrect    = g_new0 (typeof (*rrect), 1);
          do_glass = pastry_glassed_place_glass (PASTRY_GLASSED (child), rrect);

          if (do_glass)
            {
              guint      idx          = 0;
              GtkWidget *glass_widget = NULL;

              idx = g_hash_table_size (self->rrects_cache);
              g_assert (idx < self->glass_widgets->len);

              glass_widget = g_ptr_array_index (self->glass_widgets, idx);
              gtk_widget_allocate (
                  glass_widget,
                  rrect->bounds.size.width,
                  rrect->bounds.size.height,
                  -1, NULL);

              if (!g_hash_table_contains (self->rrects_cache, child) &&
                  g_hash_table_size (self->rrects_cache) >= self->glass_widgets->len)
                {
                  g_critical ("Too many PastryGlassed children for capacity %d", self->glass_widgets->len);
                  return FALSE;
                }

              g_hash_table_replace (
                  self->rrects_cache,
                  g_object_ref (child),
                  g_steal_pointer (&rrect));
            }
        }

      if (!search_glass_allocate (self, child))
        /* ran out of capacity */
        return FALSE;
    }

  return TRUE;
}

static void
fill_glass_widgets (PastryGlassRoot *self,
                    guint            from)
{
  for (guint i = from; i < self->glass_widgets->len; i++)
    {
      GtkWidget *child = NULL;

      child = gtk_frame_new (NULL);
      gtk_widget_set_parent (child, GTK_WIDGET (self));
      g_ptr_array_index (self->glass_widgets, i) = child;
    }
}
