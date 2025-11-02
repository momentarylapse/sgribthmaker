/*
 * SettingsDialog.cpp
 *
 *  Created on: 05.06.2013
 *      Author: michi
 */

#include "SettingsDialog.h"
#include "../SourceView.h"
#include "../SgribthMakerWindow.h"
#include "../lib/hui/config.h"

SettingsDialog::SettingsDialog(SgribthMakerWindow *_mw) :
	hui::Window("settings_dialog", _mw)
{
	main_win = _mw;
	set_int("tab_width", hui::config.get_int("TabWidth", 8));
	set_string("font", hui::config.get_str("Font", "Monospace 10"));
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

	event("close", [this] { onClose(); });
	event("font", [this] { onFont(); });
	event("tab_width", [this] { onTabWidth(); });
	event_x("context_list", "hui:select", [this] { onContextListSelect(); });
	event("schemes", [this] { onSchemes(); });
	event("copy_scheme", [this] { onCopyScheme(); });
	event("scheme_background", [this] { onSchemeChange(); });
	event("color_text", [this] { onSchemeChange(); });
	event("color_background", [this] { onSchemeChange(); });
	event("overwrite_background", [this] { onSchemeChange(); });
	event("bold", [this] { onSchemeChange(); });
	event("italic", [this] { onSchemeChange(); });
}

SettingsDialog::~SettingsDialog() = default;


void SettingsDialog::onClose() {
	request_destroy();
}

string get_scheme_name(syntaxhighlight::Theme *s) {
	string n = s->name;
	if (s->is_default)
		n += _(" (write protected)");
	if (s->changed)
		n += " *";
	return n;
}

void SettingsDialog::fillSchemeList() {
	reset("schemes");
	auto schemes = syntaxhighlight::get_all_themes();
	foreachi(auto *s, schemes, i) {
		add_string("schemes", get_scheme_name(s));
		if (s == syntaxhighlight::default_theme)
			set_int("schemes", i);
	}
}

void SettingsDialog::onFont() {
	string old_font = get_string("font");
	hui::select_font(this, _("Select font"), {"font=" + old_font}).then([this] (const string &font) {
		set_string("font", font);
		hui::config.set_str("Font", font);
		for (SourceView *sv: main_win->source_views)
			sv->update_font();
	});
}

void SettingsDialog::onTabWidth() {
	hui::config.set_int("TabWidth", get_int("tab_width"));
	for (SourceView *sv: main_win->source_views)
		sv->update_tab_size();
}

void SettingsDialog::onContextListSelect() {
	int n = get_int("context_list");
	const auto *s = syntaxhighlight::default_theme;
	set_color("scheme_background", s->bg);
	const auto& c = s->context[n];
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
	auto *s = syntaxhighlight::default_theme;
	auto &c = s->context[n];
	s->bg = get_color("scheme_background");
	c.fg = get_color("color_text");
	c.bg = get_color("color_background");
	c.set_bg = is_checked("overwrite_background");
	c.bold = is_checked("bold");
	c.italic = is_checked("italic");
	s->changed = true;
	for (SourceView *sv: main_win->source_views)
		sv->apply_scheme(s);
	fillSchemeList();
}

void SettingsDialog::onSchemes() {
	int n = get_int("");
	auto s = syntaxhighlight::get_all_themes()[n];
	syntaxhighlight::default_theme = s;
	for (SourceView *sv: main_win->source_views)
		sv->apply_scheme(s);
	onContextListSelect();
}

void SettingsDialog::onCopyScheme() {
	auto *s = syntaxhighlight::default_theme->copy(_("new scheme"));
	for (SourceView *sv: main_win->source_views)
		sv->apply_scheme(s);
	fillSchemeList();
	onContextListSelect();
}

