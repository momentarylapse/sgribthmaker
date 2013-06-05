/*
 * SettingsDialog.cpp
 *
 *  Created on: 05.06.2013
 *      Author: michi
 */

#include "SettingsDialog.h"

SettingsDialog::SettingsDialog(HuiWindow *parent) :
	HuiWindow("settings_dialog", parent, false)
{
	SetInt("tab_width", HuiConfigReadInt("TabWidth", 8));
	SetString("font", HuiConfigReadStr("Font", "Monospace 10"));
	AddString("context_list", _("reserviertes Wort"));
	AddString("context_list", _("Api Funktion"));
	AddString("context_list", _("Api Variable"));
	AddString("context_list", _("Typ"));
	AddString("context_list", _("Text"));
	AddString("context_list", _("Kommentar Zeile"));
	AddString("context_list", _("Kommentar Ebene 1"));
	AddString("context_list", _("Kommentar Ebene 2"));
	AddString("context_list", _("Macro"));
	AddString("context_list", _("Trennzeichen"));
	AddString("context_list", _("String"));
	AddString("context_list", _("Operator"));
	AddString("context_list", _("Zahl"));

	EventM("close", this, &SettingsDialog::OnClose);
	EventM("font", this, &SettingsDialog::OnFont);
	EventM("tab_width", this, &SettingsDialog::OnTabWidth);
}

SettingsDialog::~SettingsDialog()
{
}


void SettingsDialog::OnClose()
{
	delete(this);
}

void UpdateFont();
void UpdateTabSize();

void SettingsDialog::OnFont()
{
	if (HuiSelectFont(this, _("Font w&ahlen"))){
		SetString("font", HuiFontname);
		HuiConfigWriteStr("Font", HuiFontname);
		UpdateFont();
	}
}

void SettingsDialog::OnTabWidth()
{
	HuiConfigWriteInt("TabWidth", GetInt("tab_width"));
	UpdateTabSize();
}

