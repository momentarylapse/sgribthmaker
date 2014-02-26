#include "lib/base/base.h"
#include "lib/hui/hui.h"
#include "lib/nix/nix.h"
#include "lib/file/file.h"
#include "lib/script/script.h"
#include "SettingsDialog.h"
#include "CommandDialog.h"
#include "History.h"
#include "HighlightScheme.h"
#include "SourceView.h"
#include "Parser/BaseParser.h"
#include "Document.h"



string AppTitle = "SgribthMaker";
string AppVersion = "0.4.1.0";

#define ALLOW_LOGGING			true
//#define ALLOW_LOGGING			false

HuiWindow *MainWin;
string LastCommand;

Array<SourceView*> source_view;

extern string NixShaderError;

Array<Document*> documents;
Document *cur_doc = NULL;

//------------------------------------------------------------------------------
// Highlighting



int status_count = 0;

void UpdateStatusBar()
{
	msg_db_f("UpdateStatusBar", 2);
	status_count --;
	if (status_count == 0)
		MainWin->EnableStatusbar(false);
}

void SetMessage(const string &str)
{
	msg_db_f("SetMessage", 2);
	MainWin->SetStatusText(str);
	MainWin->EnableStatusbar(true);
	status_count ++;
	HuiRunLater(5, &UpdateStatusBar);
}

void SetWindowTitle()
{
	if (!cur_doc)
		return;
	msg_db_f("SetWinTitle", 1);
	MainWin->SetTitle(cur_doc->name(true) + " - " + AppTitle);
}

void UpdateDocList()
{
	MainWin->Reset("file_list");
	foreachi(Document *d, documents, i){
		MainWin->AddString("file_list", d->name(false));
		if (cur_doc == d)
			MainWin->SetInt("file_list", i);
	}
}

void UpdateMenu()
{
	MainWin->Enable("undo", cur_doc->history->Undoable());
	MainWin->Enable("redo", cur_doc->history->Redoable());
	MainWin->Enable("save", cur_doc->history->changed);
	UpdateDocList();
	SetWindowTitle();
}

void SetActiveDocument(Document *d)
{
	foreachi(Document *dd, documents, i)
		if (dd == d){
			MainWin->SetInt("tab", i);
			MainWin->Activate("edit" + i2s(i));
		}
	cur_doc = d;
	UpdateMenu();
}

bool Save();

bool AllowTermination()
{
	foreach(Document *d, documents)
	if (d->history->changed){
		SetActiveDocument(d);
		string answer = HuiQuestionBox(MainWin, _("dem&utige aber h&ofliche Frage"), _("Sie haben die Entropie erh&oht. Wollen Sie Ihr Werk speichern?"), true);
		if (answer == "hui:cancel")
			return false;
		if (answer == "hui:yes")
			return Save();
		return true;
	}
	return true;
}

void New()
{
	msg_db_f("New", 1);

	if (documents.num > 0)
		MainWin->HideControl("table_side", false);

	string id = "edit" + i2s(documents.num);

	MainWin->SetBorderWidth(0);
	if (documents.num > 0)
		MainWin->AddString("tab", i2s(documents.num));
	MainWin->SetTarget("tab", documents.num);
	MainWin->AddMultilineEdit("!handlekeys", 0, 0, 0, 0, id);

	documents.add(new Document);
	SourceView *sv = new SourceView(MainWin, id, documents.back());

	HighlightScheme *scheme = HighlightScheme::get(HuiConfigReadStr("HighlightScheme", "default"));
	sv->ApplyScheme(scheme);
	source_view.add(sv);

	SetActiveDocument(documents.back());
	UpdateMenu();
}

bool LoadFromFile(const string &filename)
{
	New();
	return documents.back()->load(filename);
}

bool WriteToFile(const string &filename)
{
	bool ok = cur_doc->save(filename);
	SetMessage(_("gespeichert"));
	return ok;
}

bool Open()
{
	if (HuiFileDialogOpen(MainWin, _("Datei &offnen"), cur_doc->filename.dirname(), _("Alles (*.*)"), "*"))
		return LoadFromFile(HuiFilename);
	return false;
}

