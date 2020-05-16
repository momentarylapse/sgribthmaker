/*
 * CommandDialog.cpp
 *
 *  Created on: 05.06.2013
 *      Author: michi
 */

#include "CommandDialog.h"
#include "SgribthMaker.h"

string CommandDialog::prev_command;
string CommandDialog::prev_subsitude;

CommandDialog::CommandDialog(SgribthMaker *_sgribthmaker) :
	hui::Dialog("command_dialog", _sgribthmaker->MainWin)
{
	sgribthmaker = _sgribthmaker;
	hide_control("substitude", true);
	set_string("command", prev_command);
	set_string("substitude", prev_subsitude);
	event("ok", [=]{ on_ok(); });
	event("cancel", [=]{ destroy(); });
	event("replace", [=]{ hide_control("substitude", !is_checked("replace")); });
}

CommandDialog::~CommandDialog()
{
}


void CommandDialog::on_ok() {
	prev_command = get_string("command");
	prev_subsitude = get_string("substitude");
	sgribthmaker->ExecuteCommand(prev_command);
}
