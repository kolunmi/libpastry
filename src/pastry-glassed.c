/* pastry-glassed.c
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

#define G_LOG_DOMAIN "PASTRY::GLASSED"

#include "config.h"

#include "pastry-glass-root.h"
#include "pastry-glassed.h"

G_DEFINE_INTERFACE (PastryGlassed, pastry_glassed, GTK_TYPE_WIDGET)

static gboolean
pastry_glassed_real_place_glass (PastryGlassed  *self,
                                 GskRoundedRect *dest)
{
  GtkWidget *widget = GTK_WIDGET (self);
  double     width  = 0.0;
  double     height = 0.0;

  width  = gtk_widget_get_width (widget);
  height = gtk_widget_get_height (widget);

  dest->bounds    = GRAPHENE_RECT_INIT (0.0, 0.0, width, height);
  dest->corner[0] = GRAPHENE_SIZE_INIT (0.0, 0.0);
  dest->corner[1] = GRAPHENE_SIZE_INIT (0.0, 0.0);
  dest->corner[2] = GRAPHENE_SIZE_INIT (0.0, 0.0);
  dest->corner[3] = GRAPHENE_SIZE_INIT (0.0, 0.0);

  return TRUE;
}

static void
pastry_glassed_real_snapshot_overlay (PastryGlassed *self,
                                      GtkSnapshot   *snapshot)
{
  return;
}

static void
pastry_glassed_default_init (PastryGlassedInterface *iface)
{
  iface->place_glass      = pastry_glassed_real_place_glass;
  iface->snapshot_overlay = pastry_glassed_real_snapshot_overlay;
}

gboolean
pastry_glassed_place_glass (PastryGlassed  *self,
                            GskRoundedRect *dest)
{
  gboolean ret = FALSE;

  g_return_val_if_fail (PASTRY_IS_GLASSED (self), FALSE);
  g_return_val_if_fail (dest != NULL, FALSE);

  ret = PASTRY_GLASSED_GET_IFACE (self)->place_glass (
      self,
      dest);
  return ret;
}

void
pastry_glassed_snapshot_overlay (PastryGlassed *self,
                                 GtkSnapshot   *snapshot)
{
  g_return_if_fail (PASTRY_IS_GLASSED (self));
  g_return_if_fail (GTK_IS_SNAPSHOT (snapshot));

  PASTRY_GLASSED_GET_IFACE (self)->snapshot_overlay (
      self,
      snapshot);
}

void
pastry_glassed_queue_draw (PastryGlassed *self)
{
  GtkWidget *glass_root = NULL;

  g_return_if_fail (PASTRY_IS_GLASSED (self));

  glass_root = gtk_widget_get_ancestor (GTK_WIDGET (self), PASTRY_TYPE_GLASS_ROOT);
  if (glass_root == NULL)
    {
      g_critical ("%s object lacks a %s ancestor, so it cannot queue a a redraw",
                  g_type_name (PASTRY_TYPE_GLASSED), g_type_name (PASTRY_TYPE_GLASS_ROOT));
      return;
    }

  gtk_widget_queue_allocate (glass_root);
  gtk_widget_queue_draw (glass_root);
}
