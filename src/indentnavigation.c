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

#include <string.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#define MODIFIER_MASK (GDK_CONTROL_MASK | GDK_SHIFT_MASK | GDK_MOD1_MASK | GDK_SUPER_MASK | GDK_HYPER_MASK | GDK_META_MASK)

static gint get_indent_level_and_iter(GtkTextBuffer *buffer, GtkTextIter *iter, gint line, gboolean skip_empty_line) {
	gint level = 0;
	gunichar ch;
	
	gtk_text_buffer_get_iter_at_line(buffer, iter, line);
	
	for (;;) {
		ch = gtk_text_iter_get_char(iter);
		
		if (ch != ' ' && ch != '\t')
			break;
				  
		level++;
		gtk_text_iter_forward_char(iter);
	}
	
	if (skip_empty_line && (!ch || ch == '\n'))
		level--;
	 
	return level;
}

static void navigate_indent_iter(GtkTextBuffer *buffer, GtkTextIter *iter, gint direction, gboolean outer_level)
{
	gint line = gtk_text_iter_get_line(iter);
	gint base_level = get_indent_level_and_iter(buffer, iter, line, FALSE);
	gint total_lines = gtk_text_buffer_get_line_count(buffer);
	
	gint level;
	
	for (;;) {
		line += direction;
		if (line < 0 || line >= total_lines)
			break;
		
		level = get_indent_level_and_iter(buffer, iter, line, TRUE);
		if (level == -1)
			continue;
				 
		if (level < base_level || (!outer_level && level == base_level))
			break;
	}
}

void navigate_indent(GtkTextView *view, gint direction, gboolean outer_level, GtkTextIter *iter_param)
{
	GtkTextBuffer *buff = gtk_text_view_get_buffer(view);
	GtkTextIter iter;
	GtkAdjustment *vadj;
	gint orig_y;
	gint new_y;
	
	if (!iter_param) {
		gtk_text_buffer_get_iter_at_mark(buff, &iter, gtk_text_buffer_get_insert(buff));
	} else {
		iter = *iter_param;
	}

	gtk_text_view_get_line_yrange(view, &iter, &orig_y, NULL);

	navigate_indent_iter(buff, &iter, direction, outer_level);
	
	if (gtk_text_buffer_get_has_selection(buff))
		gtk_text_buffer_move_mark_by_name(buff, "insert", &iter);
	else
		gtk_text_buffer_place_cursor(buff, &iter);
		
	gtk_text_view_get_line_yrange(view, &iter, &new_y, NULL);

	vadj = gtk_text_view_get_vadjustment(view);
	gtk_adjustment_set_value(vadj, gtk_adjustment_get_value(vadj) + new_y - orig_y);
}

gboolean indent_navigation_handle_scroll(GtkTextView *view, GdkEventScroll *event)
{
	gboolean outer_level;
	gint direction;
	
	switch(event->state & MODIFIER_MASK) {
	case GDK_CONTROL_MASK:
	case GDK_CONTROL_MASK | GDK_MOD1_MASK:
		outer_level = FALSE;
		break;
	case GDK_SHIFT_MASK | GDK_MOD1_MASK:
		outer_level = TRUE;
		break;
	default:
		return FALSE;
	}
	
	if (event->direction == GDK_SCROLL_UP) {
		direction = -1;
	} else if (event->direction == GDK_SCROLL_DOWN) {
		direction = 1;
	} else {
		return FALSE;
	}

	navigate_indent(view, direction, outer_level, NULL);
	return TRUE;
}

gboolean indent_navigation_handle_arrow_key_press(GtkTextView *view, GdkEventKey *event) {
	switch(event->state & MODIFIER_MASK) {
	case GDK_CONTROL_MASK | GDK_MOD1_MASK:
	case GDK_SHIFT_MASK | GDK_MOD1_MASK:
		navigate_indent(view, event->keyval == GDK_Down ? 1 : -1,
			event->state & GDK_SHIFT_MASK, NULL);
		return TRUE;
	}
	
	return FALSE;
}

gboolean indent_navigation_handle_button_press(GtkTextView *view, GdkEventButton *event)
{
	gboolean outer_level;
	gint direction;
	gint buff_y;
	GtkTextIter iter;
	
	switch(event->state & MODIFIER_MASK) {
	case GDK_CONTROL_MASK:
	case GDK_CONTROL_MASK | GDK_MOD1_MASK:
		outer_level = FALSE;
		break;
	case GDK_SHIFT_MASK | GDK_MOD1_MASK:
		outer_level = TRUE;
		break;
	default:
		return FALSE;
	}

	if (event->button == 1)
		direction = -1;
	else if (event->button == 3)
		direction = 1;
	else
		return FALSE;
		
		
	gtk_text_view_window_to_buffer_coords(view, GTK_TEXT_WINDOW_WIDGET, event->x, event->y, NULL, &buff_y);
	gtk_text_view_get_line_at_y(view, &iter, buff_y, NULL);
		
	navigate_indent(view, direction, outer_level, &iter);
	return TRUE;
}