bool SaveAs()
{
	if (HuiFileDialogSave(MainWin, _("Datei speichern"), cur_doc->filename.dirname(), _("Alles (*.*)"), "*"))
		return WriteToFile(HuiFilename);
	return false;
}

bool Save()
{
	if (cur_doc->filename.num > 0)
		return WriteToFile(cur_doc->filename);
	else
		return SaveAs();
}

void OnOpen()
{	Open();	}

void OnSave()
{	Save();	}

void OnSaveAs()
{	SaveAs();	}

bool Reload()
{
	if (!AllowTermination())
		return false;
	if (cur_doc->filename.num > 0){
		bool r = cur_doc->load(cur_doc->filename);
		if (r)
			SetMessage(_("neu geladen"));
		return r;
	}
	return true;
}

void OnReload()
{	Reload();	}

void OnUndo()
{	cur_doc->history->Undo();	}

void OnRedo()
{	cur_doc->history->Redo();	}

void OnCopy()
{
	HuiCopyToClipBoard(cur_doc->source_view->GetSelection());
	SetMessage(_("kopiert"));
}

void OnPaste()
{
	cur_doc->source_view->InsertAtCursor(HuiPasteFromClipBoard());
	SetMessage(_("eingef&ugt"));
}

void OnDelete()
{
	cur_doc->source_view->DeleteSelection();
}

void OnCut()
{
	OnCopy();
	OnDelete();
}

string get_time_str(float t)
{
	if (t < 0.3f)
		return format("%.2fms", t * 1000.0f);
	else
		return format("%.2fs", t);
}

void CompileKaba()
{
	msg_db_f("CompileKaba",1);

	//HuiSetDirectory(SgribthDir);
	msg_set_verbose(true);

	HuiTimer CompileTimer;

	Script::config.CompileSilently = true;

	try{
		Script::Script *compile_script = Script::Load(cur_doc->filename, true);

		float dt = CompileTimer.get();

		//compile_script->Show();

		SetMessage(format(_("Script ist fehler-frei &ubersetzbar!        (in %s)"), get_time_str(dt).c_str()));

	}catch(const Script::Exception &e){
		e.print();
		HuiErrorBox(MainWin, _("Fehler"), e.message);
		cur_doc->source_view->MoveCursorTo(e.line, e.column);
	}

	//RemoveScript(compile_script);
	Script::DeleteAllScripts(true, true);

	msg_db_m("set verbose...",1);
	msg_set_verbose(ALLOW_LOGGING);
}

void CompileShader()
{
	msg_db_f("CompileShader",1);

	HuiWindow *w = new HuiWindow("nix", -1, -1, 640, 480);
	w->AddDrawingArea("", 0, 0, 0, 0, "nix-area");
	w->Show();
	NixInit("OpenGL", w, "nix-area");

	NixShader *shader = NixLoadShader(cur_doc->filename);
	if (!shader){
		HuiErrorBox(MainWin, _("Fehler"), NixShaderError);
	}else{
		SetMessage(_("Shader ist fehler-frei &ubersetzbar!"));
		shader->unref();
	}
	delete(w);

	msg_db_m("set verbose...",1);
	msg_set_verbose(ALLOW_LOGGING);
}

void Compile()
{
	string ext = cur_doc->filename.extension();

	if (!Save())
		return;

	if (ext == "kaba")
		CompileKaba();
	else if (ext == "glsl")
		CompileShader();
	else
		SetMessage(_("nur *.kaba und *.glsl-Dateien k&onnen &ubersetzt werden!"));
}

class TestDialog : public HuiDialog
{
public:
	TestDialog(const string &title, int x, int y, HuiWindow *p, bool b) : HuiDialog(title, x, y, p, b){}
	virtual void _cdecl __delete__()
	{
		msg_write("testdia.del");
	}
};

void On__()
{
	msg_write("..........");
	delete(HuiCurWindow);
}

