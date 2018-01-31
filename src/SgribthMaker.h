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


class SgribthMaker : public hui::Application
{
public:
	SgribthMaker();
	virtual bool onStartup(const Array<string> &arg);

	void OnAbout();
	void OnExit();

	void UpdateStatusBar();
	void SetMessage(const string &str);
	void SetWindowTitle();
	void UpdateDocList();
	void UpdateMenu();
	void UpdateFunctionList();
	void SetActiveDocument(Document *d);
	bool AllowTermination();
	bool AllowDocTermination(Document *d);
	void New();
	void OnCloseDocument();
	bool LoadFromFile(const string &filename);
	bool WriteToFile(Document *doc, const string &filename);
	bool Open();
	bool SaveAs(Document *doc);
	bool Save(Document *doc);
	void OnOpen();
	void OnSave();
	void OnSaveAs();
	bool Reload();
	void OnReload();
	void OnUndo();
	void OnRedo();
	void OnCopy();
	void OnPaste();
	void OnDelete();
	void OnCut();
	void CompileKaba();
	void CompileShader();
	void Compile();
	void CompileAndRun(bool verbose);
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