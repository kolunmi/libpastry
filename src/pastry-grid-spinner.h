/* pastry-grid-spinner.h
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

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define PASTRY_TYPE_GRID_SPINNER (pastry_grid_spinner_get_type ())
G_DECLARE_FINAL_TYPE (PastryGridSpinner, pastry_grid_spinner, PASTRY, GRID_SPINNER, GtkWidget)

void
pastry_grid_spinner_set_speed (PastryGridSpinner *self,
                               double             speed);

double
pastry_grid_spinner_get_speed (PastryGridSpinner *self);

G_END_DECLS
