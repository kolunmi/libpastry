/* demo.c
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

#include <libpastry.h>

static void
on_activate (GtkApplication *app);

int
main (int argc, char **argv)
{
  g_autoptr (GtkApplication) app = NULL;

  pastry_init ();

  app = gtk_application_new (
      "em.libpastry.Demo",
      G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect (app, "activate", G_CALLBACK (on_activate), NULL);
  return g_application_run (G_APPLICATION (app), argc, argv);
}

static void
on_activate (GtkApplication *app)
{
  GtkWidget *window              = NULL;
  GtkWidget *root                = NULL;
  g_autoptr (GtkBuilder) builder = NULL;

  window = gtk_application_window_new (app);

  builder = gtk_builder_new ();
  gtk_builder_add_from_resource (builder, "/io/github/kolunmi/PastryDemo/window.ui", NULL);
  root = GTK_WIDGET (gtk_builder_get_object (builder, "root"));

  gtk_window_set_child (GTK_WINDOW (window), g_object_ref_sink (root));
  gtk_window_present (GTK_WINDOW (window));
}
