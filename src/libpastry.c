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
  static gboolean initialized = FALSE;

  gtk_init ();

  g_type_ensure (PASTRY_TYPE_ANIMATION);

  if (!initialized)
    {
      g_autoptr (GtkCssProvider) provider = NULL;
      GdkDisplay *display                 = NULL;

      provider = gtk_css_provider_new ();
      gtk_css_provider_load_from_resource (
          provider,
          "/em/libpastry/Pastry/styles/gtk.css");
      g_info ("Loaded stylesheet");

      /* TODO: listen to creation of displays */
      display = gdk_display_get_default ();
      gtk_style_context_add_provider_for_display (
          display,
          GTK_STYLE_PROVIDER (provider),
          GTK_STYLE_PROVIDER_PRIORITY_SETTINGS + 1);

      initialized = TRUE;
    }
}
