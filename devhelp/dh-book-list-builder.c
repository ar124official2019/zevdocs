/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
/*
 * This file is part of Devhelp.
 *
 * Copyright (C) 2018 Sébastien Wilmet <swilmet@gnome.org>
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

#include "config.h"
#include "dh-book-list-builder.h"
#include "dh-book-list-directory.h"
#include "dh-book-list-simple.h"

/**
 * SECTION:dh-book-list-builder
 * @Title: DhBookListBuilder
 * @Short_description: Builds #DhBookList objects
 *
 * #DhBookListBuilder permits to build #DhBookList objects.
 */

/* API design:
 *
 * It follows the builder pattern, see:
 * https://blogs.gnome.org/otte/2018/02/03/builders/
 * but it is implemented in a simpler way, to have less boilerplate.
 */

typedef struct {
        /* List of DhBookList*. */
        GList *sub_book_lists;

        DhSettings *settings;
} DhBookListBuilderPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (DhBookListBuilder, dh_book_list_builder, G_TYPE_OBJECT)

static void
dh_book_list_builder_dispose (GObject *object)
{
        DhBookListBuilder *builder = DH_BOOK_LIST_BUILDER (object);
        DhBookListBuilderPrivate *priv = dh_book_list_builder_get_instance_private (builder);

        g_list_free_full (priv->sub_book_lists, g_object_unref);
        priv->sub_book_lists = NULL;

        g_clear_object (&priv->settings);

        G_OBJECT_CLASS (dh_book_list_builder_parent_class)->dispose (object);
}

static void
dh_book_list_builder_class_init (DhBookListBuilderClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);

        object_class->dispose = dh_book_list_builder_dispose;
}

static void
dh_book_list_builder_init (DhBookListBuilder *builder)
{
}

/**
 * dh_book_list_builder_new:
 *
 * Returns: (transfer full): a new #DhBookListBuilder.
 * Since: 3.30
 */
DhBookListBuilder *
dh_book_list_builder_new (void)
{
        return g_object_new (DH_TYPE_BOOK_LIST_BUILDER, NULL);
}

/**
 * dh_book_list_builder_add_sub_book_list:
 * @builder: a #DhBookListBuilder.
 * @sub_book_list: a #DhBookList.
 *
 * Adds @sub_book_list.
 *
 * The #DhBookList object that will be created with
 * dh_book_list_builder_create_object() will contain all the sub-#DhBookList's
 * added with this function (and it will listen to their signals). The
 * sub-#DhBookList's must be added in order of decreasing priority (the first
 * sub-#DhBookList added has the highest priority). The priority is used in case
 * of book ID conflicts (see dh_book_get_id()).
 *
 * Since: 3.30
 */
void
dh_book_list_builder_add_sub_book_list (DhBookListBuilder *builder,
                                       DhBookList        *sub_book_list)
{
        DhBookListBuilderPrivate *priv = dh_book_list_builder_get_instance_private (builder);

        g_return_if_fail (DH_IS_BOOK_LIST_BUILDER (builder));
        g_return_if_fail (DH_IS_BOOK_LIST (sub_book_list));

        priv->sub_book_lists = g_list_append (priv->sub_book_lists,
                                              g_object_ref (sub_book_list));
}

static void
add_book_list_directory (DhBookListBuilder *builder,
			 const gchar       *directory_path,
			 gint scale)
{

        DhBookListDirectory *sub_book_list = dh_book_list_get_default(scale);

        dh_book_list_builder_add_sub_book_list (builder, DH_BOOK_LIST (sub_book_list));
        g_object_unref (sub_book_list);
}

static void
add_default_sub_book_lists_in_data_dir (DhBookListBuilder *builder,
					const gchar       *data_dir, gint scale)
{
        gchar *dir;

        g_return_if_fail (data_dir != NULL);

        add_book_list_directory (builder, data_dir, scale);
}

/**
 * dh_book_list_builder_add_default_sub_book_lists:
 * @builder: a #DhBookListBuilder.
 *
 * Creates the default #DhBookListDirectory's and adds them to @builder with
 * dh_book_list_builder_add_sub_book_list().
 *
 * It creates and adds a #DhBookListDirectory for the following directories (in
 * that order):
 * - `$XDG_DATA_HOME/gtk-doc/html/`
 * - `$XDG_DATA_HOME/devhelp/books/`
 * - For each directory in `$XDG_DATA_DIRS`:
 *   - `$xdg_data_dir/gtk-doc/html/`
 *   - `$xdg_data_dir/devhelp/books/`
 *
 * See g_get_user_data_dir() and g_get_system_data_dirs().
 *
 * Additionally, if the libdevhelp has been compiled with the `flatpak_build`
 * option, it creates and adds a #DhBookListDirectory for the following
 * directories (in that order, after the above ones):
 * - `/run/host/usr/share/gtk-doc/html/`
 * - `/run/host/usr/share/devhelp/books/`
 *
 * The exact list of directories is subject to change, it is not part of the
 * API.
 *
 * Since: 3.30
 */
void
dh_book_list_builder_add_default_sub_book_lists (DhBookListBuilder *builder, gint scale)
{
        const gchar * const *system_dirs;
        gint i;

        g_return_if_fail (DH_IS_BOOK_LIST_BUILDER (builder));

        add_default_sub_book_lists_in_data_dir (builder, "", scale); // zealcore handles it
}

/**
 * dh_book_list_builder_read_books_disabled_setting:
 * @builder: a #DhBookListBuilder.
 * @settings: (nullable): a #DhSettings, or %NULL.
 *
 * Sets the #DhSettings object from which to read the "books-disabled"
 * #GSettings key. If @settings is %NULL or if this function isn't called, then
 * the #DhBookList object that will be created with
 * dh_book_list_builder_create_object() will not read a "books-disabled"
 * setting.
 *
 * With #DhBookListBuilder it is not possible to read the "books-disabled"
 * settings from several #DhSettings objects and combine them. Only the last
 * call to this function is taken into account when creating the #DhBookList
 * with dh_book_list_builder_create_object().
 *
 * Since: 3.30
 */
void
dh_book_list_builder_read_books_disabled_setting (DhBookListBuilder *builder,
                                                  DhSettings        *settings)
{
        DhBookListBuilderPrivate *priv = dh_book_list_builder_get_instance_private (builder);

        g_return_if_fail (DH_IS_BOOK_LIST_BUILDER (builder));
        g_return_if_fail (settings == NULL || DH_IS_SETTINGS (settings));

        g_set_object (&priv->settings, settings);
}

/**
 * dh_book_list_builder_create_object:
 * @builder: a #DhBookListBuilder.
 *
 * Creates the #DhBookList. It actually creates a subclass of #DhBookList, but
 * the subclass is not exposed to the public API.
 *
 * Returns: (transfer full): the newly created #DhBookList object.
 * Since: 3.30
 */
DhBookList *
dh_book_list_builder_create_object (DhBookListBuilder *builder)
{
        DhBookListBuilderPrivate *priv = dh_book_list_builder_get_instance_private (builder);

        g_return_val_if_fail (DH_IS_BOOK_LIST_BUILDER (builder), NULL);

        return _dh_book_list_simple_new (priv->sub_book_lists,
                                         priv->settings);
}
