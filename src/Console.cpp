/*
 * Console.cpp
 *
 *  Created on: 18.03.2014
 *      Author: michi
 */

#include "Console.h"

Console::Console()
{
	AddControlTable("", 0, 0, 1, 2, "table_console");
	//SetBorderWidth(5);
	SetTarget("table_console", 0);
	AddMultilineEdit("", 0, 0, 0, 0, "log");
	Enable("log", false);
	AddControlTable("!noexpandy", 0, 1, 3, 1, "table_command");
	SetTarget("table_command", 0);
	AddEdit("!expandx", 0, 0, 0, 0, "console");
	AddButton("OK", 1, 0, 0, 0, "console_ok");
	SetImage("console_ok", "hui:ok");
	AddButton("", 2, 0, 0, 0, "console_close");
	SetImage("console_close", "hui:close");
	show(false);


	EventM("console_close", this, &Console::onClose);
}

Console::~Console()
{
}

void Console::onClose()
{
	HideControl("table_console", true);
}

void Console::show(bool show)
{
	HideControl("table_console", !show);
}

void Console::set(const string &msg)
{
	show(true);
	SetString("log", msg);
}

