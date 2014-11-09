/*
 * Console.cpp
 *
 *  Created on: 18.03.2014
 *      Author: michi
 */

#include "Console.h"

Console::Console()
{
	addControlTable("", 0, 0, 1, 2, "table_console");
	//SetBorderWidth(5);
	setTarget("table_console", 0);
	addMultilineEdit("", 0, 0, 0, 0, "log");
	enable("log", false);
	addControlTable("!noexpandy", 0, 1, 3, 1, "table_command");
	setTarget("table_command", 0);
	addEdit("!expandx", 0, 0, 0, 0, "console");
	addButton("OK", 1, 0, 0, 0, "console_ok");
	setImage("console_ok", "hui:ok");
	addButton("", 2, 0, 0, 0, "console_close");
	setImage("console_close", "hui:close");
	show(false);


	event("console_close", this, &Console::onClose);
}

Console::~Console()
{
}

void Console::onClose()
{
	show(false);
}

void Console::show(bool show)
{
	hideControl("table_console", !show);
}

void Console::set(const string &msg)
{
	show(true);
	setString("log", msg);
}

