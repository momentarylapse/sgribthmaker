/*
 * SettingsDialog.h
 *
 *  Created on: 05.06.2013
 *      Author: michi
 */

#ifndef SRC_DIALOG_SETTINGSDIALOG_H_
#define SRC_DIALOG_SETTINGSDIALOG_H_

#include "../lib/hui/hui.h"

class SgribthMakerWindow;

class SettingsDialog : public hui::Window {
public:
	SettingsDialog(SgribthMakerWindow *main_win);
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

	SgribthMakerWindow *main_win;
};

#endif /* SRC_DIALOG_SETTINGSDIALOG_H_ */
