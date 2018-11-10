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
	set_string("command", last_command);
	event("ok", std::bind(&CommandDialog::on_ok, this));
	event("cancel", std::bind(&CommandDialog::on_cancel, this));
}

CommandDialog::~CommandDialog()
{
}


void CommandDialog::on_ok()
{
	last_command = get_string("command");
	sgribthmaker->ExecuteCommand(last_command);
}

void CommandDialog::on_cancel()
{
	destroy();
}
