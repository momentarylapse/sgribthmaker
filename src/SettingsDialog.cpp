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
	setInt("tab_width", HuiConfig.getInt("TabWidth", 8));
	setString("font", HuiConfig.getStr("Font", "Monospace 10"));
	addString("context_list", _("Text"));
	addString("context_list", _("reserviertes Wort"));
	addString("context_list", _("Api Funktion"));
	addString("context_list", _("Api Variable"));
	addString("context_list", _("Typ"));
	addString("context_list", _("Kommentar Zeile"));
	addString("context_list", _("Kommentar Ebene 1"));
	addString("context_list", _("Kommentar Ebene 2"));
	addString("context_list", _("Macro"));
	addString("context_list", _("Trennzeichen"));
	addString("context_list", _("String"));
	addString("context_list", _("Operator"));
	addString("context_list", _("Zahl"));

	fillSchemeList();

	setInt("context_list", 0);
	onContextListSelect();

	event("close", this, &SettingsDialog::onClose);
	event("font", this, &SettingsDialog::onFont);
	event("tab_width", this, &SettingsDialog::onTabWidth);
	eventX("context_list", "hui:select", this, &SettingsDialog::onContextListSelect);
	event("schemes", this, &SettingsDialog::onSchemes);
	event("copy_scheme", this, &SettingsDialog::onCopyScheme);
	event("scheme_background", this, &SettingsDialog::onSchemeChange);
	event("color_text", this, &SettingsDialog::onSchemeChange);
	event("color_background", this, &SettingsDialog::onSchemeChange);
	event("overwrite_background", this, &SettingsDialog::onSchemeChange);
	event("bold", this, &SettingsDialog::onSchemeChange);
	event("italic", this, &SettingsDialog::onSchemeChange);
}

SettingsDialog::~SettingsDialog()
{
}


void SettingsDialog::onClose()
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

void SettingsDialog::fillSchemeList()
{
	reset("schemes");
	Array<HighlightScheme*> schemes = HighlightScheme::get_all();
	foreachi(HighlightScheme *s, schemes, i){
		addString("schemes", get_scheme_name(s));
		if (s == HighlightScheme::default_scheme)
			setInt("schemes", i);
	}
}

void SettingsDialog::onFont()
{
	if (HuiSelectFont(this, _("Font w&ahlen"))){
		setString("font", HuiFontname);
		HuiConfig.setStr("Font", HuiFontname);
		foreach(SourceView *sv, source_view)
			sv->UpdateFont();
	}
}

void SettingsDialog::onTabWidth()
{
	HuiConfig.setInt("TabWidth", getInt("tab_width"));
	foreach(SourceView *sv, source_view)
		sv->UpdateTabSize();
}

void SettingsDialog::onContextListSelect()
{
	int n = getInt("context_list");
	HighlightScheme *s = HighlightScheme::default_scheme;
	setColor("scheme_background", s->bg);
	HighlightContext c = s->context[n];
	setColor("color_text", c.fg);
	setColor("color_background", c.bg);
	check("overwrite_background", c.set_bg);
	check("bold", c.bold);
	check("italic", c.italic);


	enable("scheme_background", !s->is_default);
	enable("color_text", !s->is_default);
	enable("overwrite_background", !s->is_default);
	enable("color_background", !s->is_default && c.set_bg);
	enable("bold", !s->is_default);
	enable("italic", !s->is_default);
}

void SettingsDialog::onSchemeChange()
{
	int n = getInt("context_list");
	HighlightScheme *s = HighlightScheme::default_scheme;
	HighlightContext &c = s->context[n];
	s->bg = getColor("scheme_background");
	c.fg = getColor("color_text");
	c.bg = getColor("color_background");
	c.set_bg = isChecked("overwrite_background");
	c.bold = isChecked("bold");
	c.italic = isChecked("italic");
	s->changed = true;
	foreach(SourceView *sv, source_view)
		sv->ApplyScheme(s);
	fillSchemeList();
}

void SettingsDialog::onSchemes()
{
	int n = getInt("");
	HighlightScheme *s = HighlightScheme::get_all()[n];
	HighlightScheme::default_scheme = s;
	foreach(SourceView *sv, source_view)
		sv->ApplyScheme(s);
	onContextListSelect();
}

void SettingsDialog::onCopyScheme()
{
	HighlightScheme *s = HighlightScheme::default_scheme->copy(_("neues Schema"));
	foreach(SourceView *sv, source_view)
		sv->ApplyScheme(s);
	fillSchemeList();
	onContextListSelect();
}

