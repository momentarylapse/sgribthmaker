/*
 * SourceView.cpp
 *
 *  Created on: 06.06.2013
 *      Author: michi
 */

#include "SourceView.h"
#include "LineNumberView.h"
#include "History.h"
#include "Document.h"
#include "lib/hui/Controls/Control.h"
#include "lib/hui/config.h"
#include "parser/BaseParser.h"




#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"


void on_gtk_insert_text(GtkTextBuffer *textbuffer, GtkTextIter *location, gchar *text, gint len, gpointer user_data) {
	SourceView *sv = (SourceView*)user_data;
	if (!sv->history->enabled)
		return;
	/*if (strcmp(text, "\t") == 0) {
		g_signal_stop_emission_by_name(textbuffer, "insert-text");
		printf("no TAB\n");
		return;
	}*/

	if ((strcmp(text, "\n") == 0) and (sv->change_return)) {
		g_signal_stop_emission_by_name(textbuffer, "insert-text");
		sv->change_return = false;
		sv->insert_return();
		sv->change_return = true;
		return;
	}

	char *text2 = (char*)g_malloc(len + 1);
	memcpy(text2, text, len);
	sv->history->execute(new CommandInsert(text2, len, gtk_text_iter_get_offset(location)));
//	SetWindowTitle(); TODO

	sv->needs_update_start = gtk_text_iter_get_line(location);
	sv->needs_update_end = sv->needs_update_start;
	for (int i=0;i<len;i++)
		if (text[i] == '\n')
			sv->needs_update_end ++;
}

void on_gtk_delete_range(GtkTextBuffer *textbuffer, GtkTextIter *start, GtkTextIter *end, gpointer user_data) {
	SourceView *sv = (SourceView*)user_data;
	if (!sv->history->enabled)
		return;
	char *text = gtk_text_buffer_get_text(textbuffer, start, end, false);
	sv->history->execute(new CommandDelete(text, strlen(text), gtk_text_iter_get_offset(start)));
	//SetWindowTitle();

	sv->needs_update_start = gtk_text_iter_get_line(start);
	sv->needs_update_end = sv->needs_update_start;
}

void on_gtk_move_cursor(GtkTextView *text_view, GtkMovementStep step, gint count, gboolean extend_selection, gpointer user_data) {
	SourceView *sv = (SourceView*)user_data;
	if (step == GTK_MOVEMENT_DISPLAY_LINE_ENDS) {
		g_signal_stop_emission_by_name(text_view, "move-cursor");
		if (count > 0)
			sv->jump_to_end_of_line(extend_selection);
		else
			sv->jump_to_start_of_line(extend_selection);
	}
	//printf("move cursor  %d  %d  %d\n", count, (int)extend_selection, (int)step);
}

void on_gtk_copy_clipboard(GtkTextView *text_view, gpointer user_data) {
	g_signal_stop_emission_by_name(text_view, "copy-clipboard");
}

void on_gtk_paste_clipboard(GtkTextView *text_view, gpointer user_data) {
	g_signal_stop_emission_by_name(text_view, "paste-clipboard");
}

void on_gtk_cut_clipboard(GtkTextView *text_view, gpointer user_data) {
	g_signal_stop_emission_by_name(text_view, "cut-clipboard");
}

void on_gtk_toggle_cursor_visible(GtkTextView *text_view, gpointer user_data) {
	g_signal_stop_emission_by_name(text_view, "toggle-cursor-visible");
}

gboolean CallbackJumpLine(GtkWidget *widget, gpointer user_data) {
	auto *j = (SourceView::JumpData*)user_data;
	j->sv->show_line_on_screen(j->line);
	return FALSE;
}

