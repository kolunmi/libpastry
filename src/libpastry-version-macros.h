/* libpastry-version-macros.h
 *
 * Copyright 2025 Eva M
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#include <glib.h>

#define LIBPASTRY_INSIDE
#include "libpastry-version.h"
#undef LIBPASTRY_INSIDE

#ifndef _LIBPASTRY_EXTERN
#define _LIBPASTRY_EXTERN extern
#endif

#define LIBPASTRY_VERSION_CUR_STABLE (G_ENCODE_VERSION (LIBPASTRY_MAJOR_VERSION, 0))

#ifdef LIBPASTRY_DISABLE_DEPRECATION_WARNINGS
#define LIBPASTRY_DEPRECATED            _LIBPASTRY_EXTERN
#define LIBPASTRY_DEPRECATED_FOR(f)     _LIBPASTRY_EXTERN
#define LIBPASTRY_UNAVAILABLE(maj, min) _LIBPASTRY_EXTERN
#else
#define LIBPASTRY_DEPRECATED            G_DEPRECATED _LIBPASTRY_EXTERN
#define LIBPASTRY_DEPRECATED_FOR(f)     G_DEPRECATED_FOR (f) _LIBPASTRY_EXTERN
#define LIBPASTRY_UNAVAILABLE(maj, min) G_UNAVAILABLE (maj, min) _LIBPASTRY_EXTERN
#endif

#define LIBPASTRY_VERSION_1_0 (G_ENCODE_VERSION (1, 0))

#if LIBPASTRY_MAJOR_VERSION == LIBPASTRY_VERSION_1_0
#define LIBPASTRY_VERSION_PREV_STABLE (LIBPASTRY_VERSION_1_0)
#else
#define LIBPASTRY_VERSION_PREV_STABLE (G_ENCODE_VERSION (LIBPASTRY_MAJOR_VERSION - 1, 0))
#endif

#ifndef LIBPASTRY_VERSION_MIN_REQUIRED
#define LIBPASTRY_VERSION_MIN_REQUIRED (LIBPASTRY_VERSION_CUR_STABLE)
#endif

#ifndef LIBPASTRY_VERSION_MAX_ALLOWED
#if LIBPASTRY_VERSION_MIN_REQUIRED > LIBPASTRY_VERSION_PREV_STABLE
#define LIBPASTRY_VERSION_MAX_ALLOWED (LIBPASTRY_VERSION_MIN_REQUIRED)
#else
#define LIBPASTRY_VERSION_MAX_ALLOWED (LIBPASTRY_VERSION_CUR_STABLE)
#endif
#endif

#define LIBPASTRY_AVAILABLE_IN_ALL _LIBPASTRY_EXTERN

#if LIBPASTRY_VERSION_MIN_REQUIRED >= LIBPASTRY_VERSION_1_0
#define LIBPASTRY_DEPRECATED_IN_1_0        LIBPASTRY_DEPRECATED
#define LIBPASTRY_DEPRECATED_IN_1_0_FOR(f) LIBPASTRY_DEPRECATED_FOR (f)
#else
#define LIBPASTRY_DEPRECATED_IN_1_0        _LIBPASTRY_EXTERN
#define LIBPASTRY_DEPRECATED_IN_1_0_FOR(f) _LIBPASTRY_EXTERN
#endif
#if LIBPASTRY_VERSION_MAX_ALLOWED < LIBPASTRY_VERSION_1_0
#define LIBPASTRY_AVAILABLE_IN_1_0 LIBPASTRY_UNAVAILABLE (1, 0)
#else
#define LIBPASTRY_AVAILABLE_IN_1_0 _LIBPASTRY_EXTERN
#endif
