/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
/*
 * This file is part of Devhelp.
 *
 * Copyright (C) 2001 Mikael Hallendal <micke@imendio.com>
 * Copyright (C) 2004, 2008 Imendio AB
 * Copyright (C) 2015, 2017, 2018 Sébastien Wilmet <swilmet@gnome.org>
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

#include "dh-util-app.h"

static void
introspect_window_gsettings (GSettings *window_settings,
                             gboolean  *has_required_keys,
                             gboolean  *has_maximized_key)
{
        GSettingsSchema *schema = NULL;

        g_object_get (window_settings,
                      "settings-schema", &schema,
                      NULL);

        *has_required_keys = (g_settings_schema_has_key (schema, "width") &&
                              g_settings_schema_has_key (schema, "height"));

        *has_maximized_key = g_settings_schema_has_key (schema, "maximized");

        g_settings_schema_unref (schema);
}

void
dh_util_window_settings_save (GtkWindow *window,
                              GSettings *settings)
{
        gboolean has_required_keys;
        gboolean has_maximized_key;
        gint width;
        gint height;

        g_return_if_fail (GTK_IS_WINDOW (window));
        g_return_if_fail (G_IS_SETTINGS (settings));

        introspect_window_gsettings (settings, &has_required_keys, &has_maximized_key);
        g_return_if_fail (has_required_keys);

        if (has_maximized_key) {
                GdkWindowState state;
                gboolean maximized;

                state = gdk_window_get_state (gtk_widget_get_window (GTK_WIDGET (window)));
                maximized = (state & GDK_WINDOW_STATE_MAXIMIZED) != 0;

                g_settings_set_boolean (settings, "maximized", maximized);

                /* If maximized don't save the size. */
                if (maximized)
                        return;
        }

        /* Store the dimensions */
        gtk_window_get_size (GTK_WINDOW (window), &width, &height);
        g_settings_set_int (settings, "width", width);
        g_settings_set_int (settings, "height", height);
}

/* This should be called when @gtk_window is realized (i.e. its GdkWindow is
 * created) but not yet mapped (i.e. gtk_widget_show() has not yet been called,
 * so that when it is shown it already has the good size).
 */
void
dh_util_window_settings_restore (GtkWindow *gtk_window,
                                 GSettings *settings)
{
        gboolean has_required_keys;
        gboolean has_maximized_key;
        gint width;
        gint height;

        g_return_if_fail (GTK_IS_WINDOW (gtk_window));
        g_return_if_fail (gtk_widget_get_realized (GTK_WIDGET (gtk_window)));
        g_return_if_fail (G_IS_SETTINGS (settings));

        introspect_window_gsettings (settings, &has_required_keys, &has_maximized_key);
        g_return_if_fail (has_required_keys);

        width = g_settings_get_int (settings, "width");
        height = g_settings_get_int (settings, "height");

        if (width > 1 && height > 1) {
                GdkDisplay *display;
                GdkWindow *gdk_window;
                GdkMonitor *monitor;
                GdkRectangle monitor_workarea;
                gint max_width;
                gint max_height;

                display = gtk_widget_get_display (GTK_WIDGET (gtk_window));
                /* To get the GdkWindow the widget must be realized. */
                gdk_window = gtk_widget_get_window (GTK_WIDGET (gtk_window));
                monitor = gdk_display_get_monitor_at_window (display, gdk_window);
                gdk_monitor_get_workarea (monitor, &monitor_workarea);

                max_width = monitor_workarea.width;
                max_height = monitor_workarea.height;

                width = CLAMP (width, 0, max_width);
                height = CLAMP (height, 0, max_height);

                gtk_window_set_default_size (gtk_window, width, height);
        }

        if (has_maximized_key && g_settings_get_boolean (settings, "maximized"))
                gtk_window_maximize (gtk_window);
}