#if !GTK_CHECK_VERSION(4,0,0)
void on_gtk_populate_popup(GtkTextView *text_view, GtkMenu *menu, gpointer user_data) {
	SourceView *sv = (SourceView*)user_data;
	auto labels = sv->parser->FindLabels(sv);
	GtkWidget *m = gtk_separator_menu_item_new();
	gtk_menu_shell_prepend(GTK_MENU_SHELL(menu), m);
	gtk_widget_show(m);
	if (labels.num == 0) {
		m = gtk_menu_item_new_with_label(_("- no labels -").c_str());
		gtk_menu_shell_prepend(GTK_MENU_SHELL(menu), m);
		gtk_widget_set_sensitive(m, false);
		gtk_widget_show(m);
	}
	sv->jump_data.clear();
	for (auto &l: labels)
		sv->jump_data.add(SourceView::JumpData(sv, l.line));
	foreachib(auto &l, labels, i) {
		if (l.level > 0)
			m = gtk_menu_item_new_with_label(l.name.c_str());
		else
			m = gtk_menu_item_new_with_label((">\t" + l.name).c_str());
		gtk_menu_shell_prepend(GTK_MENU_SHELL(menu), m);
		gtk_widget_show(m);
		g_signal_connect(G_OBJECT(m), "activate", G_CALLBACK(CallbackJumpLine), (void*)&sv->jump_data[i]);
	}
}
#endif

void SourceView::create_text_colors(int first_line, int last_line) {
	parser->CreateTextColors(this, first_line, last_line);
}

void SourceView::create_colors_if_not_busy() {
	color_busy_level --;
	if (color_busy_level == 0) {
		parser->update_symbols(this);
		create_text_colors();
	}
}

void on_gtk_changed(GtkTextBuffer *textbuffer, gpointer user_data) {
	SourceView *sv = (SourceView*)user_data;
	//printf("change\n");
	sv->create_text_colors(sv->needs_update_start, sv->needs_update_end);
	hui::run_later(2, [sv] {
		sv->create_colors_if_not_busy();
	});
	sv->color_busy_level ++;
	hui::run_later(0.01f, [sv] {
		sv->update_line_numbers();
	});
}


SourceView::JumpData::JumpData(SourceView *_sv, int _line) {
	sv = _sv;
	line = _line;
}

void SourceView::update_line_numbers() {
	auto a = gtk_scrollable_get_vadjustment(GTK_SCROLLABLE(tv));
	float o = (float)gtk_adjustment_get_value(a);
	line_number_view->set_pos(o);

	GdkRectangle r;
	gtk_text_view_get_visible_rect(GTK_TEXT_VIEW(tv), &r);

	GtkTextIter iter;
	int line_top;
	gtk_text_view_get_line_at_y(GTK_TEXT_VIEW(tv), &iter, r.y, &line_top);

	int line0 = gtk_text_iter_get_line(&iter);
	int num_lines = gtk_text_buffer_get_line_count(tb);

	Array<int> yy;
	for (int l=line0; l<num_lines; l++) {
		gtk_text_buffer_get_iter_at_line(tb, &iter, l);
		int y, height;
		gtk_text_view_get_line_yrange(GTK_TEXT_VIEW(tv), &iter, &y, &height);
		yy.add(y);
		if (y > r.y + r.height)
			break;
	}
	line_number_view->set_line_offsets(line0, yy);
}

void on_gtk_text_view_scrolled(GtkAdjustment* self, SourceView *v) {
	v->update_line_numbers();
}

void on_gtk_text_view_cursor_changed(GObject* self, GParamSpec* pspec, SourceView *v) {
	GtkTextIter start, end;
	gtk_text_buffer_get_selection_bounds(v->tb, &start, &end);
	v->line_number_view->set_cursor_line(gtk_text_iter_get_line(&start));
}

void on_gtk_text_view_size_changed(GObject* self, GParamSpec* pspec, SourceView *v) {
	v->update_line_numbers();
}

