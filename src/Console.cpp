/*
 * Console.cpp
 *
 *  Created on: 18.03.2014
 *      Author: michi
 */

#include "Console.h"

Console::Console()
{
	add_grid("", 0, 0, "table_console");
	//SetBorderWidth(5);
	set_target("table_console");
	add_multiline_edit("", 0, 0, "log");
	enable("log", false);
	add_grid("!noexpandy", 0, 1, "table_command");
	set_target("table_command");
	add_edit("!expandx", 0, 0, "console");
	add_button("OK", 1, 0, "console_ok");
	set_image("console_ok", "hui:ok");
	add_button("", 2, 0, "console_close");
	set_image("console_close", "hui:close");
	show(false);


	event("console_close", std::bind(&Console::on_close, this));
}

Console::~Console()
{
}

void Console::on_close()
{
	show(false);
}

void Console::show(bool show)
{
	hide_control("table_console", !show);
}

void Console::set(const string &msg)
{
	show(true);
	set_string("log", msg);
}

