#include "lib/base/base.h"
#include "lib/hui/hui.h"
#include "lib/nix/nix.h"
#include "lib/file/file.h"
#include "lib/script/script.h"
#include "SettingsDialog.h"
#include "CommandDialog.h"
#include "History.h"
#include "HighlightSchema.h"
#include "SourceView.h"
#include "Parser/BaseParser.h"


string Filename;


string AppTitle = "SgribthMaker";
string AppVersion = "0.3.27.0";

#define ALLOW_LOGGING			true
//#define ALLOW_LOGGING			false

HuiWindow *MainWin;
string LastCommand;

SourceView *source_view;

extern string NixShaderError;

//------------------------------------------------------------------------------
// Highlighting


Array<HighlightSchema> HighlightSchemas;


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
	msg_db_f("SetWinTitle", 1);
	if (source_view->history->changed){
		if (Filename.num > 0)
			MainWin->SetTitle("*" + Filename + " - " + AppTitle);
		else
			MainWin->SetTitle("*neues Dokument - " + AppTitle);
	}else{
		if (Filename.num)
			MainWin->SetTitle(Filename + " - " + AppTitle);
		else
			MainWin->SetTitle("neues Dokument - " + AppTitle);
	}
}

void UpdateMenu()
{
	MainWin->Enable("undo", source_view->Undoable());
	MainWin->Enable("redo", source_view->Redoable());
	SetWindowTitle();
}

bool Save();

bool AllowTermination()
{
	if (source_view->history->changed){
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
	if (!AllowTermination())
		return;

	msg_db_f("New", 1);
	source_view->Clear();

	Filename = "";
	SetWindowTitle();
}

bool LoadFromFile(const string &filename)
{
	msg_db_f("LoadFromFile", 1);
	CFile *f = FileOpen(filename);
	if (!f){
		SetMessage(_("Datei l&asst sich nicht &offnen"));
		return false;
	}
	
	source_view->Clear();

	string temp = f->ReadComplete();
	FileClose(f);

	if (!source_view->Fill(temp))
		SetMessage(_("Datei nicht UTF-8 kompatibel"));

	Filename = filename;

	source_view->SetParser(Filename);

	SetWindowTitle();
	return true;
}

bool WriteToFile(const string &filename)
{
	msg_db_f("WriteToFile", 1);
	CFile *f = FileCreate(filename);
	string temp = source_view->GetAll();
	f->WriteBuffer(temp.data, temp.num);
	FileClose(f);

	Filename = filename;
	source_view->history->DefineAsSaved();
	source_view->SetParser(Filename);

	SetMessage(_("gespeichert"));
	return true;
}

bool Open()
{
	if (!AllowTermination())
		return false;

	if (HuiFileDialogOpen(MainWin, _("Datei &offnen"), Filename.dirname(), _("Alles (*.*)"), "*"))
		return LoadFromFile(HuiFilename);
	return false;
}

bool SaveAs()
{
	if (HuiFileDialogSave(MainWin, _("Datei speichern"), Filename.dirname(), _("Alles (*.*)"), "*"))
		return WriteToFile(HuiFilename);
	return false;
}

bool Save()
{
	if (Filename.num > 0)
		return WriteToFile(Filename);
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
	if (Filename.num > 0){
		bool r = LoadFromFile(Filename);
		if (r)
			SetMessage(_("neu geladen"));
		return r;
	}
	return true;
}

void OnReload()
{	Reload();	}

void OnUndo()
{	source_view->Undo();	}

void OnRedo()
{	source_view->Redo();	}

void OnCopy()
{
	HuiCopyToClipBoard(source_view->GetSelection());
	SetMessage(_("kopiert"));
}

void OnPaste()
{
	source_view->InsertAtCursor(HuiPasteFromClipBoard());
	SetMessage(_("eingef&ugt"));
}

void OnDelete()
{
	source_view->DeleteSelection();
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
		Script::Script *compile_script = Script::Load(Filename, true);

		float dt = CompileTimer.get();

		//compile_script->Show();

		SetMessage(format(_("Script ist fehler-frei &ubersetzbar!        (in %s)"), get_time_str(dt).c_str()));

	}catch(const Script::Exception &e){
		e.print();
		HuiErrorBox(MainWin, _("Fehler"), e.message);
		source_view->MoveCursorTo(e.line, e.column);
	}

	//RemoveScript(compile_script);
	Script::DeleteAllScripts(true, true);

	msg_db_m("set verbose...",1);
	msg_set_verbose(ALLOW_LOGGING);
}

void CompileShader()
{
	msg_db_f("CompileShader",1);

	HuiWindow *w = new HuiNixWindow("nix", -1, -1, 640, 480);
	w->Show();
	NixInit("OpenGL", 640, 480, 32, false, w);

	int shader = NixLoadShader(Filename);
	if (shader < 0){
		HuiErrorBox(MainWin, _("Fehler"), NixShaderError);
	}else{
		SetMessage(_("Shader ist fehler-frei &ubersetzbar!"));
		NixUnrefShader(shader);
	}
	delete(w);

	msg_db_m("set verbose...",1);
	msg_set_verbose(ALLOW_LOGGING);
}