SourceView::SourceView(hui::Window *win, const string &_id, Document *d) {
	doc = d;
	id = _id;

	control = win->_get_control_(id);
	tv = control->widget;
	tb = gtk_text_view_get_buffer(GTK_TEXT_VIEW(tv));

	line_no_tb = nullptr;



	for (int i=0; i<NUM_TAG_TYPES; i++)
		tag[i] = gtk_text_buffer_create_tag(tb, NULL, NULL);

	needs_update_start = 0;
	needs_update_end = 0;
	color_busy_level = 0;
	change_return = true;
	scheme = HighlightScheme::default_scheme;
	set_parser(Path::EMPTY);

	line_number_view = new LineNumberView(win, id + "-lines", scheme);

	g_signal_connect(G_OBJECT(tb),"insert-text",G_CALLBACK(on_gtk_insert_text),this);
	g_signal_connect(G_OBJECT(tb),"delete-range",G_CALLBACK(on_gtk_delete_range),this);
	g_signal_connect(G_OBJECT(tb),"changed",G_CALLBACK(on_gtk_changed),this);

	g_signal_connect(G_OBJECT(tv),"move-cursor",G_CALLBACK(on_gtk_move_cursor),this);
	g_signal_connect(G_OBJECT(tv),"copy-clipboard",G_CALLBACK(on_gtk_copy_clipboard),this);
	g_signal_connect(G_OBJECT(tv),"paste-clipboard",G_CALLBACK(on_gtk_paste_clipboard),this);
	g_signal_connect(G_OBJECT(tv),"cut-clipboard",G_CALLBACK(on_gtk_cut_clipboard),this);
	g_signal_connect(G_OBJECT(tv),"toggle-cursor-visible",G_CALLBACK(on_gtk_toggle_cursor_visible),this);
#if !GTK_CHECK_VERSION(4,0,0)
	g_signal_connect(G_OBJECT(tv),"populate-popup",G_CALLBACK(on_gtk_populate_popup),this);
#endif

	//gtk_text_view_set_accepts_tab(GTK_TEXT_VIEW(tv), false);

	d->source_view = this;
	history = d->history;

	update_font();
	//g_object_set(tv, "wrap-mode", GTK_WRAP_WORD_CHAR, NULL);

	hui::run_later(0.05f, [this]{ update_tab_size(); });

	auto a = gtk_scrollable_get_vadjustment(GTK_SCROLLABLE(tv));
	g_signal_connect(G_OBJECT(a),"value-changed",G_CALLBACK(on_gtk_text_view_scrolled), this);

	g_signal_connect(G_OBJECT(win->window), "notify::default-width", G_CALLBACK(on_gtk_text_view_size_changed), this);
	g_signal_connect(G_OBJECT(win->window), "notify::default-height", G_CALLBACK(on_gtk_text_view_size_changed), this);
	g_signal_connect(G_OBJECT(tb), "notify::cursor-position", G_CALLBACK(on_gtk_text_view_cursor_changed), this);

	update_line_numbers();
}

SourceView::~SourceView() {
}


void SourceView::clear() {
	history->enabled = false;

	GtkTextIter start, end;
	gtk_text_buffer_get_bounds(tb, &start, &end);
	gtk_text_buffer_delete(tb, &start, &end);
	gtk_text_buffer_set_modified(tb, false);
	set_parser(Path::EMPTY);

	history->reset();
	history->enabled = true;

}


string convert_to_utf8(string temp) {
	string utf8;// = new char[strlen(temp)];
	const char *t = temp.c_str();
	while (*t) {
		gunichar a = g_utf8_get_char_validated(t, -1);
		if (a != (gunichar)-1)
			utf8.append_1_single(a);
		else
			utf8.append_1_single('?');
		t ++;
	}
	/*return utf8;
	gunichar *aa = g_utf8_to_ucs4(temp, -1, NULL, NULL, NULL);
	char *utf8 = g_ucs4_to_utf8(aa, -1, NULL, NULL, NULL);
	g_free(aa);*/
	return utf8;
}

bool SourceView::fill(const string &text) {
	bool ok = true;
	if (g_utf8_validate((char*)text.data, text.num, NULL)) {
		gtk_text_buffer_set_text(tb, text.c_str(), -1);
		gtk_text_buffer_set_modified(tb, false);
		history->reset();
	} else {
		string temp_utf8 = convert_to_utf8(text);
		gtk_text_buffer_set_text(tb, temp_utf8.c_str(), -1);
		gtk_text_buffer_set_modified(tb, true);
		ok = false;
		history->reset();
		history->saved_pos = -1;
		history->changed = true;
	}

	GtkTextIter start;
	gtk_text_buffer_get_start_iter(tb, &start);
	gtk_text_buffer_place_cursor(tb, &start);
	return ok;
}