void CompileAndRun(bool verbose)
{
	/*HuiDialog *d = new TestDialog("test", 400, 300, MainWin, false);
	d->Event("hui:close", &On__);
	Script::VirtualTable *vt = *(Script::VirtualTable**)d;
	Script::VirtualTable *vt2 = new Script::VirtualTable[17];
	for (int i=0;i<17;i++){
		msg_write(p2s(vt[i]));
		vt2[i] = vt[i];
	}
	*(Script::VirtualTable**)d = vt2;
	vt2[0] = Script::mf(&VirtualBase::__delete_win__);
	d->Run();
	return;*/


	if (cur_doc->filename.extension() != "kaba"){
		SetMessage(_("nur *.kaba-Dateien k&onnen ausgef&uhrt werden!"));
		return;
	}

	if (!Save())
		return;

	msg_db_f("CompileAndRun",1);

	HuiSetDirectory(cur_doc->filename.dirname());
	//if (verbose)
		msg_set_verbose(true);

	// compile
	HuiTimer CompileTimer;
	Script::config.CompileSilently = true;

	try{
		Script::Script *compile_script = Script::Load(cur_doc->filename);
		float dt_compile = CompileTimer.get();

		if (!verbose)
			msg_set_verbose(true);

		int msg_size0 = msg_get_buffer_size();


		float dt_execute = 0;
		if (compile_script->syntax->FlagCompileOS)
			HuiErrorBox(MainWin, _("Fehler"), _("Script nicht ausf&uhrbar. (#os)"));
		else{
			HuiPushMainLevel();
			CompileTimer.reset();
			if (!compile_script->syntax->FlagNoExecution){
				typedef void void_func();
				void_func *f = (void_func*)compile_script->MatchFunction("main", "void", 0);
				if (f)
					f();
				//compile_script->ShowVars(false);
			}
			dt_execute = CompileTimer.get();
			HuiPopMainLevel();
		}
		
		SetMessage(format(_("Compilieren: %s         Opcode: %db         Ausf&uhren: %s"), get_time_str(dt_compile).c_str(), compile_script->OpcodeSize, get_time_str(dt_execute).c_str()));
		//if (verbose)
		//	HuiInfoBox(MainWin,"Speicher",string("nicht freigegebener Speicher des Scriptes: ",i2s(script->MemoryUsed),"b"));}

		// messages?
		int msg_size = msg_get_buffer_size();
		if (msg_size > msg_size0){
			MainWin->HideControl("table_console", false);
			MainWin->SetString("log", msg_get_buffer(msg_size - msg_size0));
		}

	}catch(const Script::Exception &e){
		e.print();
		HuiErrorBox(MainWin, _("Fehler"), e.message);
		cur_doc->source_view->MoveCursorTo(e.line, e.column);
	}
	

	//RemoveScript(compile_script);
	Script::DeleteAllScripts(true, true);

	msg_set_verbose(ALLOW_LOGGING);
}

void OnCompileAndRunVerbose()
{	CompileAndRun(true);	}

void OnCompileAndRunSilent()
{	CompileAndRun(false);	}


void ShowCurLine()
{
	msg_db_f("ShowCurLine", 1);
	int line, off;
	cur_doc->source_view->GetCurLinePos(line, off);
	SetMessage(format(_("Zeile  %d : %d"), line + 1, off + 1));
}

void ExecuteCommand(const string &cmd)
{
	msg_db_f("ExecCmd", 1);
	bool found = cur_doc->source_view->Find(cmd);
	if (!found)
		SetMessage(format(_("\"%s\" nicht gefunden"), cmd.c_str()));
}

void ExecuteCommandDialog()
{
	CommandDialog *dlg = new CommandDialog(MainWin);
	dlg->Run();
}

void ExecuteSettingsDialog()
{
	SettingsDialog *dlg = new SettingsDialog(MainWin);
	dlg->Run();
}



void OnAbout()
{	HuiAboutBox(MainWin);	}

void OnConsoleClose()
{	MainWin->HideControl("table_console", true);	}

