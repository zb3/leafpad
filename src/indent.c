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
//#include <gdk/gdkkeysyms.h>
#include <undo.h>

static gboolean auto_indent = FALSE;
static gint default_tab_width = 8;
static gint current_tab_width = 8;

gint get_current_tab_width(void)
{
	return current_tab_width;
}

void indent_set_state(gboolean state)
{
	auto_indent = state;
}

gboolean indent_get_state(void)
{
	return auto_indent;
}

static gchar *compute_indentation(GtkTextBuffer *buffer, GtkTextIter *iter, gint line)
{
	GtkTextIter start_iter, end_iter;
	gunichar ch;

	gtk_text_buffer_get_iter_at_line(buffer, &start_iter, line);
	end_iter = start_iter;
	ch = gtk_text_iter_get_char(&end_iter);
	while (g_unichar_isspace(ch) && ch != '\n') {
		if (!gtk_text_iter_forward_char(&end_iter))
			break;
		ch = gtk_text_iter_get_char(&end_iter);
	}
	if (gtk_text_iter_equal(&start_iter, &end_iter))
		return NULL;

	if (iter && gtk_text_iter_compare(iter, &end_iter) < 0)
		return gtk_text_iter_get_text(&start_iter, iter);
	return gtk_text_iter_get_text(&start_iter, &end_iter);
}

static gunichar first_char_in_line(GtkTextBuffer *buffer, gint line)
{
	GtkTextIter iter;
	gunichar ch;

	gtk_text_buffer_get_iter_at_line(buffer, &iter, line);
	ch = gtk_text_iter_get_char(&iter);

	return ch;
}

void indent_real(GtkWidget *text_view)
{
	GtkTextIter iter;
	gchar *ind, *str;

	GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));

	g_signal_emit_by_name(G_OBJECT(buffer), "begin-user-action");

	// interactive must be FALSE here, otherwise it 'll emit the end-user-action signal
	gtk_text_buffer_delete_selection(buffer, FALSE, TRUE);

	gtk_text_buffer_get_iter_at_mark(buffer, &iter, gtk_text_buffer_get_insert(buffer));
	ind = compute_indentation(buffer, &iter, gtk_text_iter_get_line(&iter));
	str = g_strconcat("\n", ind, NULL);
	gtk_text_buffer_insert(buffer, &iter, str, -1);
	g_signal_emit_by_name(G_OBJECT(buffer), "end-user-action");
	g_free(str);
	g_free(ind);

	gtk_text_view_scroll_mark_onscreen(
		GTK_TEXT_VIEW(text_view),
		gtk_text_buffer_get_insert(buffer));
}

static gint calculate_real_tab_width(GtkWidget *text_view, guint tab_size) //from gtksourceview
{
	PangoLayout *layout;
	gchar *tab_string;
	gint tab_width = 0;

	if (tab_size == 0)
		return -1;

	tab_string = g_strnfill(tab_size, 0x20);
	layout = gtk_widget_create_pango_layout(text_view, tab_string);
	g_free (tab_string);

	if (layout != NULL) {
		pango_layout_get_pixel_size(layout, &tab_width, NULL);
		g_object_unref(G_OBJECT(layout));
	} else
		tab_width = -1;

	return tab_width;
}

void indent_refresh_tab_width(GtkWidget *text_view)
{
	PangoTabArray *tab_array;

	tab_array = pango_tab_array_new(1, TRUE);
	pango_tab_array_set_tab(tab_array, 0, PANGO_TAB_LEFT,
		calculate_real_tab_width(text_view, current_tab_width));
	gtk_text_view_set_tabs(GTK_TEXT_VIEW(text_view), tab_array);
	pango_tab_array_free(tab_array);
}

void indent_toggle_tab_width(GtkWidget *text_view)
{
	if (current_tab_width == default_tab_width)
		if (default_tab_width == 8)
			current_tab_width = 4;
		else
			current_tab_width = 8;
	else
		current_tab_width = default_tab_width;
	indent_refresh_tab_width(text_view);
}

void indent_set_default_tab_width(gint width)
{
	default_tab_width = width;
	current_tab_width = default_tab_width;
}