bool SourceView::is_undoable() {
	return history->undoable();
}

bool SourceView::is_redoable() {
	return history->redoable();
}

void SourceView::undo() {
	history->undo();
}

void SourceView::redo() {
	history->redo();
}

void SourceView::set_parser(const Path &filename) {
	parser = GetParser(filename);
	doc->parser = parser;
	//msg_write("parser: " + parser->GetName());
	create_text_colors();
}

string sv_get_content(GtkTextBuffer *tb, GtkTextIter start, GtkTextIter end) {
	char *temp = gtk_text_buffer_get_text(tb, &start, &end, false);
	auto r = string(temp, strlen(temp));
	g_free(temp);
	//gtk_text_buffer_set_modified(tb, false);;
	return r;
}

string SourceView::get_all() {
	GtkTextIter start, end;
	gtk_text_buffer_get_bounds(tb, &start, &end);
	return sv_get_content(tb, start, end);
}

string SourceView::get_content(int pos0, int pos1) {
	GtkTextIter start, end;
	gtk_text_buffer_get_iter_at_offset(tb, &start, pos0);
	gtk_text_buffer_get_iter_at_offset(tb, &end, pos1);
	return sv_get_content(tb, start, end);
}

void SourceView::get_selection(int &pos0, int &pos1) {
	GtkTextIter start, end;
	gtk_text_buffer_get_selection_bounds(tb, &start, &end);
	pos0 = gtk_text_iter_get_offset(&start);
	pos1 = gtk_text_iter_get_offset(&end);
}

bool SourceView::has_selection() {
	int p0, p1;
	get_selection(p0, p1);
	return p0 != p1;
}

int SourceView::get_line_no_at(int pos) {
	GtkTextIter iter;
	gtk_text_buffer_get_iter_at_offset(tb, &iter, pos);
	return gtk_text_iter_get_line(&iter);
}

int SourceView::get_line_offset(int line) {
	GtkTextIter start;
	gtk_text_buffer_get_iter_at_line_index(tb, &start, line, 0);
	return gtk_text_iter_get_offset(&start);
}

string SourceView::get_selection_content() {
	GtkTextIter start, end;
	gtk_text_buffer_get_selection_bounds(tb, &start, &end);
	return sv_get_content(tb, start, end);
}

string SourceView::get_line(int line) {
	GtkTextIter start, end;
	gtk_text_buffer_get_iter_at_line_index(tb, &start, line, 0);
	gtk_text_buffer_get_iter_at_line_index(tb, &end, line, 0);
	int pos = 0;
	while (!gtk_text_iter_ends_line(&end)) {
		if (!gtk_text_iter_forward_char(&end))
			break;
		else
			pos ++;
	}
	return sv_get_content(tb, start, end);
}

int SourceView::get_num_lines() {
	return gtk_text_buffer_get_line_count(tb);
}

void SourceView::insert_at(int pos, const string &text) {
	if (text == "")
		return;
	GtkTextIter iter;
	gtk_text_buffer_get_iter_at_offset(tb, &iter, pos);
	gtk_text_buffer_insert(tb, &iter, (const gchar*)text.data, text.num);
	gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(tv), gtk_text_buffer_get_insert(tb));
}

void SourceView::insert_at_cursor(const string &text) {
	if (text == "")
		return;
	gtk_text_buffer_insert_at_cursor(tb, (const gchar*)text.c_str(), text.num);
	gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(tv), gtk_text_buffer_get_insert(tb));
}

void SourceView::delete_selection() {
	gtk_text_buffer_delete_selection(tb, true, true);
	gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(tv), gtk_text_buffer_get_insert(tb));
}

void SourceView::delete_content(int pos0, int pos1) {
	GtkTextIter start, end;
	gtk_text_buffer_get_iter_at_offset(tb, &start, pos0);
	gtk_text_buffer_get_iter_at_offset(tb, &end, pos1);
	gtk_text_buffer_delete(tb, &start, &end);
	gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(tv), gtk_text_buffer_get_insert(tb));
}

