/*
 * Document.cpp
 *
 *  Created on: 18.02.2014
 *      Author: michi
 */

#include "Document.h"
#include "History.h"
#include "SourceView.h"
#include "SgribthMakerWindow.h"
#include "lib/os/file.h"
#include "lib/hui/hui.h"

Document::Document(SgribthMakerWindow *_win) {
	win = _win;
	parser = nullptr;
	source_view = nullptr;
	scheme = nullptr;
	buffer_dirty = false;
	history = new History(this);
}

Document::~Document() {
	delete history;
}

string simplify_path(const Path &filename) {
	string home = getenv("HOME");
	if (filename.str().head(home.num) == home)
		return "~" + filename.str().sub(home.num);
	return filename.str();
}

string Document::name(bool long_name) const {
	string r = simplify_path(filename);
	if (!long_name)
		r = filename.basename();
	if (r.num == 0)
		r = _("new document");
	if (history->changed)
		return "*" + r;
	return r;
}


bool Document::load(const Path &_filename) {
	try {
		string temp = os::fs::read_text(_filename);
		if (!source_view->fill(temp))
			win->set_message(_("File is not UTF-8 compatible"));

		filename = _filename;

		source_view->set_parser(filename);

		win->update_menu();
	} catch(...) {
		win->set_message(_("File does not want to be opened"));
		return false;
	}
	return true;
}

bool Document::save(const Path &_filename) {
	try {
		os::fs::write_text(_filename, source_view->get_all());
		filename = _filename;
		history->define_as_saved();
		//SetMessage(_("saved"));
		//UpdateMenu();
	} catch(...) {
		win->set_message(_("Can not save file"));
		return false;
	}

	source_view->set_parser(filename);
	return true;
}
