/*
 * SettingsDialog.cpp
 *
 *  Created on: 05.06.2013
 *      Author: michi
 */

#include "SettingsDialog.h"
#include "SourceView.h"

extern Array<SourceView*> source_view;

SettingsDialog::SettingsDialog(HuiWindow *parent) :
	HuiWindow("settings_dialog", parent, false)
{
	SetInt("tab_width", HuiConfig.getInt("TabWidth", 8));
	SetString("font", HuiConfig.getStr("Font", "Monospace 10"));
	AddString("context_list", _("Text"));
	AddString("context_list", _("reserviertes Wort"));
	AddString("context_list", _("Api Funktion"));
	AddString("context_list", _("Api Variable"));
	AddString("context_list", _("Typ"));
	AddString("context_list", _("Kommentar Zeile"));
	AddString("context_list", _("Kommentar Ebene 1"));
	AddString("context_list", _("Kommentar Ebene 2"));
	AddString("context_list", _("Macro"));
	AddString("context_list", _("Trennzeichen"));
	AddString("context_list", _("String"));
	AddString("context_list", _("Operator"));
	AddString("context_list", _("Zahl"));

	FillSchemeList();

	SetInt("context_list", 0);
	OnContextListSelect();

	EventM("close", this, &SettingsDialog::OnClose);
	EventM("font", this, &SettingsDialog::OnFont);
	EventM("tab_width", this, &SettingsDialog::OnTabWidth);
	EventMX("context_list", "hui:select", this, &SettingsDialog::OnContextListSelect);
	EventM("schemes", this, &SettingsDialog::OnSchemes);
	EventM("copy_scheme", this, &SettingsDialog::OnCopyScheme);
	EventM("scheme_background", this, &SettingsDialog::OnSchemeChange);
	EventM("color_text", this, &SettingsDialog::OnSchemeChange);
	EventM("color_background", this, &SettingsDialog::OnSchemeChange);
	EventM("overwrite_background", this, &SettingsDialog::OnSchemeChange);
	EventM("bold", this, &SettingsDialog::OnSchemeChange);
	EventM("italic", this, &SettingsDialog::OnSchemeChange);
}

SettingsDialog::~SettingsDialog()
{
}


void SettingsDialog::OnClose()
{
	delete(this);
}

string get_scheme_name(HighlightScheme *s)
{
	string n = s->name;
	if (s->is_default)
		n += _(" (schreibgesch&utzt)");
	if (s->changed)
		n += " *";
	return n;
}

void SettingsDialog::FillSchemeList()
{
	Reset("schemes");
	Array<HighlightScheme*> schemes = HighlightScheme::get_all();
	foreachi(HighlightScheme *s, schemes, i){
		AddString("schemes", get_scheme_name(s));
		if (s == HighlightScheme::default_scheme)
			SetInt("schemes", i);
	}
}

void SettingsDialog::OnFont()
{
	if (HuiSelectFont(this, _("Font w&ahlen"))){
		SetString("font", HuiFontname);
		HuiConfig.setStr("Font", HuiFontname);
		foreach(SourceView *sv, source_view)
			sv->UpdateFont();
	}
}

void SettingsDialog::OnTabWidth()
{
	HuiConfig.setInt("TabWidth", GetInt("tab_width"));
	foreach(SourceView *sv, source_view)
		sv->UpdateTabSize();
}

void SettingsDialog::OnContextListSelect()
{
	int n = GetInt("context_list");
	HighlightScheme *s = HighlightScheme::default_scheme;
	SetColor("scheme_background", s->bg);
	HighlightContext c = s->context[n];
	SetColor("color_text", c.fg);
	SetColor("color_background", c.bg);
	Check("overwrite_background", c.set_bg);
	Check("bold", c.bold);
	Check("italic", c.italic);


	Enable("scheme_background", !s->is_default);
	Enable("color_text", !s->is_default);
	Enable("overwrite_background", !s->is_default);
	Enable("color_background", !s->is_default && c.set_bg);
	Enable("bold", !s->is_default);
	Enable("italic", !s->is_default);
}

void SettingsDialog::OnSchemeChange()
{
	int n = GetInt("context_list");
	HighlightScheme *s = HighlightScheme::default_scheme;
	HighlightContext &c = s->context[n];
	s->bg = GetColor("scheme_background");
	c.fg = GetColor("color_text");
	c.bg = GetColor("color_background");
	c.set_bg = IsChecked("overwrite_background");
	c.bold = IsChecked("bold");
	c.italic = IsChecked("italic");
	s->changed = true;
	foreach(SourceView *sv, source_view)
		sv->ApplyScheme(s);
	FillSchemeList();
}

void SettingsDialog::OnSchemes()
{
	int n = GetInt("");
	HighlightScheme *s = HighlightScheme::get_all()[n];
	HighlightScheme::default_scheme = s;
	foreach(SourceView *sv, source_view)
		sv->ApplyScheme(s);
	OnContextListSelect();
}

void SettingsDialog::OnCopyScheme()
{
	HighlightScheme *s = HighlightScheme::default_scheme->copy(_("neues Schema"));
	foreach(SourceView *sv, source_view)
		sv->ApplyScheme(s);
	FillSchemeList();
	OnContextListSelect();
}

