/*
 * History.cpp
 *
 *  Created on: 05.06.2013
 *      Author: michi
 */

#include "History.h"
#include "lib/hui/hui.h"

void UpdateMenu();

void undo_insert_text(int pos, char *text, int length);
void undo_remove_text(int pos, char *text, int length);

History::History()
{
	pos = 0;
	saved_pos = 0;
	changed = false;
	enabled = true;
}

History::~History()
{
	for (int i=0;i<stack.num;i++)
		delete(stack[i]);
}



void History::Reset()
{
	for (int i=0;i<stack.num;i++)
		delete(stack[i]);
	stack.clear();
	pos = 0;
	saved_pos = 0;
	changed = false;
	enabled = true;
	UpdateMenu();
}

bool History::Undoable()
{
	return pos > 0;
}

bool History::Redoable()
{
	return pos < stack.num - 1;
}

void History::Undo()
{
	if (!Undoable())
		return;
	enabled = false;
	pos --;
	Command *u = stack[pos];
	u->Undo();
	enabled = true;
	changed = (pos != saved_pos);
	UpdateMenu();
}

void History::Redo()
{
	if (!Redoable())
		return;
	enabled = false;
	Command *u = stack[pos];
	u->Execute();
	pos ++;
	enabled = true;
	changed = (pos != saved_pos);
	UpdateMenu();
}



void History::Execute(History::Command *c)
{
	if (!enabled)
		return;

	// remove illegal actions
	for (int i=pos; i<stack.num; i++)
		delete(stack[i]);
	stack.resize(pos);
	if (pos < saved_pos)
		saved_pos = -1;

	// insert
	stack.add(c);
	pos ++;
	changed = true;
	UpdateMenu();
}

void History::DefineAsSaved()
{
	saved_pos = pos;
	changed = false;
	UpdateMenu();
}

CommandInsert::CommandInsert(char *_text, int _length, int _pos)
{
	text = _text;
	length = _length;
	pos = _pos;
}

CommandInsert::~CommandInsert()
{
	g_free(text);
}

void CommandInsert::Execute()
{
	undo_insert_text(pos, text, length);
}

void CommandInsert::Undo()
{
	undo_remove_text(pos, text, length);
}

CommandDelete::CommandDelete(char *_text, int _length, int _pos)
{
	text = _text;
	length = _length;
	pos = _pos;
}

CommandDelete::~CommandDelete()
{
	g_free(text);
}

void CommandDelete::Execute()
{
	undo_remove_text(pos, text, length);
}

void CommandDelete::Undo()
{
	undo_insert_text(pos, text, length);
}

