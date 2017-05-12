/*
 * CommandDialog.cpp
 *
 *  Created on: 05.06.2013
 *      Author: michi
 */

#include "CommandDialog.h"

extern string LastCommand;
void ExecuteCommand(const string&);

CommandDialog::CommandDialog(Window *parent) :
	hui::Window("command_dialog", parent)
{
	setString("command", LastCommand);
	event("ok", std::bind(&CommandDialog::onOk, this));
	event("cancel", std::bind(&CommandDialog::onCancel, this));
}

CommandDialog::~CommandDialog()
{
}


void CommandDialog::onOk()
{
	LastCommand = getString("command");
	ExecuteCommand(LastCommand);
}

void CommandDialog::onCancel()
{
	delete(this);
}
