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

#ifndef GTKHASH_NAUTILUS_PROPERTIES_H
#define GTKHASH_NAUTILUS_PROPERTIES_H

#include <gtk/gtk.h>

#include "../hash/hash-file.h"

#if ENABLE_NLS
	#include <glib/gi18n-lib.h>
#else
	#define _(X) (X)
#endif

struct page_s {
	char *uri;
	GtkWidget *box;
	GtkProgressBar *progressbar;
	GtkTreeView *treeview;
	GtkCellRendererToggle *cellrendtoggle;
	GtkMenu *menu;
	GtkImageMenuItem *menuitem_copy;
	GtkCheckMenuItem *menuitem_show_funcs;
	GtkButton *button_hash, *button_stop;
	struct hash_func_s funcs[HASH_FUNCS_N];
	struct hash_file_s hash_file;
};

void gtkhash_properties_idle(struct page_s *page);

#endif