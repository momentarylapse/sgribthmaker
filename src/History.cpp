/*
 * History.cpp
 *
 *  Created on: 05.06.2013
 *      Author: michi
 */

#include "History.h"
#include "Document.h"
#include "SourceView.h"
#include "lib/hui/hui.h"


History::History(Document* _doc) {
	pos = 0;
	saved_pos = 0;
	changed = false;
	enabled = true;
	doc = _doc;
}

History::~History() {
}



void History::reset() {
	stack.clear();
	pos = 0;
	saved_pos = 0;
	changed = false;
	enabled = true;
	out_changed.notify();
}

bool History::undoable() {
	return pos > 0;
}

bool History::redoable() {
	return pos < stack.num;
}

void History::undo() {
	if (!undoable())
		return;
	enabled = false;
	pos --;
	Command *u = stack[pos];
	u->undo(doc);
	enabled = true;
	changed = (pos != saved_pos);
	out_changed.notify();
}

void History::redo() {
	if (!redoable())
		return;
	enabled = false;
	Command *u = stack[pos];
	u->execute(doc);
	pos ++;
	enabled = true;
	changed = (pos != saved_pos);
	out_changed.notify();
}



void History::execute(History::Command *c) {
	if (!enabled)
		return;

	// remove illegal actions
	stack.resize(pos);
	if (pos < saved_pos)
		saved_pos = -1;

	// insert
	stack.add(c);
	pos ++;
	changed = true;
	out_changed.notify();
}

void History::define_as_saved() {
	saved_pos = pos;
	changed = false;
	out_changed.notify();
}

CommandInsert::CommandInsert(char *_text, int _length, int _pos) {
	text = _text;
	length = _length;
	pos = _pos;
}

CommandInsert::~CommandInsert() {
	g_free(text);
}

void CommandInsert::execute(Document* doc) {
	doc->source_view->undo_insert_text(pos, text, length);
}

void CommandInsert::undo(Document* doc) {
	doc->source_view->undo_remove_text(pos, text, length);
}

CommandDelete::CommandDelete(char *_text, int _length, int _pos) {
	text = _text;
	length = _length;
	pos = _pos;
}

CommandDelete::~CommandDelete() {
	g_free(text);
}

void CommandDelete::execute(Document* doc) {
	doc->source_view->undo_remove_text(pos, text, length);
}

void CommandDelete::undo(Document* doc) {
	doc->source_view->undo_insert_text(pos, text, length);
}

