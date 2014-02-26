/*
 * Document.cpp
 *
 *  Created on: 18.02.2014
 *      Author: michi
 */

#include "Document.h"
#include "History.h"
#include "SourceView.h"
#include "lib/file/file.h"
#include "lib/hui/hui.h"

void SetMessage(const string &);
void UpdateMenu();

Document::Document()
{
	parser = NULL;
	source_view = NULL;
	history = NULL;
	//history = new History;
}

Document::~Document()
{
	delete(history);
}

string Document::name(bool long_name) const
{
	string r = filename;
	if (!long_name)
		r = filename.basename();
	if (r.num == 0)
		r = "neues Dokument";
	if (history->changed)
		return "*" + r;
	return r;
}


bool Document::load(const string &_filename)
{
	msg_db_f("LoadFromFile", 1);
	CFile *f = FileOpen(_filename);
	if (!f){
		SetMessage(_("Datei l&asst sich nicht &offnen"));
		return false;
	}

	string temp = f->ReadComplete();
	FileClose(f);

	if (!source_view->Fill(temp))
		SetMessage(_("Datei nicht UTF-8 kompatibel"));

	filename = _filename;

	source_view->SetParser(filename);

	UpdateMenu();
	return true;
}

bool Document::save(const string &_filename)
{
	msg_db_f("Document.save", 1);
	CFile *f = FileCreate(_filename);
	string temp = source_view->GetAll();
	f->WriteBuffer(temp.data, temp.num);
	FileClose(f);

	filename = _filename;
	history->DefineAsSaved();
	source_view->SetParser(filename);

	//SetMessage(_("gespeichert"));
	//UpdateMenu();
	return true;
}
