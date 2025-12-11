/* pastry-util.h
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

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define pastry_clear_free(_loc) g_clear_pointer ((_loc), g_free)

G_GNUC_NULL_TERMINATED
void
pastry_clear_pointers (gpointer first_ptr,
                       gpointer free_func,
                       ...);

G_GNUC_NULL_TERMINATED
void
pastry_clear_frees (gpointer first_ptr,
                    ...);

G_GNUC_NULL_TERMINATED
void
pastry_clear_objects (gpointer first_object,
                      ...);

gboolean
pastry_get_valist (gpointer    object,
                   GValue     *value,
                   const char *property,
                   va_list     var_args);

#define _DEFINE_GETTER(_type, _func_type, _get, _orelse)             \
  G_GNUC_NULL_TERMINATED                                             \
  G_GNUC_UNUSED                                                      \
  static inline _type                                                \
  pastry_get_##_func_type (gpointer    object,                       \
                           const char *property,                     \
                           ...)                                      \
  {                                                                  \
    va_list  var_args = { 0 };                                       \
    gboolean result   = FALSE;                                       \
    GValue   value    = G_VALUE_INIT;                                \
                                                                     \
    va_start (var_args, property);                                   \
    result = pastry_get_valist (object, &value, property, var_args); \
    va_end (var_args);                                               \
                                                                     \
    if (result)                                                      \
      return g_value_##_get (&value);                                \
    else                                                             \
      return (_orelse);                                              \
  }

_DEFINE_GETTER (gchar, char, get_schar, 0);
_DEFINE_GETTER (guchar, uchar, get_uchar, 0);
_DEFINE_GETTER (gboolean, boolean, get_boolean, 0);
_DEFINE_GETTER (gint, int, get_int, 0);
_DEFINE_GETTER (guint, uint, get_uint, 0);
_DEFINE_GETTER (glong, long, get_long, 0);
_DEFINE_GETTER (gulong, ulong, get_ulong, 0);
_DEFINE_GETTER (gint64, int64, get_int64, 0);
_DEFINE_GETTER (guint64, uint64, get_uint64, 0);
_DEFINE_GETTER (int, enum, get_enum, 0);
_DEFINE_GETTER (int, flags, get_flags, 0);
_DEFINE_GETTER (gfloat, float, get_float, 0.0);
_DEFINE_GETTER (gdouble, double, get_double, 0.0);

_DEFINE_GETTER (char *, string, dup_string, NULL);
_DEFINE_GETTER (gpointer, object, dup_object, NULL);
_DEFINE_GETTER (gpointer, boxed, dup_boxed, NULL);

#undef _DEFINE_GETTER

G_END_DECLS
