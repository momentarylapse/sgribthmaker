/*
 * Console.h
 *
 *  Created on: 18.03.2014
 *      Author: michi
 */

#ifndef CONSOLE_H_
#define CONSOLE_H_

#include "lib/hui/hui.h"

class Console : public hui::Panel {
public:
	Console();

	void clear();
	void set(const string &msg);
	void add(const string &msg);
	void show(bool show);

	void on_close();

	string text;
};

#endif /* CONSOLE_H_ */
