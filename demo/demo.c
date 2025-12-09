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
  GtkWidget *window = NULL;

  window = gtk_application_window_new (app);

  {
    GtkWidget *box = NULL;

    box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 5);
    gtk_window_set_child (GTK_WINDOW (window), box);

    gtk_widget_set_margin_start (box, 5);
    gtk_widget_set_margin_end (box, 5);
    gtk_widget_set_margin_top (box, 5);
    gtk_widget_set_margin_bottom (box, 5);

    {
      GtkWidget *label = NULL;

      label = gtk_label_new ("Hello World!");
      gtk_box_append (GTK_BOX (box), label);

      gtk_widget_set_vexpand (label, TRUE);
      gtk_widget_add_css_class (label, "title-1");
    }

    {
      GtkWidget *button = NULL;

      button = gtk_button_new_with_label ("Hello World!");
      gtk_box_append (GTK_BOX (box), button);

      gtk_widget_add_css_class (button, "title-1");
    }
  }

  gtk_window_present (GTK_WINDOW (window));
}
