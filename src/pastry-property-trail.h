/* pastry-property-trail.h
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

#define PASTRY_TYPE_PROPERTY_TRAIL (pastry_property_trail_get_type ())
G_DECLARE_FINAL_TYPE (PastryPropertyTrail, pastry_property_trail, PASTRY, PROPERTY_TRAIL, GObject)

LIBPASTRY_AVAILABLE_IN_ALL
G_GNUC_NULL_TERMINATED
PastryPropertyTrail *
pastry_property_trail_new (gpointer    object,
                           const char *property,
                           ...);

LIBPASTRY_AVAILABLE_IN_ALL
void
pastry_property_trail_set_object (PastryPropertyTrail *self,
                                  GObject             *object);
LIBPASTRY_AVAILABLE_IN_ALL
GObject *
pastry_property_trail_get_object (PastryPropertyTrail *self);

LIBPASTRY_AVAILABLE_IN_ALL
void
pastry_property_trail_set_trail (PastryPropertyTrail *self,
                                 GListModel          *trail);
LIBPASTRY_AVAILABLE_IN_ALL
GListModel *
pastry_property_trail_get_trail (PastryPropertyTrail *self);

LIBPASTRY_AVAILABLE_IN_ALL
gpointer
pastry_property_trail_dup_resolved (PastryPropertyTrail *self);

G_END_DECLS
