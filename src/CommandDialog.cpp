/*
 * CommandDialog.cpp
 *
 *  Created on: 05.06.2013
 *      Author: michi
 */

#include "CommandDialog.h"

extern string LastCommand;
void ExecuteCommand(const string&);

CommandDialog::CommandDialog(HuiWindow *parent) :
	HuiWindow("command_dialog", parent)
{
	setString("command", LastCommand);
	event("ok", this, &CommandDialog::onOk);
	event("cancel", this, &CommandDialog::onCancel);
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
