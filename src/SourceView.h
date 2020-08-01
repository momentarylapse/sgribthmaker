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

	void Clear();
	bool Fill(const string &text);
	string GetAll();
	string GetSelection();
	string GetLine(int line);
	int GetNumLines();
	void DeleteSelection();
	void InsertAtCursor(const string &text);

	void ApplyScheme(HighlightScheme *s);
	void SetTag(int i, const char *fg_color, const char *bg_color, bool bold, bool italic);
	void UpdateFont();
	void UpdateTabSize();

	void ClearMarkings(int first_line, int last_line);
	void MarkWord(int line, int start, int end, int type, char *p0, char *p);
	void CreateColorsIfNotBusy();
	void CreateTextColors(int first_line = -1, int last_line = -1);

	void undo_insert_text(int pos, char *text, int length);
	void undo_remove_text(int pos, char *text, int length);


	bool Undoable();
	bool Redoable();
	void Undo();
	void Redo();

	void JumpToStartOfLine(bool shift);
	void JumpToEndOfLine(bool shift);
	void MoveCursorTo(int line, int pos);
	void ShowLineOnScreen(int line);
	void InsertReturn();

	void GetCurLinePos(int &line, int &pos);

	bool Find(const string &str);

	string id;
	GtkTextBuffer *tb;
	GtkWidget *tv;
	GtkTextBuffer *line_no_tb;
	GtkWidget *line_no_tv;

	GtkTextTag *tag[NUM_TAG_TYPES];

	//void SwitchToDocument(Document *d);
	Document *doc;
	History *history;

	int NeedsUpdateStart, NeedsUpdateEnd;
	int color_busy_level;
	bool change_return;

	struct JumpData {
		SourceView *sv;
		int line;
		JumpData(){}
		JumpData(SourceView *sv, int line);
	};
	Array<JumpData> jump_data;

	void SetParser(const Path &filename);
	Parser *parser;
	HighlightScheme *scheme;
};

#endif /* SOURCEVIEW_H_ */
