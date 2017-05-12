/*
 * SettingsDialog.cpp
 *
 *  Created on: 05.06.2013
 *      Author: michi
 */

#include "SettingsDialog.h"
#include "SourceView.h"

extern Array<SourceView*> source_view;

SettingsDialog::SettingsDialog(hui::HuiWindow *parent) :
	hui::HuiWindow("settings_dialog", parent)
{
	setInt("tab_width", hui::HuiConfig.getInt("TabWidth", 8));
	setString("font", hui::HuiConfig.getStr("Font", "Monospace 10"));
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

	event("close", std::bind(&SettingsDialog::onClose, this));
	event("font", std::bind(&SettingsDialog::onFont, this));
	event("tab_width", std::bind(&SettingsDialog::onTabWidth, this));
	eventX("context_list", "hui:select", std::bind(&SettingsDialog::onContextListSelect, this));
	event("schemes", std::bind(&SettingsDialog::onSchemes, this));
	event("copy_scheme", std::bind(&SettingsDialog::onCopyScheme, this));
	event("scheme_background", std::bind(&SettingsDialog::onSchemeChange, this));
	event("color_text", std::bind(&SettingsDialog::onSchemeChange, this));
	event("color_background", std::bind(&SettingsDialog::onSchemeChange, this));
	event("overwrite_background", std::bind(&SettingsDialog::onSchemeChange, this));
	event("bold", std::bind(&SettingsDialog::onSchemeChange, this));
	event("italic", std::bind(&SettingsDialog::onSchemeChange, this));
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
	if (hui::HuiSelectFont(this, _("Font w&ahlen"))){
		setString("font", hui::HuiFontname);
		hui::HuiConfig.setStr("Font", hui::HuiFontname);
		for (SourceView *sv: source_view)
			sv->UpdateFont();
	}
}

void SettingsDialog::onTabWidth()
{
	hui::HuiConfig.setInt("TabWidth", getInt("tab_width"));
	for (SourceView *sv: source_view)
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
	for (SourceView *sv: source_view)
		sv->ApplyScheme(s);
	fillSchemeList();
}

void SettingsDialog::onSchemes()
{
	int n = getInt("");
	HighlightScheme *s = HighlightScheme::get_all()[n];
	HighlightScheme::default_scheme = s;
	for (SourceView *sv: source_view)
		sv->ApplyScheme(s);
	onContextListSelect();
}

void SettingsDialog::onCopyScheme()
{
	HighlightScheme *s = HighlightScheme::default_scheme->copy(_("neues Schema"));
	for (SourceView *sv: source_view)
		sv->ApplyScheme(s);
	fillSchemeList();
	onContextListSelect();
}

