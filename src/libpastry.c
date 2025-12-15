/* libpastry.c
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

#define G_LOG_DOMAIN "PASTRY::CORE"

#include "libpastry.h"

/**
 * pastry_init:
 *
 * Initializes Pastry (And GTK).
 *
 * This function must be called before using any other Pastry functions.
 */
void
pastry_init (void)
{
  gtk_init ();

  g_type_ensure (PASTRY_TYPE_ANIMATION);
  g_type_ensure (PASTRY_TYPE_GRID_SPINNER);
  g_type_ensure (PASTRY_TYPE_PROPERTY_TRAIL);
  g_type_ensure (PASTRY_TYPE_SETTINGS);
  g_type_ensure (PASTRY_TYPE_SOUND_THEME);
  g_type_ensure (PASTRY_TYPE_SPINNER);
  g_type_ensure (PASTRY_TYPE_THEME);
  g_type_ensure (PASTRY_TYPE_VISUAL_THEME);

  (void) pastry_settings_get_default ();
}
