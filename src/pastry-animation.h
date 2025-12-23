/* pastry-animation.h
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

#define PASTRY_TYPE_ANIMATION (pastry_animation_get_type ())
G_DECLARE_FINAL_TYPE (PastryAnimation, pastry_animation, PASTRY, ANIMATION, GObject)

typedef void (*PastryAnimationCallback) (GtkWidget  *widget,
                                         const char *key,
                                         double      value,
                                         gpointer    user_data);

PastryAnimation *
pastry_animation_new (GtkWidget *widget);

GtkWidget *
pastry_animation_dup_widget (PastryAnimation *self);

void
pastry_animation_add_spring (PastryAnimation        *self,
                             const char             *key,
                             double                  from,
                             double                  to,
                             double                  damping_ratio,
                             double                  mass,
                             double                  stiffness,
                             PastryAnimationCallback cb,
                             gpointer                user_data,
                             GDestroyNotify          destroy_data);

G_END_DECLS
