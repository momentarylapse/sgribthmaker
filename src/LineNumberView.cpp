/*
 * LineNumberView.cpp
 *
 *  Created on: 30 Sept 2023
 *      Author: michi
 */

#include "LineNumberView.h"
#include "HighlightScheme.h"


LineNumberView::LineNumberView(hui::Window *_win, const string &_id,  HighlightScheme *_scheme) {
	font_name = "Monospace";
	font_size = 10;
	line_height = 18;
	cursor_line = 0;
	offset = 0;
	win = _win;
	id = _id;
	scheme = _scheme;
	win->event_xp(id, hui::EventID::DRAW, [this] (Painter *p) { on_draw(p); });
}

void LineNumberView::set_font(const string &f, float lh) {
	font_name = implode(f.explode(" ").sub_ref(0, -1), " ");
	font_size = f.explode(" ").back()._float();
	line_height = lh;
	win->redraw(id);
}

void LineNumberView::set_pos(float pos) {
	offset = pos;
	win->redraw(id);
}


void LineNumberView::set_cursor_line(int line) {
	cursor_line = line;
	win->redraw(id);
}

void LineNumberView::set_line_offsets(int _line0, const Array<int> &y) {
	line0 = _line0;
	line_offsets = y;
	win->redraw(id);
}

void LineNumberView::on_draw(Painter *p) {
	p->set_font(font_name, font_size, false, false);

	int i0 = (int)(offset / line_height);
	int i1 = (int)((offset + p->height) / line_height) + 1;

	float dx = p->get_str_width("M") * 0.6f;
	float dy = line_height * 0.25f;

	int wreq = p->get_str_width(str(line0 + line_offsets.num)) + dx * 2;
	if (wreq > p->width)
		win->set_options(id, "width=" + i2s(wreq));

	if (line_offsets.num == 0) {
		for (int i=i0; i<i1; i++) {
			p->set_color(scheme->context[IN_LINE_COMMENT].fg);
			if (i == cursor_line)
				p->set_color(scheme->context[IN_WORD].fg);
			string s = str(i + 1);
			p->draw_str({p->width - p->get_str_width(s) - dx, i*line_height + dy - offset}, s);
		}
	} else {
		for (int i=0; i<line_offsets.num; i++) {
			p->set_color(scheme->context[IN_LINE_COMMENT].fg);
			int l = line0 + i;
			if (l == cursor_line)
				p->set_color(scheme->context[IN_WORD].fg);
			string s = str(l + 1);
			p->draw_str({p->width - p->get_str_width(s) - dx, line_offsets[i] + dy - offset}, s);
		}

	}
}


