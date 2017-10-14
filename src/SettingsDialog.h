/*
 * SettingsDialog.h
 *
 *  Created on: 05.06.2013
 *      Author: michi
 */

#ifndef SETTINGSDIALOG_H_
#define SETTINGSDIALOG_H_

#include "lib/hui/hui.h"

class SgribthMaker;

class SettingsDialog : public hui::Window
{
public:
	SettingsDialog(SgribthMaker *sgribthmaker);
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

	SgribthMaker *sgribthmaker;
};

#endif /* SETTINGSDIALOG_H_ */
