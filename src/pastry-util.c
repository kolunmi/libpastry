/* pastry-util.c
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

#define G_LOG_DOMAIN "PASTRY::UTIL"

#include "config.h"

#include "pastry-util.h"

void
pastry_clear_pointers (gpointer first_ptr,
                       gpointer free_func,
                       ...)
{
  va_list        var_args = { 0 };
  gpointer      *cur_ptr  = NULL;
  GDestroyNotify cur_func = NULL;

  cur_ptr  = first_ptr;
  cur_func = free_func;

  va_start (var_args, free_func);
  for (;;)
    {
      g_clear_pointer (cur_ptr, cur_func);

      cur_ptr = va_arg (var_args, gpointer *);
      if (cur_ptr == NULL)
        break;
      cur_func = va_arg (var_args, GDestroyNotify);
      if (cur_func == NULL)
        {
          g_critical ("%s: pointer has no corresponding free func", G_STRLOC);
          break;
        }
    }
  va_end (var_args);
}

void
pastry_clear_frees (gpointer first_ptr,
                    ...)
{
  va_list   var_args = { 0 };
  gpointer *cur_ptr  = NULL;

  cur_ptr = first_ptr;

  va_start (var_args, first_ptr);
  for (;;)
    {
      g_clear_pointer (cur_ptr, g_free);

      cur_ptr = va_arg (var_args, gpointer *);
      if (cur_ptr == NULL)
        break;
    }
  va_end (var_args);
}

void
pastry_clear_objects (gpointer first_object,
                      ...)
{
  va_list   var_args   = { 0 };
  gpointer *cur_object = NULL;

  cur_object = first_object;

  va_start (var_args, first_object);
  for (;;)
    {
      g_clear_object (cur_object);

      cur_object = va_arg (var_args, gpointer *);
      if (cur_object == NULL)
        break;
    }
  va_end (var_args);
}

gboolean
pastry_get_valist (gpointer    object,
                   GValue     *value,
                   const char *property,
                   va_list     var_args)
{
  gboolean    ret          = FALSE;
  GObject    *cur_object   = NULL;
  const char *cur_property = NULL;
  GValue      cur_value    = G_VALUE_INIT;

  g_return_val_if_fail (G_IS_OBJECT (object), FALSE);
  g_return_val_if_fail (value != NULL, FALSE);
  g_return_val_if_fail (property != NULL, FALSE);

  g_value_unset (value);
  cur_property = property;

  for (;;)
    {
      GValue tmp_value = G_VALUE_INIT;

      if (cur_object == NULL)
        cur_object = object;
      else if (G_VALUE_HOLDS (&cur_value, G_TYPE_OBJECT))
        cur_object = g_value_get_object (&cur_value);
      else
        {
          g_value_unset (&cur_value);
          g_warning ("Cannot get property %s on object of type %s",
                     cur_property, g_type_name (cur_value.g_type));
          break;
        }

      if (cur_object == NULL)
        {
          g_value_unset (&cur_value);
          break;
        }

      g_object_get_property (cur_object, cur_property, &tmp_value);
      g_value_unset (&cur_value);
      cur_value = tmp_value;

      cur_property = va_arg (var_args, const char *);
      if (cur_property == NULL)
        {
          ret = TRUE;
          break;
        }
    }

  *value = cur_value;
  return ret;
}
