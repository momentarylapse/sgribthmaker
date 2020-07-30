/*
 * Document.cpp
 *
 *  Created on: 18.02.2014
 *      Author: michi
 */

#include "Document.h"
#include "History.h"
#include "SourceView.h"
#include "SgribthMaker.h"
#include "lib/file/file.h"
#include "lib/hui/hui.h"

Document::Document(SgribthMaker *_sgribthmaker) {
	sgribthmaker = _sgribthmaker;
	parser = NULL;
	source_view = NULL;
	history = NULL;
	//history = new History;
	scheme = NULL;
	buffer_dirty = false;
}

Document::~Document() {
	//delete(history);
}

string simplify_path(const string &filename) {
	string home = getenv("HOME");
	if (filename.head(home.num) == home)
		return "~" + filename.substr(home.num, -1);
	return filename;
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


bool Document::load(const string &_filename) {
	try {
		string temp = FileReadText(_filename);
		if (!source_view->Fill(temp))
			sgribthmaker->SetMessage(_("File is not UTF-8 compatible"));

		filename = _filename;

		source_view->SetParser(filename);

		sgribthmaker->UpdateMenu();
	} catch(...) {
		sgribthmaker->SetMessage(_("File does not want to be opened"));
		return false;
	}
	return true;
}

bool Document::save(const string &_filename) {
	try {
		FileWriteText(_filename, source_view->GetAll());
		filename = _filename;
		history->DefineAsSaved();
		//SetMessage(_("saved"));
		//UpdateMenu();
	} catch(...) {
		sgribthmaker->SetMessage(_("Can not save file"));
		return false;
	}

	source_view->SetParser(filename);
	return true;
}
