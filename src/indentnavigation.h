/*
 *  Leafpad - GTK+ based simple text editor
 *  Copyright (C) 2004-2005 Tarot Osuji
 *  
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _INDENT_NAVIGATION_H
#define _INDENT_NAVIGATION_H

void navigate_indent(GtkTextView *view, gint direction, gboolean outer_level, GtkTextIter *iter_param);
gboolean indent_navigation_handle_scroll(GtkTextView *view, GdkEventButton *event);
gboolean indent_navigation_handle_arrow_key_press(GtkTextView *view, GdkEventKey *event);
gboolean indent_navigation_handle_button_press(GtkTextView *view, GdkEventButton *event);

#endif /* __INDENT_NAVIGATION_H */
