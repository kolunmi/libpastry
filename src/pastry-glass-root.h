/* pastry-glass-root.h
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

#define PASTRY_TYPE_GLASS_ROOT (pastry_glass_root_get_type ())
G_DECLARE_FINAL_TYPE (PastryGlassRoot, pastry_glass_root, PASTRY, GLASS_ROOT, GtkWidget)

LIBPASTRY_AVAILABLE_IN_ALL
void
pastry_glass_root_set_child (PastryGlassRoot *self,
                             GtkWidget       *child);

LIBPASTRY_AVAILABLE_IN_ALL
GtkWidget *
pastry_glass_root_get_child (PastryGlassRoot *self);

LIBPASTRY_AVAILABLE_IN_ALL
void
pastry_glass_root_set_capacity (PastryGlassRoot *self,
                                int              capacity);

LIBPASTRY_AVAILABLE_IN_ALL
int
pastry_glass_root_get_capacity (PastryGlassRoot *self);

LIBPASTRY_AVAILABLE_IN_ALL
void
pastry_glass_root_set_blur_radius (PastryGlassRoot *self,
                                   double           blur_radius);

LIBPASTRY_AVAILABLE_IN_ALL
double
pastry_glass_root_get_blur_radius (PastryGlassRoot *self);

G_END_DECLS
