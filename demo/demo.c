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

static void
set_theme_cb (GtkDropDown *drop_down,
              GParamSpec  *pspec,
              gpointer     user_data);

int
main (int argc, char **argv)
{
  g_autoptr (GtkApplication) app      = NULL;
  g_autoptr (GtkCssProvider) provider = NULL;

  pastry_init ();

  app = gtk_application_new (
      "em.libpastry.Demo",
      G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect (app, "activate", G_CALLBACK (on_activate), NULL);

  provider = gtk_css_provider_new ();
  gtk_css_provider_load_from_resource (provider, "/io/github/kolunmi/PastryDemo/style.css");
  gtk_style_context_add_provider_for_display (
      gdk_display_get_default (),
      GTK_STYLE_PROVIDER (provider),
      GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

  return g_application_run (G_APPLICATION (app), argc, argv);
}

static void
on_activate (GtkApplication *app)
{
  GtkWidget *window                 = NULL;
  GtkWidget *root                   = NULL;
  g_autoptr (GtkBuilder) builder    = NULL;
  g_autoptr (GtkBuilderScope) scope = NULL;

  window = gtk_application_window_new (app);

  scope = gtk_builder_cscope_new ();
  gtk_builder_cscope_add_callback (scope, set_theme_cb);

  builder = gtk_builder_new ();
  gtk_builder_set_scope (builder, scope);
  gtk_builder_add_from_resource (builder, "/io/github/kolunmi/PastryDemo/window.ui", NULL);
  root = GTK_WIDGET (gtk_builder_get_object (builder, "root"));

  gtk_window_set_child (GTK_WINDOW (window), g_object_ref_sink (root));
  gtk_window_present (GTK_WINDOW (window));
}

static void
set_theme_cb (GtkDropDown *drop_down,
              GParamSpec  *pspec,
              gpointer     user_data)
{
  GtkStringObject *string = NULL;
  const char      *name   = NULL;
  const char      *accent = NULL;

  string = gtk_drop_down_get_selected_item (drop_down);
  name   = gtk_string_object_get_string (string);

  if (name != NULL)
    {
      if (g_strcmp0 (name, "Default") == 0)
        accent = "#dfdfe7";
      else if (g_strcmp0 (name, "Purple") == 0)
        accent = "#9c63dd";
      else if (g_strcmp0 (name, "Green") == 0)
        accent = "#28c900";
      else if (g_strcmp0 (name, "Blue") == 0)
        accent = "#0fb6ff";
      else if (g_strcmp0 (name, "Red") == 0)
        accent = "#f42634";
      else if (g_strcmp0 (name, "Pink") == 0)
        accent = "#ff8ed7";
      else if (g_strcmp0 (name, "Orange") == 0)
        accent = "#ff8600";
    }

  if (accent != NULL)
    {
      g_autoptr (PastryVisualTheme) visual_theme = NULL;
      PastrySettings *settings                   = NULL;
      PastryTheme    *theme                      = NULL;

      visual_theme = g_object_new (
          PASTRY_TYPE_VISUAL_THEME,
          "name", name,
          "accent", accent,
          NULL);

      settings = pastry_settings_get_default ();
      theme    = pastry_settings_get_theme (settings);
      if (theme != NULL)
        pastry_theme_set_visual_theme (theme, visual_theme);
    }
}
