/*
 * CommandDialog.cpp
 *
 *  Created on: 05.06.2013
 *      Author: michi
 */

#include "CommandDialog.h"
#include "SgribthMaker.h"

CommandDialog::CommandDialog(SgribthMaker *_sgribthmaker) :
	hui::Window("command_dialog", _sgribthmaker->MainWin)
{
	sgribthmaker = _sgribthmaker;
	setString("command", last_command);
	event("ok", std::bind(&CommandDialog::onOk, this));
	event("cancel", std::bind(&CommandDialog::onCancel, this));
}

CommandDialog::~CommandDialog()
{
}


void CommandDialog::onOk()
{
	last_command = getString("command");
	sgribthmaker->ExecuteCommand(last_command);
}

void CommandDialog::onCancel()
{
	destroy();
}
