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
	explicit History(Document* doc);
	~History() override;

	void reset();

	bool undoable() const;
	bool redoable() const;

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
		virtual ~Command() = default;
		virtual void execute(Document* doc) = 0;
		virtual void undo(Document* doc) = 0;
	};
	owned_array<Command> stack;


	void execute(Command *c);
};

class CommandInsert : public History::Command {
	int pos;
	string text;
public:
	CommandInsert(const string& text, int pos);
	void execute(Document* doc) override;
	void undo(Document* doc) override;
};

class CommandDelete : public History::Command {
	int pos;
	string text;
public:
	CommandDelete(const string& text, int pos);
	void execute(Document* doc) override;
	void undo(Document* doc) override;
};

class CommandGroup : public History::Command {
public:
	owned_array<Command> commands;
	void execute(Document* doc) override;
	void undo(Document* doc) override;
};

#endif /* HISTORY_H_ */
