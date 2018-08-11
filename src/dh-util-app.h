/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
/*
 * This file is part of Devhelp.
 *
 * Copyright (C) 2001-2002 Mikael Hallendal <micke@imendio.com>
 * Copyright (C) 2004,2008 Imendio AB
 * Copyright (C) 2015, 2017 Sébastien Wilmet <swilmet@gnome.org>
 *
 * Devhelp is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * Devhelp is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Devhelp.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DH_UTIL_APP_H
#define DH_UTIL_APP_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

void    dh_util_window_settings_save            (GtkWindow *window,
                                                 GSettings *settings);

void    dh_util_window_settings_restore         (GtkWindow *gtk_window,
                                                 GSettings *settings);

G_END_DECLS

#endif /* DH_UTIL_APP_H */
