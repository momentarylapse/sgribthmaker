/*
 * SettingsDialog.h
 *
 *  Created on: 05.06.2013
 *      Author: michi
 */

#ifndef SETTINGSDIALOG_H_
#define SETTINGSDIALOG_H_

#include "lib/hui/hui.h"

class SettingsDialog : public HuiWindow
{
public:
	SettingsDialog(HuiWindow *parent);
	virtual ~SettingsDialog();

	void fillSchemeList();

	void onClose();
	void onFont();
	void onTabWidth();
	void onSchemes();
	void onCopyScheme();
	void onSaveScheme();
	void onContextListSelect();
	void onSchemeChange();
};

#endif /* SETTINGSDIALOG_H_ */
