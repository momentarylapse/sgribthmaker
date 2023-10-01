/*
 * LineNumberView.h
 *
 *  Created on: 30 Sept 2023
 *      Author: michi
 */

#ifndef SRC_LINENUMBERVIEW_H_
#define SRC_LINENUMBERVIEW_H_

#include "lib/hui/hui.h"

class HighlightScheme;

class LineNumberView : public hui::EventHandler {
public:
	LineNumberView(hui::Window *win, const string &id, HighlightScheme *scheme);
	void on_draw(Painter *p);

	void set_font(const string& font, float line_height);
	void set_pos(float pos);
	void set_cursor_line(int line);
	void set_line_offsets(int line0, const Array<int> &y);

	hui::Window *win;
	string id;
	float line_height;
	int cursor_line;
	string font_name;
	float font_size;
	float offset;
	int line0;
	Array<int> line_offsets;
	HighlightScheme *scheme;
};

#endif /* SRC_LINENUMBERVIEW_H_ */
