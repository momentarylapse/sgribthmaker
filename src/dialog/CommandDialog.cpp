/*
 * CommandDialog.cpp
 *
 *  Created on: 05.06.2013
 *      Author: michi
 */

#include "CommandDialog.h"
#include "../SgribthMakerWindow.h"

string CommandDialog::prev_command;
string CommandDialog::prev_subsitude;

CommandDialog::CommandDialog(SgribthMakerWindow *_mw) :
	hui::Dialog("command_dialog", _mw)
{
	main_win = _mw;
	hide_control("substitude", true);
	set_string("command", prev_command);
	set_string("substitude", prev_subsitude);
	event("ok", [=]{ on_ok(); });
	event("cancel", [=]{ request_destroy(); });
	event("replace", [=]{ hide_control("substitude", !is_checked("replace")); });
}


void CommandDialog::on_ok() {
	prev_command = get_string("command");
	prev_subsitude = get_string("substitude");
	main_win->execute_command(prev_command);
}
