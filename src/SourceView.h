/*
 * SourceView.h
 *
 *  Created on: 06.06.2013
 *      Author: michi
 */

#ifndef SOURCEVIEW_H_
#define SOURCEVIEW_H_

#include "lib/hui/hui.h"
#include "HighlightScheme.h"

class Document;
class History;
class Parser;

class SourceView : public hui::EventHandler {
public:
	SourceView(hui::Window *win, const string &id, Document *d);
	virtual ~SourceView();

	void clear();
	bool fill(const string &text);
	string get_all();
	string get_content(int pos0, int pos1);
	int get_line_offset(int line);
	void get_selection(int &pos0, int &pos1);
	bool has_selection();
	int get_line_no_at(int pos);
	int get_line_offset_at(int pos);
	string get_selection_content();
	string get_line(int line);
	int get_num_lines();
	void delete_selection();
	void delete_content(int pos0, int pos1);
	void insert_at(int pos, const string &text);
	void insert_at_cursor(const string &text);

	void apply_scheme(HighlightScheme *s);
	void set_tag(int i, const char *fg_color, const char *bg_color, bool bold, bool italic);
	void update_font();
	void update_tab_size();

	void clear_markings(int first_line, int last_line);
	void mark_word(int line, int start, int end, int type, char *p0, char *p);
	void create_colors_if_not_busy();
	void create_text_colors(int first_line = -1, int last_line = -1);

	void undo_insert_text(int pos, char *text, int length);
	void undo_remove_text(int pos, char *text, int length);


	bool is_undoable();
	bool is_redoable();
	void undo();
	void redo();

	void jump_to_start_of_line(bool shift);
	void jump_to_end_of_line(bool shift);
	void move_cursor_to(int line, int pos);
	void show_line_on_screen(int line);
	void insert_return();

	void get_cur_line_pos(int &line, int &pos);

	bool find(const string &str);

	string id;
	hui::Control *control;
	GtkTextBuffer *tb;
	GtkWidget *tv;
	GtkTextBuffer *line_no_tb;
	GtkWidget *line_no_tv;

	GtkTextTag *tag[NUM_TAG_TYPES];

	Document *doc;
	History *history;

	int needs_update_start, needs_update_end;
	int color_busy_level;
	bool change_return;

	struct JumpData {
		SourceView *sv;
		int line;
		JumpData(){}
		JumpData(SourceView *sv, int line);
	};
	Array<JumpData> jump_data;

	void set_parser(const Path &filename);
	Parser *parser;
	HighlightScheme *scheme;
};

#endif /* SOURCEVIEW_H_ */