void indent_multi_line_indent(GtkTextBuffer *buffer)
{
	GtkTextIter start_iter, end_iter, iter;
	gint start_line, end_line, i;
	gboolean selection_rtl;

	gtk_text_buffer_get_selection_bounds(buffer, &start_iter, &end_iter);
	start_line = gtk_text_iter_get_line(&start_iter);

	end_line = gtk_text_iter_get_line(&end_iter);
	gint end_offset = gtk_text_iter_get_line_offset(&end_iter);

	if (end_offset) end_line++;

	gtk_text_buffer_get_iter_at_mark(buffer, &iter, gtk_text_buffer_get_insert(buffer));
	selection_rtl = gtk_text_iter_equal(&iter, &start_iter);


	gchar *indent = first_char_in_line(buffer, start_line) == '\t' ? "\t" : " ";

	undo_set_sequency(FALSE);
	for (i = start_line; i < end_line; i++) {
		gtk_text_buffer_get_iter_at_line(buffer, &iter, i);
		g_signal_emit_by_name(G_OBJECT(buffer), "begin-user-action");
		gtk_text_buffer_insert(buffer, &iter, indent, 1);
		g_signal_emit_by_name(G_OBJECT(buffer), "end-user-action");
		undo_set_sequency(TRUE);
	}
	undo_set_sequency(FALSE);


	gtk_text_buffer_get_iter_at_line(buffer, &start_iter, start_line);

	gint line_count = gtk_text_buffer_get_line_count(buffer);

	if (end_line < line_count) {
		gtk_text_buffer_get_iter_at_line(buffer, &end_iter, end_line);
	} else {
		gtk_text_buffer_get_end_iter(buffer, &end_iter);
	}

	if (selection_rtl) {
		gtk_text_buffer_place_cursor(buffer, &end_iter);
		gtk_text_buffer_move_mark_by_name(buffer, "insert", &start_iter);
	} else {
		gtk_text_buffer_place_cursor(buffer, &start_iter);
		gtk_text_buffer_move_mark_by_name(buffer, "insert", &end_iter);
	}
}


void indent_multi_line_unindent(GtkTextBuffer *buffer)
{
	GtkTextIter start_iter, end_iter, iter;
	gint start_line, end_line, i;
	gboolean selection_rtl;

	gtk_text_buffer_get_selection_bounds(buffer, &start_iter, &end_iter);
	start_line = gtk_text_iter_get_line(&start_iter);

	end_line = gtk_text_iter_get_line(&end_iter);
	gint end_offset = gtk_text_iter_get_line_offset(&end_iter);

	if (end_offset) end_line++;

	gtk_text_buffer_get_iter_at_mark(buffer, &iter, gtk_text_buffer_get_insert(buffer));

	selection_rtl = gtk_text_iter_equal(&iter, &start_iter);


	undo_set_sequency(FALSE);
	for (i = start_line; i < end_line; i++) {
		gunichar first_char = first_char_in_line(buffer, i);
		gchar *indent = first_char == ' ' ? " " : first_char == '\t' ? "\t" : NULL;

		if (!indent) continue;

		gtk_text_buffer_get_iter_at_line(buffer, &start_iter, i);

		end_iter = start_iter;
		gtk_text_iter_forward_char(&end_iter);

		g_signal_emit_by_name(G_OBJECT(buffer), "begin-user-action");
		gtk_text_buffer_delete(buffer, &start_iter, &end_iter);
		g_signal_emit_by_name(G_OBJECT(buffer), "end-user-action");
		undo_set_sequency(TRUE);
	}
	undo_set_sequency(FALSE);


	gtk_text_buffer_get_iter_at_line(buffer, &start_iter, start_line);

	gint line_count = gtk_text_buffer_get_line_count(buffer);

	if (end_line < line_count) {
		gtk_text_buffer_get_iter_at_line(buffer, &end_iter, end_line);
	} else {
		gtk_text_buffer_get_end_iter(buffer, &end_iter);
	}

	if (selection_rtl) {
		gtk_text_buffer_place_cursor(buffer, &end_iter);
		gtk_text_buffer_move_mark_by_name(buffer, "insert", &start_iter);
	} else {
		gtk_text_buffer_place_cursor(buffer, &start_iter);
		gtk_text_buffer_move_mark_by_name(buffer, "insert", &end_iter);
	}
}

typedef struct {
	GtkTextView *view;
	GtkTextBuffer *buffer;

} IndentPasteData;