void SourceView::clear_markings(int first_line, int last_line) {
	GtkTextIter start, end;
	gtk_text_buffer_get_iter_at_line_offset(tb, &start, first_line, 0);
	gtk_text_buffer_get_iter_at_line_offset(tb, &end, last_line + 1, 0);
	gtk_text_buffer_remove_all_tags(tb, &start, &end);
}

static string word_namespace;

void SourceView::mark_word(int line, int start, int end, int type, char *p0, char *p) {
	if (start == end)
		return;
	string temp;
	if ((int_p)p - (int_p)p0 < 64)
		temp = string(p0, (int_p)p - (int_p)p0);
	if (start == 0)
		word_namespace = "";

	if (type == IN_WORD) {
		//word_namespace
		//if ((start == 0) or (p0[-1] != '.')) {
			int type2 = parser->WordType(word_namespace + temp);
			if (type2 >= 0)
				type = type2;
			if (type == IN_WORD_TYPE)
				word_namespace += temp;
			else
				word_namespace = "";
		//}
	} else if (temp == ".") {
		word_namespace += ".";
	} else {
		word_namespace = "";
	}
	GtkTextIter _start, _end;
	gtk_text_buffer_get_iter_at_line_offset(tb, &_start, line, start);
	gtk_text_buffer_get_iter_at_line_offset(tb, &_end, line, end);
	gtk_text_buffer_apply_tag (tb, tag[type], &_start, &_end);
}



void SourceView::undo_insert_text(int pos, char *text, int length) {
	GtkTextIter start;
	gtk_text_buffer_get_iter_at_offset(tb, &start, pos);
	gtk_text_buffer_insert(tb, &start, text, length);
	gtk_text_buffer_place_cursor(tb, &start);
	gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(tv), gtk_text_buffer_get_insert(tb));
}

void SourceView::undo_remove_text(int pos, char *text, int length) {
	GtkTextIter start, end;
	gtk_text_buffer_get_iter_at_offset(tb, &start, pos);
	gtk_text_buffer_get_iter_at_offset(tb, &end, pos);
	gtk_text_iter_forward_chars(&end, g_utf8_strlen(text, length));
	gtk_text_buffer_delete(tb, &start, &end);
	gtk_text_buffer_place_cursor(tb, &start);
	gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(tv), gtk_text_buffer_get_insert(tb));
}

void SourceView::jump_to_start_of_line(bool shift) {
	GtkTextMark *mi = gtk_text_buffer_get_insert(tb);
	GtkTextMark *msb = gtk_text_buffer_get_selection_bound(tb);
	GtkTextIter ii, isb, i0, i1;
	gtk_text_buffer_get_iter_at_mark(tb, &ii, mi);
	gtk_text_buffer_get_iter_at_mark(tb, &isb, msb);

	int line = gtk_text_iter_get_line(&ii);
	gtk_text_buffer_get_iter_at_line_index(tb, &i0, line, 0);
	i1 = i0;
	while (!gtk_text_iter_ends_line(&i1)) {
		int c = gtk_text_iter_get_char(&i1);
		if (!g_unichar_isspace(c))
			break;
		if (!gtk_text_iter_forward_char(&i1))
			break;
	}
	ii = (gtk_text_iter_equal(&i1, &ii)) ? i0 : i1;
	if (shift)
		gtk_text_buffer_select_range(tb, &ii, &isb);
	else
		gtk_text_buffer_place_cursor(tb, &ii);
	gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(tv), gtk_text_buffer_get_insert(tb));
}

void SourceView::jump_to_end_of_line(bool shift) {
	GtkTextMark *mi = gtk_text_buffer_get_insert(tb);
	GtkTextMark *msb = gtk_text_buffer_get_selection_bound(tb);
	GtkTextIter ii, isb;
	gtk_text_buffer_get_iter_at_mark(tb, &ii, mi);
	gtk_text_buffer_get_iter_at_mark(tb, &isb, msb);

	int line = gtk_text_iter_get_line(&ii);
	gtk_text_buffer_get_iter_at_line_index(tb, &ii, line, 0);
	while (!gtk_text_iter_ends_line(&ii))
		if (!gtk_text_iter_forward_char(&ii))
			break;
	if (shift)
		gtk_text_buffer_select_range(tb, &ii, &isb);
	else
		gtk_text_buffer_place_cursor(tb, &ii);
	gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(tv), gtk_text_buffer_get_insert(tb));
}

