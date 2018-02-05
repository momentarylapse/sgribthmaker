/*
 * Console.cpp
 *
 *  Created on: 18.03.2014
 *      Author: michi
 */

#include "Console.h"

Console::Console()
{
	addGrid("", 0, 0, "table_console");
	//SetBorderWidth(5);
	setTarget("table_console");
	addMultilineEdit("", 0, 0, "log");
	enable("log", false);
	addGrid("!noexpandy", 0, 1, "table_command");
	setTarget("table_command");
	addEdit("!expandx", 0, 0, "console");
	addButton("OK", 1, 0, "console_ok");
	setImage("console_ok", "hui:ok");
	addButton("", 2, 0, "console_close");
	setImage("console_close", "hui:close");
	show(false);


	event("console_close", std::bind(&Console::onClose, this));
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