static gint compute_text_indent_and_lines(const gchar *text, gint *line_count_ptr)
{
	gint min_indent = -1, cur_indent = 0;
	gint line_count = 1;
	gchar ch;

	while((ch = *text)) {
		if (ch == '\r' || ch == '\n') {
			if (ch == '\r' && *(text+1) == '\n')
				text++;

			cur_indent = 0;
			line_count++;
		} else if (cur_indent >=0 && (ch == ' ' || ch == '\t'))
			cur_indent++;
		else if (cur_indent != -1) {
			if (min_indent == -1 || cur_indent < min_indent)
				min_indent = cur_indent;

			cur_indent = -1;
		}

		text++;
	}

	if (line_count_ptr)
		*line_count_ptr = line_count;

	return min_indent == -1 ? 0 : min_indent;
}

static void do_indent_paste(GtkTextView *view, GtkTextBuffer *buffer, const gchar *text)
{
	GtkTextIter iter, start, end;
	const gchar *text_ptr = text;
	const gchar *text_line_start = text;
	gchar *new_text, *new_text_ptr;
	gint base_indent;
	gint insert_len;
	gboolean first_line_inserted = FALSE;
	gint line_count;
	gchar *indent;
	gint indent_length = 0;
	gchar ch;

	// all lengths here are in bytes, but text is assumed to be utf8 encoded

	undo_set_sequency(FALSE);
	g_signal_emit_by_name(G_OBJECT(buffer), "begin-user-action");

	if (gtk_text_buffer_get_selection_bounds(buffer, &start, &end)) {
		gtk_text_buffer_delete(buffer, &start, &end);
		undo_set_sequency(TRUE);
	}

	gtk_text_buffer_get_iter_at_mark(buffer, &iter, gtk_text_buffer_get_insert(buffer));
	indent = compute_indentation(buffer, &iter, gtk_text_iter_get_line(&iter));

	if (indent)
		indent_length = strlen(indent);

	base_indent = compute_text_indent_and_lines(text, &line_count);

	new_text = g_malloc(strlen(text) + line_count*indent_length);
	new_text_ptr = new_text;

	for(;;) {
		ch = *text_ptr;

		if (!ch || ch == '\r' || ch == '\n') {
			if (ch == '\r' && *(text_ptr+1) == '\n')
				text_ptr++;

			// from text start to textptr might contain nl
			// we want it to do so
			if (indent) {
				if (first_line_inserted) {
					memcpy(new_text_ptr, indent, indent_length);
					new_text_ptr += indent_length;
				} else
					first_line_inserted = TRUE;
			}

			// base length (without newline)
			insert_len = text_ptr - text_line_start;

			// now we want to subtract base indent, but only if present
			// blank lines won't have it
			if (base_indent && insert_len >= base_indent) {
				text_line_start += base_indent;
				insert_len -= base_indent;
			}

			// we want to include ch if it's a newline char
			// but not when it's a null byte
			if (ch)
				insert_len++;

			memcpy(new_text_ptr, text_line_start, insert_len);
			new_text_ptr += insert_len;
			text_line_start = text_ptr+1;
		}

		if (!ch)
			break;

		text_ptr++;
	}

	gtk_text_buffer_insert(buffer, &iter, new_text, new_text_ptr-new_text);
	g_signal_emit_by_name(G_OBJECT(buffer), "end-user-action");
	undo_set_sequency(FALSE);

	gtk_text_view_scroll_mark_onscreen(view, gtk_text_buffer_get_insert(buffer));
	g_free(new_text);
	g_free(indent);
}

static void indent_paste_text_received(GtkClipboard *clipboard, const gchar *text,
									   gpointer data)
{
	IndentPasteData *paste_data = data;

	if (text)
		do_indent_paste(paste_data->view, paste_data->buffer, text);

	g_object_unref(paste_data->buffer);
	g_object_unref(paste_data->view);
	g_slice_free(IndentPasteData, paste_data);
}

void indent_paste(GtkTextView *view, gboolean primary)
{
	GtkClipboard *clipboard = gtk_widget_get_clipboard(GTK_WIDGET(view), primary ?
										GDK_SELECTION_PRIMARY : GDK_SELECTION_CLIPBOARD);
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(view);
	IndentPasteData *data = g_slice_new(IndentPasteData);

	data->buffer = g_object_ref(buffer);
	data->view = g_object_ref(view);

	gtk_clipboard_request_text(clipboard, indent_paste_text_received, data);
}