void SourceView::move_cursor_to(int line, int pos) {
	GtkTextIter iter;
	gtk_text_buffer_get_iter_at_line_index(tb, &iter, line, 0);
	while (!gtk_text_iter_ends_line(&iter))
		if (!gtk_text_iter_forward_char(&iter))
			break;
	if (gtk_text_iter_get_line_index(&iter) > pos)
		gtk_text_buffer_get_iter_at_line_index(tb, &iter, line, pos);
	gtk_text_buffer_place_cursor(tb, &iter);
	gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(tv), gtk_text_buffer_get_insert(tb));
}

void SourceView::show_line_on_screen(int line) {
	GtkTextIter it;
	gtk_text_buffer_get_iter_at_line(tb, &it, line);
	gtk_text_buffer_place_cursor(tb, &it);
	//gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(tv), gtk_text_buffer_get_insert(tb)); // line is minimally visible
	gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(tv), &it, false, true, 0.0, 0.5); // line is vertically centered
}

void SourceView::insert_return() {
	GtkTextMark *mi = gtk_text_buffer_get_insert(tb);
	GtkTextIter ii, i0, i1;
	gtk_text_buffer_get_iter_at_mark(tb, &ii, mi);
	int line = gtk_text_iter_get_line(&ii);
	gtk_text_buffer_get_iter_at_line_index(tb, &i0, line, 0);
	i1 = i0;
	while ((!gtk_text_iter_ends_line(&i1)) and (!gtk_text_iter_equal(&i1, &ii))) {
		int c = gtk_text_iter_get_char(&i1);
		if (!g_unichar_isspace(c))
			break;
		if (!gtk_text_iter_forward_char(&i1))
			break;
	}
	char *text = gtk_text_buffer_get_text(tb, &i0, &i1, false);
	gtk_text_buffer_insert_at_cursor(tb, "\n", -1);
	gtk_text_buffer_insert_at_cursor(tb, text, -1);
	g_free(text);
}

void SourceView::get_cur_line_pos(int &line, int &pos) {
	GtkTextIter ii;
	gtk_text_buffer_get_iter_at_mark(tb, &ii, gtk_text_buffer_get_insert(tb));
	line = gtk_text_iter_get_line(&ii);
	pos = gtk_text_iter_get_line_offset(&ii);
}


void SourceView::set_tag(int i, const char *fg_color, const char *bg_color, bool bold, bool italic) {
	g_object_set(tag[i], "foreground", fg_color, NULL);
	if (bg_color)
		g_object_set(tag[i], "background", bg_color, NULL);
	else
		g_object_steal_data(G_OBJECT(tag[i]), "background");
	g_object_set(tag[i], "weight", bold ? PANGO_WEIGHT_BOLD : PANGO_WEIGHT_NORMAL, NULL);
	g_object_set(tag[i], "style", italic ? PANGO_STYLE_ITALIC : PANGO_STYLE_NORMAL, NULL);
}

string color_to_hex(const color &c) {
	return c.hex().sub(0, 7); // skip alpha
}

void color2gdkrgba(const color &c, GdkRGBA &g) {
	g.alpha = 1;
	g.red = c.r;
	g.green = c.g;
	g.blue = c.b;
}

void SourceView::apply_scheme(HighlightScheme *s) {
	for (int i=0; i<NUM_TAG_TYPES; i++) {
		if (s->context[i].set_bg)
			set_tag(i, color_to_hex(s->context[i].fg).c_str(), color_to_hex(s->context[i].bg).c_str(), s->context[i].bold, s->context[i].italic);
		else
			set_tag(i, color_to_hex(s->context[i].fg).c_str(), NULL, s->context[i].bold, s->context[i].italic);
	}
#if !GTK_CHECK_VERSION(4,0,0)
	if (false){
	GdkRGBA _color;
	color2gdkrgba(s->bg, _color);
	gtk_widget_override_background_color(tv, GTK_STATE_FLAG_NORMAL, &_color);
	color2gdkrgba(s->context[IN_WORD].fg, _color);
	gtk_widget_override_background_color(tv, GTK_STATE_FLAG_SELECTED, &_color);
	color2gdkrgba(s->bg, _color);
	gtk_widget_override_color(tv, GTK_STATE_FLAG_SELECTED, &_color);
	color2gdkrgba(s->context[IN_WORD].fg, _color);
	gtk_widget_override_color(tv, GTK_STATE_FLAG_NORMAL, &_color);
	gtk_widget_override_cursor(tv, &_color, &_color);
	}
#endif
	scheme = s;
	hui::config.set_str("HighlightScheme", s->name);
}

