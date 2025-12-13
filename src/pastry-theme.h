/* pastry-theme.h
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

#define PASTRY_TYPE_THEME (pastry_theme_get_type ())
G_DECLARE_FINAL_TYPE (PastryTheme, pastry_theme, PASTRY, THEME, GObject)

LIBPASTRY_AVAILABLE_IN_ALL
void
pastry_theme_set_name (PastryTheme *self,
                       const char  *name);

LIBPASTRY_AVAILABLE_IN_ALL
const char *
pastry_theme_get_name (PastryTheme *self);

LIBPASTRY_AVAILABLE_IN_ALL
void
pastry_theme_set_accent (PastryTheme *self,
                         const char  *accent);

LIBPASTRY_AVAILABLE_IN_ALL
const char *
pastry_theme_get_accent (PastryTheme *self);

LIBPASTRY_AVAILABLE_IN_ALL
void
pastry_theme_copy_accent_rgba (PastryTheme *self,
                               GdkRGBA     *rgba);

G_END_DECLS
