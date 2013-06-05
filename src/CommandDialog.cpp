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
	HuiWindow("command_dialog", parent, false)
{
	SetString("command", LastCommand);
	EventM("ok", this, &CommandDialog::OnOk);
	EventM("cancel", this, &CommandDialog::OnClose);
	EventM("hui:close", this, &CommandDialog::OnClose);
}

CommandDialog::~CommandDialog()
{
}


void CommandDialog::OnOk()
{
	LastCommand = GetString("command");
	ExecuteCommand(LastCommand);
}

void CommandDialog::OnClose()
{
	delete(this);
}
