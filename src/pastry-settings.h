/* pastry-settings.h
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

#include <pastry-theme.h>

G_BEGIN_DECLS

#define PASTRY_TYPE_SETTINGS (pastry_settings_get_type ())
G_DECLARE_FINAL_TYPE (PastrySettings, pastry_settings, PASTRY, SETTINGS, GObject)

LIBPASTRY_AVAILABLE_IN_ALL
PastrySettings *
pastry_settings_get_default (void);

LIBPASTRY_AVAILABLE_IN_ALL
void
pastry_copy_accent_rgba (GdkRGBA *rgba);

LIBPASTRY_AVAILABLE_IN_ALL
void
pastry_settings_set_theme (PastrySettings *self,
                           PastryTheme    *theme);
LIBPASTRY_AVAILABLE_IN_ALL
PastryTheme *
pastry_settings_get_theme (PastrySettings *self);

G_END_DECLS
