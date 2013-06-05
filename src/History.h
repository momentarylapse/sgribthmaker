/*
 * History.h
 *
 *  Created on: 05.06.2013
 *      Author: michi
 */

#ifndef HISTORY_H_
#define HISTORY_H_

#include "lib/base/base.h"

class History
{
public:
	History();
	virtual ~History();

	void Reset();

	bool Undoable();
	bool Redoable();

	void Undo();
	void Redo();

	void DefineAsSaved();

	bool changed;
	bool enabled;


	int pos;
	int saved_pos;
	class Command{
	public:
		virtual ~Command(){}
		virtual void Execute() = 0;
		virtual void Undo() = 0;
	};
	Array<Command*> stack;



	void Execute(Command *c);

};

class CommandInsert : public History::Command
{
	int pos, length;
	char *text;
public:
	CommandInsert(char *text, int length, int pos);
	virtual ~CommandInsert();
	virtual void Execute();
	virtual void Undo();
};

class CommandDelete : public History::Command
{
	int pos, length;
	char *text;
public:
	CommandDelete(char *text, int length, int pos);
	virtual ~CommandDelete();
	virtual void Execute();
	virtual void Undo();
};

#endif /* HISTORY_H_ */
