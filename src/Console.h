/*
 * Console.h
 *
 *  Created on: 18.03.2014
 *      Author: michi
 */

#ifndef CONSOLE_H_
#define CONSOLE_H_

#include "lib/hui/hui.h"

class Console : public HuiPanel
{
public:
	Console();
	virtual ~Console();

	void set(const string &msg);
	void show(bool show);

	void onClose();
};

#endif /* CONSOLE_H_ */
