/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     dongsen <dongsen@deepin.com>
 *
 * Maintainer: dongsen <dongsen@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef KCOREADDONS_EXPORT_H
#define KCOREADDONS_EXPORT_H

#ifdef KCOREADDONS_STATIC_DEFINE
#  define KCOREADDONS_EXPORT
#  define KCOREADDONS_NO_EXPORT
#else
#  ifndef KCOREADDONS_EXPORT
#    ifdef KF5CoreAddons_EXPORTS
        /* We are building this library */
#      define KCOREADDONS_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define KCOREADDONS_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef KCOREADDONS_NO_EXPORT
#    define KCOREADDONS_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef KCOREADDONS_DEPRECATED
#  define KCOREADDONS_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef KCOREADDONS_DEPRECATED_EXPORT
#  define KCOREADDONS_DEPRECATED_EXPORT KCOREADDONS_EXPORT KCOREADDONS_DEPRECATED
#endif

#ifndef KCOREADDONS_DEPRECATED_NO_EXPORT
#  define KCOREADDONS_DEPRECATED_NO_EXPORT KCOREADDONS_NO_EXPORT KCOREADDONS_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef KCOREADDONS_NO_DEPRECATED
#    define KCOREADDONS_NO_DEPRECATED
#  endif
#endif

#endif /* KCOREADDONS_EXPORT_H */