void Compile()
{
	string ext = Filename.extension();

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


	if (Filename.extension() != "kaba"){
		SetMessage(_("nur *.kaba-Dateien k&onnen ausgef&uhrt werden!"));
		return;
	}

	if (!Save())
		return;

	msg_db_f("CompileAndRun",1);

	HuiSetDirectory(Filename);
	//if (verbose)
		msg_set_verbose(true);

	// compile
	HuiTimer CompileTimer;
	Script::config.CompileSilently = true;

	try{
		Script::Script *compile_script = Script::Load(Filename);
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
		source_view->MoveCursorTo(e.line, e.column);
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
	source_view->GetCurLinePos(line, off);
	SetMessage(format(_("Zeile  %d : %d"), line + 1, off + 1));
}

void ExecuteCommand(const string &cmd)
{
	msg_db_f("ExecCmd", 1);
	bool found = source_view->Find(cmd);
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
		irect r=MainWin->GetOuteriorDesired();
		HuiConfigWriteInt("Width",r.x2-r.x1);
		HuiConfigWriteInt("Height",r.y2-r.y1);
		HuiConfigWriteInt("X",r.x1);
		HuiConfigWriteInt("Y",r.y1);
		HuiConfigWriteBool("Maximized",MainWin->IsMaximized());
		HuiEnd();
	}
}

int aaa = 0;

class TestClass
{
public:
	int i;
	virtual ~TestClass(){}
	virtual void _cdecl f0(){ msg_write("TestClass.f0"); }
	virtual void _cdecl f1(){ msg_write("TestClass.f1"); }
	virtual void _cdecl f2(){ msg_write("TestClass.f2"); }
	virtual void _cdecl f(int x){ msg_write("TestClass.f"); }
	void _cdecl g(int x){ i = x; msg_write("TestClass.g"); }
	void _cdecl g2(int x){ i = x; }
};

class TestDerivedClass : public TestClass
{
public:
	virtual ~TestDerivedClass(){}
	virtual void _cdecl f(int x){ msg_write("TestDerivedClass.f"); }
	void _cdecl g(int x){ msg_write("TestDerivedClass.g"); }
};

typedef void (_cdecl TestClass::*tmf)();

void _cdecl TestTest(int x, TestClass *w, tmf p)
{
	aaa = 13;
	//(w->*p)();
	//string s;
	w->f(x);
}

class TTest : public VirtualBase
{
public:
	virtual ~TTest(){ msg_write("TTest.del");	}
};

void kabadel(VirtualBase *b)
{
	b->__delete__();
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
	HuiSetProperty("copyright", "© 2006-2013 by MichiSoft TM");
	HuiSetProperty("author", "Michael Ankele <michi@lupina.de>");

	HuiRegisterFileType("kaba","MichiSoft Script Datei",HuiAppDirectory + "Data/kaba.ico",HuiAppFilename,"open",true);

	int width = HuiConfigReadInt("Width", 800);
	int height = HuiConfigReadInt("Height", 600);
	int x = HuiConfigReadInt("X", 0);
	int y = HuiConfigReadInt("Y", 0);
	bool maximized = HuiConfigReadBool("Maximized", false);

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
	
	HuiAddCommand("show_cur_line", "", KEY_F2, &ShowCurLine);

	MainWin = new HuiWindow(AppTitle, -1, -1, width, height);
	MainWin->AllowEvents("key");

	MainWin->Event("about", &OnAbout);
	MainWin->Event("console_close", &OnConsoleClose);
	MainWin->Event("exit", &OnExit);
	MainWin->Event("hui:close", &OnExit);


	MainWin->SetBorderWidth(0);
	MainWin->AddControlTable("", 0, 0, 1, 2, "table_main");
	MainWin->SetTarget("table_main", 0);
	MainWin->AddMultilineEdit("", 0, 0, 0, 0, "edit");
	MainWin->AddControlTable("", 0, 1, 1, 2, "table_console");
	MainWin->SetTarget("table_console", 0);
	MainWin->AddMultilineEdit("", 0, 0, 0, 0, "log");
	MainWin->Enable("log", false);
	MainWin->AddControlTable("!noexpandy", 0, 1, 3, 1, "table_command");
	MainWin->SetTarget("table_command", 0);
	MainWin->AddEdit("", 0, 0, 0, 0, "console");
	MainWin->AddButton("!noexpandx\\Ok", 1, 0, 0, 0, "console_ok");
	MainWin->SetImage("console_ok", "hui:ok");
	MainWin->AddButton("", 2, 0, 0, 0, "console_close");
	MainWin->SetImage("console_close", "hui:close");
	MainWin->HideControl("table_console", true);

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
	source_view = new SourceView(MainWin, "edit");
	MainWin->Activate("edit");

	HighlightSchema schema = GetDefaultSchema();
	HighlightSchemas.add(schema);
	schema.apply(source_view);

	MainWin->SetMenu(HuiCreateResourceMenu("menu"));
	MainWin->SetMaximized(maximized);
	MainWin->Show();

	Script::Init();

//	msg_write(Asm::Disassemble(&TestTest));
//	msg_write(Asm::Disassemble(Script::mf(&TestClass::g2)));
	
	/*tmf p = &TestClass::f;
	msg_write(string((char*)&p, sizeof(p)).hex());
	p = &TestClass::g;
	msg_write(string((char*)&p, sizeof(p)).hex());

	TestDerivedClass w;
	TestTest(&w, &TestClass::f);
	TestTest(&w, &TestClass::g);

	union{
		tmf mp;
		void *p;
	} pp;
	pp.mp = &TestClass::f;
	msg_write(Asm::Disassemble(pp.p));

	return 0;*/


	New();

	if (arg.num > 1)
		LoadFromFile(arg[1]);

	return HuiRun();
}

