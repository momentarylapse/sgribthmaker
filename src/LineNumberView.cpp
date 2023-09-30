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

void LineNumberView::on_draw(Painter *p) {
	p->set_font(font_name, font_size, false, false);

	int i0 = (int)(offset / line_height);
	int i1 = (int)((offset + p->height) / line_height) + 1;

	float dx = 8;
	float dy = line_height * 0.25f;

	for (int i=i0; i<i1; i++) {
		p->set_color(scheme->context[IN_LINE_COMMENT].fg);
		if (i == cursor_line)
			p->set_color(scheme->context[IN_WORD].fg);
		string s = str(i + 1);
		p->draw_str({p->width - p->get_str_width(s) - dx, i*line_height + dy - offset}, s);
	}
}


