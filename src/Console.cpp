/*
 * Console.cpp
 *
 *  Created on: 18.03.2014
 *      Author: michi
 */

#include "Console.h"

Console::Console() {
	from_resource("console-panel");
	show(false);

	event("console-close", [this] { on_close(); });
}

void Console::on_close() {
	show(false);
}

void Console::show(bool show) {
	hide_control("grid-console", !show);
}

void Console::set(const string &msg) {
	show(true);
	set_string("log", msg);
	text = msg;
}

void Console::clear() {
	reset("log");
	text = "";
}

void Console::add(const string &msg) {
	set(text + msg);
}

