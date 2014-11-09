/*
 * CommandDialog.h
 *
 *  Created on: 05.06.2013
 *      Author: michi
 */

#ifndef COMMANDDIALOG_H_
#define COMMANDDIALOG_H_

#include "lib/hui/hui.h"

class CommandDialog : public HuiWindow
{
public:
	CommandDialog(HuiWindow *parent);
	virtual ~CommandDialog();

	void onOk();
	void onCancel();
};

#endif /* COMMANDDIALOG_H_ */
