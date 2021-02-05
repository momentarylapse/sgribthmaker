/*
 * SgribthMaker.h
 *
 *  Created on: 14.10.2017
 *      Author: michi
 */

#ifndef SRC_SGRIBTHMAKER_H_
#define SRC_SGRIBTHMAKER_H_


#include "lib/hui/hui.h"

class Document;
class SourceView;
class Console;
class Path;


class SgribthMaker : public hui::Application {
public:
	SgribthMaker();
	bool on_startup(const Array<string> &arg) override;

	void OnAbout();
	void OnExit();

	void UpdateStatusBar();
	void SetMessage(const string &str);
	void SetError(const string &str);
	void SetInfo(const string &str);
	void SetWindowTitle();
	void UpdateDocList();
	void UpdateMenu();
	void UpdateFunctionList();
	void SetActiveDocument(Document *d);
	bool AllowTermination();
	bool AllowDocTermination(Document *d);
	void New();
	void OnCloseDocument();
	bool LoadFromFile(const Path &filename);
	bool WriteToFile(Document *doc, const Path &filename);
	bool Open();
	bool SaveAs(Document *doc);
	bool Save(Document *doc);
	void OnOpen();
	void OnSave();
	void OnSaveAs();
	bool Reload();
	void OnReload();
	void on_undo();
	void on_redo();
	void on_copy();
	void on_paste();
	void on_delete();
	void on_cut();
	void prepend_selected_lines(const string &s);
	void unprepend_selected_lines(const string &s);
	void on_comment();
	void on_uncomment();
	void on_indent();
	void on_unindent();
	void CompileKaba();
	void CompileShader();
	void Compile();
	void CompileAndRun(bool verbose);
	void OnAutoComplete();
	void OnInsertAutoComplete(int n);
	void OnCompileAndRunVerbose();
	void OnCompileAndRunSilent();
	void ShowCurLine();
	void ExecuteCommand(const string &cmd);
	void ExecuteCommandDialog();
	void ExecuteSettingsDialog();
	void OnFunctionList();
	void OnFileList();
	void OnNextDocument();
	void OnPreviousDocument();


	hui::Window *MainWin;

	Array<SourceView*> source_view;
	Console *console;

	Array<Document*> documents;
	Document *cur_doc = NULL;
};




#endif /* SRC_SGRIBTHMAKER_H_ */
