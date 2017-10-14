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

class CommandDialog : public hui::Window
{
public:
	CommandDialog(SgribthMaker *sgribthmaker);
	virtual ~CommandDialog();

	void onOk();
	void onCancel();

	SgribthMaker *sgribthmaker;
	string last_command;
};

#endif /* COMMANDDIALOG_H_ */
