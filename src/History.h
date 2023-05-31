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

class SourceView;

class History {
public:
	History(SourceView *sv);
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

	class Command {
	public:
		virtual ~Command() {}
		virtual void execute(SourceView *sv) = 0;
		virtual void undo(SourceView *sv) = 0;
	};
	owned_array<Command> stack;


	void execute(Command *c);
	SourceView *sv;
};

class CommandInsert : public History::Command {
	int pos, length;
	char *text;
public:
	CommandInsert(char *text, int length, int pos);
	virtual ~CommandInsert();
	virtual void execute(SourceView *sv);
	virtual void undo(SourceView *sv);
};

class CommandDelete : public History::Command {
	int pos, length;
	char *text;
public:
	CommandDelete(char *text, int length, int pos);
	virtual ~CommandDelete();
	virtual void execute(SourceView *sv);
	virtual void undo(SourceView *sv);
};

#endif /* HISTORY_H_ */
