/*
 * SettingsDialog.cpp
 *
 *  Created on: 05.06.2013
 *      Author: michi
 */

#include "SettingsDialog.h"
#include "SourceView.h"
#include "SgribthMaker.h"

SettingsDialog::SettingsDialog(SgribthMaker *_sgribthmaker) :
	hui::Window("settings_dialog", _sgribthmaker->MainWin)
{
	sgribthmaker = _sgribthmaker;
	set_int("tab_width", hui::Config.get_int("TabWidth", 8));
	set_string("font", hui::Config.get_str("Font", "Monospace 10"));
	add_string("context_list", _("Text"));
	add_string("context_list", _("reserved word"));
	add_string("context_list", _("Api function"));
	add_string("context_list", _("Api variable"));
	add_string("context_list", _("Type"));
	add_string("context_list", _("Comment line"));
	add_string("context_list", _("Comment level 1"));
	add_string("context_list", _("Comment level 2"));
	add_string("context_list", _("Macro"));
	add_string("context_list", _("Separator"));
	add_string("context_list", _("String"));
	add_string("context_list", _("Operator"));
	add_string("context_list", _("Number"));

	fillSchemeList();

	set_int("context_list", 0);
	onContextListSelect();

	event("close", std::bind(&SettingsDialog::onClose, this));
	event("font", std::bind(&SettingsDialog::onFont, this));
	event("tab_width", std::bind(&SettingsDialog::onTabWidth, this));
	event_x("context_list", "hui:select", std::bind(&SettingsDialog::onContextListSelect, this));
	event("schemes", std::bind(&SettingsDialog::onSchemes, this));
	event("copy_scheme", std::bind(&SettingsDialog::onCopyScheme, this));
	event("scheme_background", std::bind(&SettingsDialog::onSchemeChange, this));
	event("color_text", std::bind(&SettingsDialog::onSchemeChange, this));
	event("color_background", std::bind(&SettingsDialog::onSchemeChange, this));
	event("overwrite_background", std::bind(&SettingsDialog::onSchemeChange, this));
	event("bold", std::bind(&SettingsDialog::onSchemeChange, this));
	event("italic", std::bind(&SettingsDialog::onSchemeChange, this));
}

SettingsDialog::~SettingsDialog() {
}


void SettingsDialog::onClose() {
	request_destroy();
}

string get_scheme_name(HighlightScheme *s) {
	string n = s->name;
	if (s->is_default)
		n += _(" (write protected)");
	if (s->changed)
		n += " *";
	return n;
}

void SettingsDialog::fillSchemeList() {
	reset("schemes");
	Array<HighlightScheme*> schemes = HighlightScheme::get_all();
	foreachi(HighlightScheme *s, schemes, i) {
		add_string("schemes", get_scheme_name(s));
		if (s == HighlightScheme::default_scheme)
			set_int("schemes", i);
	}
}

void SettingsDialog::onFont() {
	if (hui::SelectFont(this, _("Select font"))) {
		set_string("font", hui::Fontname);
		hui::Config.set_str("Font", hui::Fontname);
		for (SourceView *sv: sgribthmaker->source_view)
			sv->update_font();
	}
}

void SettingsDialog::onTabWidth() {
	hui::Config.set_int("TabWidth", get_int("tab_width"));
	for (SourceView *sv: sgribthmaker->source_view)
		sv->update_tab_size();
}

void SettingsDialog::onContextListSelect() {
	int n = get_int("context_list");
	HighlightScheme *s = HighlightScheme::default_scheme;
	set_color("scheme_background", s->bg);
	HighlightContext c = s->context[n];
	set_color("color_text", c.fg);
	set_color("color_background", c.bg);
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

void SettingsDialog::onSchemeChange() {
	int n = get_int("context_list");
	HighlightScheme *s = HighlightScheme::default_scheme;
	HighlightContext &c = s->context[n];
	s->bg = get_color("scheme_background");
	c.fg = get_color("color_text");
	c.bg = get_color("color_background");
	c.set_bg = is_checked("overwrite_background");
	c.bold = is_checked("bold");
	c.italic = is_checked("italic");
	s->changed = true;
	for (SourceView *sv: sgribthmaker->source_view)
		sv->apply_scheme(s);
	fillSchemeList();
}

void SettingsDialog::onSchemes() {
	int n = get_int("");
	HighlightScheme *s = HighlightScheme::get_all()[n];
	HighlightScheme::default_scheme = s;
	for (SourceView *sv: sgribthmaker->source_view)
		sv->apply_scheme(s);
	onContextListSelect();
}

void SettingsDialog::onCopyScheme() {
	HighlightScheme *s = HighlightScheme::default_scheme->copy(_("new scheme"));
	for (SourceView *sv: sgribthmaker->source_view)
		sv->apply_scheme(s);
	fillSchemeList();
	onContextListSelect();
}