void SourceView::update_tab_size() {
	int tab_size = hui::config.get_int("TabWidth", 8);
	PangoLayout *layout = gtk_widget_create_pango_layout(tv, "W");
	int width, height;
	pango_layout_get_pixel_size(layout, &width, &height);
	PangoTabArray *ta = pango_tab_array_new(1, true);
	pango_tab_array_set_tab(ta, 0, PANGO_TAB_LEFT, width * tab_size);
	gtk_text_view_set_tabs(GTK_TEXT_VIEW(tv), ta);
}

void SourceView::update_font() {

	string font_name = hui::config.get_str("Font", "Monospace 10");
	PangoFontDescription *font_desc = pango_font_description_from_string(font_name.c_str());


#if GTK_CHECK_VERSION(4,0,0)
	float size = (float)pango_font_description_get_size(font_desc) / 1024.0f;
	string family = pango_font_description_get_family(font_desc);

	//control->_set_css(format("* { font-family: %s; font-size: %.1fpt; }", family, size));
//	control->add_css_class("monospace");
	gtk_text_view_set_monospace(GTK_TEXT_VIEW(tv), true);


	auto *css_provider = gtk_css_provider_new();

	string css = format(".hui-source-view { font-size: %.1fpt; }", size);

	gtk_css_provider_load_from_data(css_provider, (char*)css.data, css.num);
	gtk_style_context_add_provider_for_display(gdk_display_get_default(), GTK_STYLE_PROVIDER(css_provider),  GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

	control->add_css_class("hui-source-view");
#else
	gtk_widget_override_font(tv, font_desc);
	//if (line_no_tv)
	//	gtk_widget_override_font(line_no_tv, font_desc);
	pango_font_description_free(font_desc);
#endif

	hui::run_later(0.010f, [this, font_name] {
		auto ccc = gtk_widget_get_pango_context(tv);
		auto fd = pango_context_get_font_description(ccc);
		auto mmm = pango_context_get_metrics(ccc, fd, nullptr);
		float lh = pango_units_to_double(pango_font_metrics_get_height(mmm));
		//line_number_view->set_font(pango_font_description_to_string(fd), lh);// + 1.0f);
		line_number_view->set_font(font_name, lh);
		update_line_numbers();
		update_tab_size();
	});
}


bool SourceView::find(const string &str) {
	// find...
	GtkTextIter ii, isb;
	gtk_text_buffer_get_iter_at_mark(tb, &ii, gtk_text_buffer_get_insert(tb));
	//int off = gtk_text_iter_get_offset(&ii);
	//int nn = gtk_text_buffer_get_char_count(tb);

	string temp;
	temp.resize(str.num);
	for (int i=0;i<str.num;i++){
		temp[i] = gtk_text_iter_get_char(&ii);
		if (gtk_text_iter_forward_char(&ii))
			break;
	}

	bool found = false;
	while(true){
		if (temp == str){
			found = true;
			break;
		}
		if (!gtk_text_iter_forward_char(&ii))
			break;
		for (int i=0;i<str.num - 1;i++)
			temp[i] = temp[i + 1];
		temp[str.num - 1] = gtk_text_iter_get_char(&ii);
	}
	if (found){
		gtk_text_iter_forward_char(&ii);
		isb = ii;
		gtk_text_iter_backward_chars(&ii, str.num);
		gtk_text_buffer_select_range(tb, &ii, &isb);
		gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(tv), gtk_text_buffer_get_insert(tb));
	}
	return found;
}


#pragma GCC diagnostic pop
