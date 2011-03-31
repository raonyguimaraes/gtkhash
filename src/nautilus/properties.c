/*
 *   Copyright (C) 2007-2010 Tristan Heaven <tristanheaven@gmail.com>
 *
 *   This file is part of GtkHash.
 *
 *   GtkHash is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   GtkHash is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with GtkHash. If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
	#include "config.h"
#endif

#include <stdlib.h>
#include <stdbool.h>
#include <gio/gio.h>
#include <gtk/gtk.h>
#include <libnautilus-extension/nautilus-property-page.h>
#include <libnautilus-extension/nautilus-property-page-provider.h>

#include "properties.h"
#include "properties-list.h"
#include "properties-hash.h"
#include "properties-prefs.h"
#include "../hash/hash-func.h"
#include "../hash/hash-file.h"

static GType page_type;

static GObject *gtkhash_properties_get_object(GtkBuilder *builder,
	const char *name)
{
	GObject *obj = gtk_builder_get_object(builder, name);
	if (!obj)
		g_warning("unknown object: \"%s\"", name);

	return obj;
}

static void gtkhash_properties_busy(struct page_s *page)
{
	gtk_widget_set_sensitive(GTK_WIDGET(page->button_hash), false);
	gtk_widget_set_sensitive(GTK_WIDGET(page->button_stop), true);
	gtk_widget_set_sensitive(GTK_WIDGET(page->treeview), false);

	// Reset progress bar
	gtk_progress_bar_set_fraction(page->progressbar, 0.0);
	gtk_progress_bar_set_text(page->progressbar, " ");
	gtk_widget_show(GTK_WIDGET(page->progressbar));
}

void gtkhash_properties_idle(struct page_s *page)
{
	gtk_widget_hide(GTK_WIDGET(page->progressbar));

	gtk_widget_set_sensitive(GTK_WIDGET(page->treeview), true);
	gtk_widget_set_sensitive(GTK_WIDGET(page->button_stop), false);
	gtk_widget_set_sensitive(GTK_WIDGET(page->button_hash), true);
}

static void gtkhash_properties_on_cell_toggled(struct page_s *page, char *path)
{
	int id = gtkhash_properties_list_get_row_id(page, path);
	page->hash_file.funcs[id].enabled = !page->hash_file.funcs[id].enabled;

	gtkhash_properties_list_update_enabled(page, id);
}

static void gtkhash_properties_on_treeview_popup_menu(struct page_s *page)
{
	gtk_menu_popup(page->menu, NULL, NULL, NULL, NULL, 0,
		gtk_get_current_event_time());
}

static bool gtkhash_properties_on_treeview_button_press(struct page_s *page,
	GdkEventButton *event)
{
	// Right click
	if (event->type == GDK_BUTTON_PRESS && event->button == 3) {
		gtk_menu_popup(page->menu, NULL, NULL, NULL, NULL, 3,
			gdk_event_get_time((GdkEvent *)event));
	}

	return false;
}

static void gtkhash_properties_on_treeview_cursor_changed(struct page_s *page)
{
	bool sensitive = false;
	char *digest = gtkhash_properties_list_get_selected_digest(page);
	if (digest) {
		sensitive = true;
		g_free(digest);
	}

	gtk_widget_set_sensitive(GTK_WIDGET(page->menuitem_copy), sensitive);
}

static void gtkhash_properties_on_menuitem_copy_activate(struct page_s *page)
{
	GtkClipboard *clipboard = gtk_clipboard_get(GDK_NONE);
	char *digest = gtkhash_properties_list_get_selected_digest(page);

	gtk_clipboard_set_text(clipboard, digest, -1);

	g_free(digest);
}

static void gtkhash_properties_on_menuitem_show_funcs_toggled(
	struct page_s *page)
{
	gtkhash_properties_list_refilter(page);
}

static void gtkhash_properties_on_button_hash_clicked(struct page_s *page)
{
	gtkhash_properties_busy(page);
	gtkhash_hash_file_clear_digests(&page->hash_file);
	gtkhash_properties_list_update_digests(page);

	gtkhash_properties_hash_start(page);
}

static void gtkhash_properties_on_button_stop_clicked(struct page_s *page)
{
	gtk_widget_set_sensitive(GTK_WIDGET(page->button_stop), false);
	gtkhash_properties_hash_stop(page);
}

static void gtkhash_properties_free_page(struct page_s *page)
{
	gtkhash_properties_hash_stop(page);
	gtkhash_properties_prefs_save(page);
	gtkhash_properties_hash_deinit(page);
	g_free(page->uri);
	g_free(page);
}

static void gtkhash_properties_get_objects(struct page_s *page,
	GtkBuilder *builder)
{
	// Main container
	page->box = GTK_WIDGET(gtkhash_properties_get_object(builder, "vbox"));

	// Progress bar
	page->progressbar = GTK_PROGRESS_BAR(gtkhash_properties_get_object(builder,
		"progressbar"));

	// Treeview
	page->treeview = GTK_TREE_VIEW(gtkhash_properties_get_object(builder,
		"treeview"));
	page->cellrendtoggle = GTK_CELL_RENDERER_TOGGLE(gtkhash_properties_get_object(builder,
		"cellrenderertoggle"));

	// Popup menu
	page->menu = GTK_MENU(gtkhash_properties_get_object(builder,
		"menu"));
	page->menuitem_copy = GTK_IMAGE_MENU_ITEM(gtkhash_properties_get_object(builder,
		"imagemenuitem_copy"));
	page->menuitem_show_funcs = GTK_CHECK_MENU_ITEM(gtkhash_properties_get_object(builder,
		"checkmenuitem_show_funcs"));

	// Buttons
	page->button_hash = GTK_BUTTON(gtkhash_properties_get_object(builder,
		"button_hash"));
	page->button_stop = GTK_BUTTON(gtkhash_properties_get_object(builder,
		"button_stop"));
}

static void gtkhash_properties_connect_signals(struct page_s *page)
{
	// Main container
	g_signal_connect_swapped(page->box, "destroy",
		G_CALLBACK(gtkhash_properties_free_page), page);

	// Treeview
	g_signal_connect_swapped(page->cellrendtoggle, "toggled",
		G_CALLBACK(gtkhash_properties_on_cell_toggled), page);
	g_signal_connect_swapped(page->treeview, "popup-menu",
		G_CALLBACK(gtkhash_properties_on_treeview_popup_menu), page);
	g_signal_connect_swapped(page->treeview, "button-press-event",
		G_CALLBACK(gtkhash_properties_on_treeview_button_press), page);
	g_signal_connect_swapped(page->treeview, "cursor-changed",
		G_CALLBACK(gtkhash_properties_on_treeview_cursor_changed), page);

	// Popup menu
	g_signal_connect_swapped(page->menuitem_copy, "activate",
		G_CALLBACK(gtkhash_properties_on_menuitem_copy_activate), page);
	g_signal_connect_swapped(page->menuitem_show_funcs, "toggled",
		G_CALLBACK(gtkhash_properties_on_menuitem_show_funcs_toggled), page);

	// Buttons
	g_signal_connect_swapped(page->button_hash, "clicked",
		G_CALLBACK(gtkhash_properties_on_button_hash_clicked), page);
	g_signal_connect_swapped(page->button_stop, "clicked",
		G_CALLBACK(gtkhash_properties_on_button_stop_clicked), page);
}

static struct page_s *gtkhash_properties_new_page(char *uri)
{
	GtkBuilder *builder = gtk_builder_new();

#if ENABLE_NLS
	gtk_builder_set_translation_domain(builder, PACKAGE);
#endif

	if (!gtk_builder_add_from_file(builder, BUILDER_XML, NULL)) {
		g_warning("failed to read %s", BUILDER_XML);
		return NULL;
	}

	struct page_s *page = g_new(struct page_s, 1);

	page->uri = uri;

	gtkhash_properties_get_objects(page, builder);
	g_object_ref(page->box);
	g_object_ref(page->menu);
	g_object_unref(builder);

	gtkhash_properties_hash_init(page, uri);
	gtkhash_properties_prefs_load(page);
	gtkhash_properties_list_init(page);
	gtkhash_properties_idle(page);

	gtkhash_properties_connect_signals(page);

	return page;
}

static GList *gtkhash_properties_get_pages(
	G_GNUC_UNUSED NautilusPropertyPageProvider *provider, GList *files)
{
	// Only display page for a single file
	if (!files || files->next)
		return NULL;

	// Only display page for regular files
	if (nautilus_file_info_get_file_type(files->data) != G_FILE_TYPE_REGULAR)
		return NULL;

	char *uri = nautilus_file_info_get_uri(files->data);
	struct page_s *page = gtkhash_properties_new_page(uri);
	if (!page)
		return NULL;

	NautilusPropertyPage *ppage = nautilus_property_page_new(
		"GtkHash::properties", gtk_label_new(_("Digests")), page->box);

	GList *pages = g_list_append(NULL, ppage);

	return pages;
}

static void gtkhash_properties_iface_init(NautilusPropertyPageProviderIface *iface)
{
	iface->get_pages = gtkhash_properties_get_pages;
}

static void gtkhash_properties_register_type(GTypeModule *module)
{
	const GTypeInfo info = {
		sizeof(GObjectClass),
		(GBaseInitFunc)NULL,
		(GBaseFinalizeFunc)NULL,
		(GClassInitFunc)NULL,
		(GClassFinalizeFunc)NULL,
		NULL,
		sizeof(GObject),
		0,
		(GInstanceInitFunc)NULL,
		(GTypeValueTable *)NULL
	};

	page_type = g_type_module_register_type(module, G_TYPE_OBJECT,
		"GtkHash", &info, 0);

	const GInterfaceInfo iface_info = {
		(GInterfaceInitFunc)gtkhash_properties_iface_init,
		(GInterfaceFinalizeFunc)NULL,
		NULL
	};

	g_type_module_add_interface(module, page_type,
		NAUTILUS_TYPE_PROPERTY_PAGE_PROVIDER, &iface_info);
}

void nautilus_module_initialize(GTypeModule *module)
{
	gtkhash_properties_register_type(module);

#if ENABLE_NLS
	bindtextdomain(PACKAGE, LOCALEDIR);
	bind_textdomain_codeset(PACKAGE, "UTF-8");
#endif
}

void nautilus_module_shutdown(void)
{
}

void nautilus_module_list_types(const GType **types, int *num_types)
{
	static GType type_list[1];

	type_list[0] = page_type;
	*types = type_list;
	*num_types = G_N_ELEMENTS(type_list);
}