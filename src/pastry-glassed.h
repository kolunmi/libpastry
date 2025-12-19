/* pastry-glassed.h
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

#pragma once

#ifndef LIBPASTRY_INSIDE
#error "Only <libpastry.h> can be included directly."
#endif

#include "libpastry-version-macros.h"

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define PASTRY_TYPE_GLASSED (pastry_glassed_get_type ())
G_DECLARE_INTERFACE (PastryGlassed, pastry_glassed, PASTRY, GLASSED, GtkWidget)

struct _PastryGlassedInterface
{
  GTypeInterface parent_iface;

  gboolean (*place_glass) (PastryGlassed  *self,
                           GskRoundedRect *dest);

  void (*snapshot_overlay) (PastryGlassed *self,
                            GtkSnapshot   *snapshot);
};

LIBPASTRY_AVAILABLE_IN_ALL
gboolean
pastry_glassed_place_glass (PastryGlassed  *self,
                            GskRoundedRect *dest);

LIBPASTRY_AVAILABLE_IN_ALL
void
pastry_glassed_snapshot_overlay (PastryGlassed *self,
                                 GtkSnapshot   *snapshot);

G_END_DECLS
