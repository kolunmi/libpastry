/* pastry-spinner.h
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

#define PASTRY_TYPE_SPINNER (pastry_spinner_get_type ())
G_DECLARE_FINAL_TYPE (PastrySpinner, pastry_spinner, PASTRY, SPINNER, GtkWidget)

void
pastry_spinner_set_speed (PastrySpinner *self,
                          double         speed);

double
pastry_spinner_get_speed (PastrySpinner *self);

void
pastry_spinner_set_n_dots (PastrySpinner *self,
                           int            n_dots);

int
pastry_spinner_get_n_dots (PastrySpinner *self);

G_END_DECLS
