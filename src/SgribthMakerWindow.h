/*
 * SgribthMakerWindow.h
 *
 *  Created on: 01.06.2023
 *      Author: michi
 */

#ifndef SRC_SGRIBTHMAKERWINDOW_H_
#define SRC_SGRIBTHMAKERWINDOW_H_

#include "lib/hui/hui.h"
#include "lib/pattern/Observable.h"

class Document;
class SourceView;
class Console;
class Path;

class SgribthMakerWindow : public obs::Node<hui::Window> {
public:
	SgribthMakerWindow();

	obs::Sink in_update;


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
	void AllowTermination(const hui::Callback &on_success = nullptr, const hui::Callback &on_fail = nullptr);
	void AllowDocTermination(Document *d, const hui::Callback &on_success = nullptr, const hui::Callback &on_fail = nullptr);
	void New();
	void OnCloseDocument();
	bool LoadFromFile(const Path &filename);
	bool WriteToFile(Document *doc, const Path &filename);
	void Open();

	void SaveAs(Document *doc, const hui::Callback &on_success = nullptr, const hui::Callback &on_fail = nullptr);
	void Save(Document *doc, const hui::Callback &on_success = nullptr, const hui::Callback &on_fail = nullptr);
	void OnOpen();
	void OnSave();
	void OnSaveAs();
	void Reload();
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


	Array<SourceView*> source_view;
	Console *console;

	Array<Document*> documents;
	Document *cur_doc = nullptr;

	int status_count = -1;
};

#endif /* SRC_SGRIBTHMAKERWINDOW_H_ */
