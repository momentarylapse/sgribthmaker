/*
 * CommandDialog.h
 *
 *  Created on: 05.06.2013
 *      Author: michi
 */

#ifndef COMMANDDIALOG_H_
#define COMMANDDIALOG_H_

#include "lib/hui/hui.h"

class SgribthMaker;

class CommandDialog : public hui::Dialog {
public:
	CommandDialog(SgribthMaker *sgribthmaker);
	virtual ~CommandDialog();

	void on_ok();

	SgribthMaker *sgribthmaker;

	static string prev_command;
	static string prev_subsitude;
};

#endif /* COMMANDDIALOG_H_ */
