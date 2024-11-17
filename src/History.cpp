/*
 * History.cpp
 *
 *  Created on: 05.06.2013
 *      Author: michi
 */

#include "History.h"
#include "Document.h"
#include "SourceView.h"
#include "lib/base/sort.h"
#include "lib/hui/hui.h"


History::History(Document* _doc) {
	pos = 0;
	saved_pos = 0;
	changed = false;
	enabled = true;
	doc = _doc;
}

History::~History() = default;



void History::reset() {
	stack.clear();
	pos = 0;
	saved_pos = 0;
	changed = false;
	enabled = true;
	out_changed.notify();
}

bool History::undoable() const {
	return pos > 0;
}

bool History::redoable() const {
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

CommandInsert::CommandInsert(const string& _text, int _pos) {
	text = _text;
	pos = _pos;
}

void CommandInsert::execute(Document* doc) {
	doc->source_view->undo_insert_text(pos, text);
}

void CommandInsert::undo(Document* doc) {
	doc->source_view->undo_remove_text(pos, text);
}

CommandDelete::CommandDelete(const string& _text, int _pos) {
	text = _text;
	pos = _pos;
}

void CommandDelete::execute(Document* doc) {
	doc->source_view->undo_remove_text(pos, text);
}

void CommandDelete::undo(Document* doc) {
	doc->source_view->undo_insert_text(pos, text);
}

void CommandGroup::execute(Document* doc) {
	for (auto c: weak(commands))
		c->execute(doc);
}

void CommandGroup::undo(Document* doc) {
	for (auto c: base::reverse(weak(commands)))
		c->undo(doc);
}


