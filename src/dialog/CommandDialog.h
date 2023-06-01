/*
 * CommandDialog.h
 *
 *  Created on: 05.06.2013
 *      Author: michi
 */

#ifndef SRC_DIALOG_COMMANDDIALOG_H_
#define SRC_DIALOG_COMMANDDIALOG_H_

#include "../lib/hui/hui.h"

class SgribthMakerWindow;

class CommandDialog : public hui::Dialog {
public:
	CommandDialog(SgribthMakerWindow *main_win);

	void on_ok();

	SgribthMakerWindow *main_win;

	static string prev_command;
	static string prev_subsitude;
};

#endif /* SRC_DIALOG_COMMANDDIALOG_H_ */
