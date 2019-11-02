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
#include <undo.h>

#define MODIFIER_MASK (GDK_CONTROL_MASK | GDK_SHIFT_MASK | GDK_MOD1_MASK | GDK_SUPER_MASK | GDK_HYPER_MASK | GDK_META_MASK)

static gint get_indent_level_and_iter(GtkTextBuffer *buffer, GtkTextIter *iter, gint line,
									  gboolean skip_empty_line, gboolean *was_empty_ptr) {
	gint level = 0;
	gboolean was_empty;
	gunichar ch;

	gtk_text_buffer_get_iter_at_line(buffer, iter, line);

	for (;;) {
		ch = gtk_text_iter_get_char(iter);

		if (ch != ' ' && ch != '\t')
			break;

		level++;
		gtk_text_iter_forward_char(iter);
	}

	was_empty = !ch || ch == '\r' || ch == '\n';

	if (was_empty_ptr)
		*was_empty_ptr = was_empty;

	if (was_empty && skip_empty_line)
		level = -1;

	return level;
}

static void insert_indent_line_from_template(GtkTextBuffer *buffer, GtkTextIter *iter,
											 gint template_line)
{
	GtkTextIter template_start, template_indent_end;
	gchar *indent;
	gchar ch;

	// get template indent first
	gtk_text_buffer_get_iter_at_line(buffer, &template_start, template_line);
	get_indent_level_and_iter(buffer, &template_indent_end, template_line, FALSE, NULL);
	indent = gtk_text_iter_get_text(&template_start, &template_indent_end);

	// iter points to the line before our new one
	// so move iter to the end of this line
	for (;;) {
		ch = gtk_text_iter_get_char(iter);
		if (!ch || ch == '\r' || ch == '\n')
			break;

		gtk_text_iter_forward_char(iter);
	}

	// and insert a new one
	undo_set_sequency(FALSE);
	g_signal_emit_by_name(G_OBJECT(buffer), "begin-user-action");

	gtk_text_buffer_insert(buffer, iter, "\n", 1);
	undo_set_sequency(TRUE);
	gtk_text_buffer_insert(buffer, iter, indent, -1);

	g_signal_emit_by_name(G_OBJECT(buffer), "end-user-action");
	undo_set_sequency(FALSE);

	g_free(indent);
}

static void navigate_indent_iter(GtkTextBuffer *buffer, GtkTextIter *iter, gint direction,
								 gboolean outer_level, gboolean may_insert)
{
	gint total_lines = gtk_text_buffer_get_line_count(buffer);
	gint base_line = gtk_text_iter_get_line(iter);
	gint base_level;
	GtkTextIter base_iter;
	gint level = -1;
	gint line = base_line;

	base_level = get_indent_level_and_iter(buffer, &base_iter, line, FALSE, NULL);

	for (;;) {
		line += direction;
		if (line < 0 || line >= total_lines)
			break;

		level = get_indent_level_and_iter(buffer, iter, line, TRUE, NULL);
		if (level == -1)
			continue;

		if (level < base_level || (!outer_level && level == base_level))
			break;
	}

	if (may_insert) {
		if (level < base_level) {
			gboolean was_empty;

			// now iterate backwards to find the last non-blank line before the current one
			// but if a blank line with proper indent level already exists, just move there
			for (;;) {
				line -= direction;
				level = get_indent_level_and_iter(buffer, iter, line, FALSE, &was_empty);

				if (line == base_line) {
					// if this non-blank line is our base line, discard this level
					// because we need to insert a new line below
					level = -1;
					break;
				}

				if (!was_empty || level == base_level)
					break;
			}
		}

		if (level != base_level)
			insert_indent_line_from_template(buffer, iter, base_line);
	}
}

void navigate_indent(GtkTextView *view, GtkTextIter *iter_param, gint direction,
					 gboolean outer_level, gboolean may_insert)
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
	navigate_indent_iter(buff, &iter, direction, outer_level, may_insert);

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
	gboolean outer_level = FALSE;
	gint direction = 1;

	switch(event->state & MODIFIER_MASK) {
	case GDK_CONTROL_MASK:
	case GDK_CONTROL_MASK | GDK_MOD1_MASK:
		break;
	case GDK_SHIFT_MASK | GDK_MOD1_MASK:
		outer_level = TRUE;
		break;
	default:
		return FALSE;
	}

	if (event->direction == GDK_SCROLL_UP)
		direction = -1;
	else if (event->direction != GDK_SCROLL_DOWN)
		return FALSE;

	navigate_indent(view, NULL, direction, outer_level, FALSE);
	return TRUE;
}

gboolean indent_navigation_handle_key_press(GtkTextView *view, GdkEventKey *event) {
	gboolean outer_level = FALSE;
	gboolean may_insert = FALSE;
	gint direction = 1;

	switch(event->state & MODIFIER_MASK) {
	case GDK_CONTROL_MASK | GDK_MOD1_MASK:
		break;
	case GDK_SHIFT_MASK | GDK_MOD1_MASK:
		outer_level = TRUE;
		break;
	default:
		return FALSE;
	}

	if (event->keyval == GDK_Up)
		direction = -1;
	else if (event->keyval == GDK_Right || event->keyval == GDK_Insert) {
		may_insert = TRUE;
		outer_level = FALSE;
	}
	else if (event->keyval != GDK_Down)
		return FALSE;

	navigate_indent(view, NULL, direction, outer_level, may_insert);
	return TRUE;
}

gboolean indent_navigation_handle_button_press(GtkTextView *view, GdkEventButton *event)
{
	gboolean outer_level = FALSE;
	gboolean may_insert = FALSE;
	gint direction = 1;
	gint buff_y;
	GtkTextIter iter;

	switch(event->state & MODIFIER_MASK) {
	case GDK_CONTROL_MASK | GDK_MOD1_MASK:
		break;
	case GDK_SHIFT_MASK | GDK_MOD1_MASK:
		outer_level = TRUE;
		break;
	default:
		return FALSE;
	}

	if (event->button == 1)
		direction = -1;
	else if (!outer_level && event->button == 2)
		may_insert = TRUE;
	else if (event->button != 3)
		return FALSE;

	gtk_text_view_window_to_buffer_coords(view, GTK_TEXT_WINDOW_WIDGET, event->x, event->y, NULL, &buff_y);
	gtk_text_view_get_line_at_y(view, &iter, buff_y, NULL);

	navigate_indent(view, &iter, direction, outer_level, may_insert);
	return TRUE;
}