void OnExit()
{
	if (AllowTermination()){
		int w, h;
		MainWin->GetSizeDesired(w, h);
		HuiConfigWriteInt("Window.Width", w);
		HuiConfigWriteInt("Window.Height", h);
		HuiConfigWriteBool("Window.Maximized", MainWin->IsMaximized());
		HuiEnd();
	}
}

void OnFileList()
{
	int s = MainWin->GetInt("");
	if (s >= 0)
		SetActiveDocument(documents[s]);
}

void OnNextDocument()
{
	foreachi(Document *d, documents, i)
		if (d == cur_doc){
			if (i < documents.num - 1)
				SetActiveDocument(documents[i + 1]);
			else
				SetActiveDocument(documents[0]);
			break;
		}
}

void OnPreviousDocument()
{
	foreachi(Document *d, documents, i)
		if (d == cur_doc){
			if (i > 0)
				SetActiveDocument(documents[i - 1]);
			else
				SetActiveDocument(documents.back());
			break;
		}
}

int hui_main(Array<string> arg)
{
	msg_init(true);
	msg_db_f("main",1);
	HuiInit("sgribthmaker", true, "Deutsch");
	msg_init(HuiAppDirectory + "message.txt", ALLOW_LOGGING);

	HuiSetProperty("name", AppTitle);
	HuiSetProperty("version", AppVersion);
	HuiSetProperty("comment", _("Texteditor und Kaba-Compiler"));
	HuiSetProperty("website", "http://michi.is-a-geek.org/michisoft");
	HuiSetProperty("copyright", "© 2006-2014 by MichiSoft TM");
	HuiSetProperty("author", "Michael Ankele <michi@lupina.de>");

	HuiRegisterFileType("kaba","MichiSoft Script Datei",HuiAppDirectory + "Data/kaba.ico",HuiAppFilename,"open",true);

	int width = HuiConfigReadInt("Window.Width", 800);
	int height = HuiConfigReadInt("Window.Height", 600);
	bool maximized = HuiConfigReadBool("Window.Maximized", false);

	HuiAddCommand("new", "hui:new", KEY_N + KEY_CONTROL, &New);
	//HuiAddKeyCode(HMM_NEW_HEX, KEY_F1 + 256);
	HuiAddCommand("open", "hui:open", KEY_O + KEY_CONTROL, &OnOpen);
	//HuiAddKeyCode(HMM_OPEN_HEX, KEY_F9 + 256);
	HuiAddCommand("save", "hui:save", KEY_S + KEY_CONTROL, &OnSave);
	HuiAddCommand("save_as", "hui:save_as", KEY_S + KEY_SHIFT + KEY_CONTROL, &OnSaveAs);
	HuiAddCommand("exit", "hui:quit", KEY_Q + KEY_CONTROL, &OnExit);
	//HuiAddCommand("show_data", "", KEY_D + KEY_CONTROL, &ShowData);
	HuiAddCommand("execute_command", "", KEY_E + KEY_CONTROL, &ExecuteCommandDialog);
	HuiAddCommand("find", "", KEY_F + KEY_CONTROL, &ExecuteCommandDialog);
	HuiAddCommand("cut", "hui:cut", KEY_X + KEY_CONTROL, &OnCut);
	HuiAddCommand("copy", "hui:copy", KEY_C + KEY_CONTROL, &OnCopy);
	HuiAddCommand("paste", "hui:paste", KEY_V + KEY_CONTROL, &OnPaste);
	HuiAddCommand("reload", "hui:reload", KEY_R + KEY_CONTROL, &OnReload);
	HuiAddCommand("undo", "hui:undo", KEY_Z + KEY_CONTROL, &OnUndo);
	HuiAddCommand("redo", "hui:redo", KEY_Y + KEY_CONTROL, &OnRedo);
	HuiAddKeyCode("redo", KEY_Z + KEY_SHIFT + KEY_CONTROL);
	HuiAddCommand("compile", "", KEY_F7, &Compile);
	HuiAddCommand("compile_and_run_verbose", "", KEY_F6 + KEY_CONTROL, &OnCompileAndRunVerbose);
	HuiAddCommand("compile_and_run", "", KEY_F6, &OnCompileAndRunSilent);
	HuiAddCommand("settings", "", -1, &ExecuteSettingsDialog);
	//HuiAddCommand("script_help", "hui:help", KEY_F1 + KEY_SHIFT);
	HuiAddCommand("next_document", "hui:down", KEY_PRIOR + KEY_CONTROL, &OnNextDocument);
	HuiAddCommand("prev_document", "hui:up", KEY_NEXT + KEY_CONTROL, &OnPreviousDocument);
	
	HuiAddCommand("show_cur_line", "", KEY_F2, &ShowCurLine);

	MainWin = new HuiWindow(AppTitle, -1, -1, width, height);

	MainWin->Event("about", &OnAbout);
	MainWin->Event("console_close", &OnConsoleClose);
	MainWin->Event("exit", &OnExit);
	MainWin->Event("hui:close", &OnExit);


	MainWin->SetBorderWidth(0);
	MainWin->AddControlTable("", 0, 0, 1, 2, "table_main");
	MainWin->SetTarget("table_main", 0);
	MainWin->AddControlTable("", 0, 0, 2, 1, "table_doc");
	MainWin->AddControlTable("", 0, 1, 1, 2, "table_console");
	MainWin->SetTarget("table_doc", 0);
	MainWin->AddTabControl("!nobar", 0, 0, 0, 0, "tab");
	MainWin->AddControlTable("!noexpandx,width=180", 1, 0, 1, 2, "table_side");
	MainWin->SetTarget("table_side", 0);
	MainWin->AddListView("!nobar,select-single\\file", 0, 0, 0, 0, "file_list");
	MainWin->AddExpander("Funktionen", 0, 1, 0, 0, "function_expander");
	MainWin->SetTarget("function_expander", 0);
	MainWin->AddTreeView("!nobar\\function", 0, 0, 0, 0, "function_list");
	MainWin->SetBorderWidth(5);
	MainWin->SetTarget("table_console", 0);
	MainWin->AddMultilineEdit("", 0, 0, 0, 0, "log");
	MainWin->Enable("log", false);
	MainWin->AddControlTable("!noexpandy", 0, 1, 3, 1, "table_command");
	MainWin->SetTarget("table_command", 0);
	MainWin->AddEdit("!expandx", 0, 0, 0, 0, "console");
	MainWin->AddButton("OK", 1, 0, 0, 0, "console_ok");
	MainWin->SetImage("console_ok", "hui:ok");
	MainWin->AddButton("", 2, 0, 0, 0, "console_close");
	MainWin->SetImage("console_close", "hui:close");
	MainWin->HideControl("table_console", true);
	MainWin->HideControl("table_side", true);

	MainWin->toolbar[0]->Enable(true);
	MainWin->toolbar[0]->AddItem("", "hui:new", "new");
	MainWin->toolbar[0]->AddItem("", "hui:open", "open");
	MainWin->toolbar[0]->AddItem("", "hui:save", "save");
	MainWin->toolbar[0]->AddSeparator();
	MainWin->toolbar[0]->AddItem("", "hui:undo", "undo");
	MainWin->toolbar[0]->AddItem("", "hui:redo", "redo");
	MainWin->toolbar[0]->AddSeparator();
	MainWin->toolbar[0]->AddItem("", "hui:find", "compile");
	MainWin->toolbar[0]->AddItem("", "hui:media-play", "compile_and_run");

	MainWin->SetTooltip("new", _("neue Datei"));
	MainWin->SetTooltip("open", _("eine Datei &offnen"));
	MainWin->SetTooltip("save", _("Datei speichern"));

	InitParser();

	MainWin->SetMenu(HuiCreateResourceMenu("menu"));
	MainWin->SetMaximized(maximized);
	MainWin->Show();

	MainWin->Event("file_list", &OnFileList);


	Script::Init();

	//msg_write(Asm::Disassemble((void*)&TestTest));

	if (arg.num > 1)
		LoadFromFile(arg[1]);
	else
		New();

	return HuiRun();
}

