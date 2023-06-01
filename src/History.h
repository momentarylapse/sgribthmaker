/*
 * History.h
 *
 *  Created on: 05.06.2013
 *      Author: michi
 */

#ifndef HISTORY_H_
#define HISTORY_H_

#include "lib/base/base.h"
#include "lib/base/pointer.h"
#include "lib/pattern/Observable.h"

class SourceView;
class Document;

class History : public obs::Node<VirtualBase> {
public:
	History(Document* doc);
	virtual ~History();

	void reset();

	bool undoable();
	bool redoable();

	void undo();
	void redo();

	void define_as_saved();

	bool changed;
	bool enabled;


	int pos;
	int saved_pos;
	Document* doc;

	class Command {
	public:
		virtual ~Command() {}
		virtual void execute(Document* doc) = 0;
		virtual void undo(Document* doc) = 0;
	};
	owned_array<Command> stack;


	void execute(Command *c);
};

class CommandInsert : public History::Command {
	int pos, length;
	char *text;
public:
	CommandInsert(char *text, int length, int pos);
	virtual ~CommandInsert();
	virtual void execute(Document* doc);
	virtual void undo(Document* doc);
};

class CommandDelete : public History::Command {
	int pos, length;
	char *text;
public:
	CommandDelete(char *text, int length, int pos);
	virtual ~CommandDelete();
	virtual void execute(Document* doc);
	virtual void undo(Document* doc);
};

#endif /* HISTORY_H_ */
