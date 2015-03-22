#include "lib/base/base.h"
#include "lib/hui/hui.h"
#include "lib/nix/nix.h"
#include "lib/file/file.h"
#include "lib/script/script.h"
#include "SettingsDialog.h"
#include "CommandDialog.h"
#include "Console.h"
#include "History.h"
#include "HighlightScheme.h"
#include "SourceView.h"
#include "Parser/BaseParser.h"
#include "Document.h"


	#include <sys/mman.h>



string AppTitle = "SgribthMaker";
string AppVersion = "0.4.3.2";

#define ALLOW_LOGGING			true
//#define ALLOW_LOGGING			false

HuiWindow *MainWin;
string LastCommand;

Array<SourceView*> source_view;
Console *console;

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
		MainWin->enableStatusbar(false);
}

void SetMessage(const string &str)
{
	msg_db_f("SetMessage", 2);
	MainWin->setStatusText(str);
	MainWin->enableStatusbar(true);
	status_count ++;
	HuiRunLater(5, &UpdateStatusBar);
}

void SetWindowTitle()
{
	if (!cur_doc)
		return;
	msg_db_f("SetWinTitle", 1);
	MainWin->setTitle(cur_doc->name(true) + " - " + AppTitle);
}

void UpdateDocList()
{
	MainWin->reset("file_list");
	foreachi(Document *d, documents, i){
		MainWin->addString("file_list", d->name(false));
		if (cur_doc == d)
			MainWin->setInt("file_list", i);
	}
}

void UpdateMenu()
{
	MainWin->enable("undo", cur_doc->history->Undoable());
	MainWin->enable("redo", cur_doc->history->Redoable());
	MainWin->enable("save", cur_doc->history->changed);
	UpdateDocList();
	SetWindowTitle();
}

void UpdateFunctionList()
{
	MainWin->reset("function_list");
	if (!cur_doc->parser)
		return;
	Array<Parser::Label> labels = cur_doc->parser->FindLabels(cur_doc->source_view);
	int last_parent = -1;
	foreachi(Parser::Label &l, labels, i){
		if (l.level > 0){
			MainWin->addChildString("function_list", last_parent, l.name);
		}else{
			last_parent = i;
			MainWin->addString("function_list", l.name);
		}
	}
}

void SetActiveDocument(Document *d)
{
	foreachi(Document *dd, documents, i)
		if (dd == d){
			MainWin->setInt("tab", i);
			MainWin->activate("edit" + i2s(i));
			break;
		}
	cur_doc = d;
	UpdateMenu();
	UpdateFunctionList();
}

bool Save(Document *doc);

bool AllowTermination()
{
	foreach(Document *d, documents)
	if (d->history->changed){
		SetActiveDocument(d);
		string answer = HuiQuestionBox(MainWin, _("dem&utige aber h&ofliche Frage"), _("Sie haben die Entropie erh&oht. Wollen Sie Ihr Werk speichern?"), true);
		if (answer == "hui:cancel")
			return false;
		if (answer == "hui:yes")
			return Save(d);
		return true;
	}
	return true;
}

void New()
{
	msg_db_f("New", 1);

	if (documents.num > 0)
		MainWin->hideControl("table_side", false);

	string id = "edit" + i2s(documents.num);

	MainWin->setBorderWidth(0);
	if (documents.num > 0)
		MainWin->addString("tab", i2s(documents.num));
	MainWin->setTarget("tab", documents.num);
	MainWin->addMultilineEdit("!handlekeys,noframe", 0, 0, 0, 0, id);

	documents.add(new Document);
	SourceView *sv = new SourceView(MainWin, id, documents.back());

	sv->ApplyScheme(HighlightScheme::default_scheme);
	source_view.add(sv);

	SetActiveDocument(documents.back());
	UpdateMenu();
}

bool LoadFromFile(const string &filename)
{
	New();
	return documents.back()->load(filename);
}

bool WriteToFile(Document *doc, const string &filename)
{
	bool ok = doc->save(filename);
	if (ok)
		SetMessage(_("gespeichert"));
	return ok;
}

bool Open()
{
	if (HuiFileDialogOpen(MainWin, _("Datei &offnen"), cur_doc->filename.dirname(), _("Alles (*.*)"), "*"))
		return LoadFromFile(HuiFilename);
	return false;
}

bool SaveAs(Document *doc)
{
	if (HuiFileDialogSave(MainWin, _("Datei speichern"), doc->filename.dirname(), _("Alles (*.*)"), "*"))
		return WriteToFile(doc, HuiFilename);
	return false;
}

bool Save(Document *doc)
{
	if (doc->filename.num > 0)
		return WriteToFile(doc, doc->filename);
	else
		return SaveAs(doc);
}

void OnOpen()
{	Open();	}

void OnSave()
{	Save(cur_doc);	}

void OnSaveAs()
{	SaveAs(cur_doc);	}

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
	w->addDrawingArea("", 0, 0, 0, 0, "nix-area");
	w->show();
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

	if (!Save(cur_doc))
		return;

	if (ext == "kaba")
		CompileKaba();
	else if (ext == "glsl")
		CompileShader();
	else
		SetMessage(_("nur *.kaba und *.glsl-Dateien k&onnen &ubersetzt werden!"));
}

void CompileAndRun(bool verbose)
{
	if (cur_doc->filename.extension() != "kaba"){
		SetMessage(_("nur *.kaba-Dateien k&onnen ausgef&uhrt werden!"));
		return;
	}

	if (!Save(cur_doc))
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
		if (msg_size > msg_size0)
			console->set(msg_get_buffer(msg_size - msg_size0));

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
	dlg->run();
}

void ExecuteSettingsDialog()
{
	SettingsDialog *dlg = new SettingsDialog(MainWin);
	dlg->run();
}



void OnAbout()
{	HuiAboutBox(MainWin);	}

void OnExit()
{
	if (AllowTermination()){
		int w, h;
		MainWin->getSizeDesired(w, h);
		HuiConfig.setInt("Window.Width", w);
		HuiConfig.setInt("Window.Height", h);
		HuiConfig.setBool("Window.Maximized", MainWin->isMaximized());
		HuiEnd();
	}
}

void OnFunctionList()
{
	int n = MainWin->getInt("");
	Array<Parser::Label> labels = cur_doc->parser->FindLabels(cur_doc->source_view);
	if ((n >= 0) && (n < labels.num)){
		cur_doc->source_view->ShowLineOnScreen(labels[n].line);
		MainWin->activate(cur_doc->source_view->id);
	}
}

void OnFileList()
{
	int s = MainWin->getInt("");
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

class SgribthMaker : public HuiApplication
{
public:
	SgribthMaker() :
		HuiApplication("sgribthmaker", "Deutsch", HUI_FLAG_LOAD_RESOURCE | HUI_FLAG_SILENT)
	{
		HuiSetProperty("name", AppTitle);
		HuiSetProperty("version", AppVersion);
		HuiSetProperty("comment", _("Texteditor und Kaba-Compiler"));
		HuiSetProperty("website", "http://michi.is-a-geek.org/michisoft");
		HuiSetProperty("copyright", "© 2006-2015 by MichiSoft TM");
		HuiSetProperty("author", "Michael Ankele <michi@lupina.de>");

		HuiRegisterFileType("kaba","MichiSoft Script Datei",HuiAppDirectory + "Data/kaba.ico",HuiAppFilename,"open",true);

		Script::Init(Asm::INSTRUCTION_SET_ARM);
	}

	virtual bool onStartup(const Array<string> &arg)
	{
		int *fff = (int*)mmap(0, 100, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED | MAP_ANONYMOUS | MAP_EXECUTABLE, -1, 0);
		typedef int ifii(int, int);
		ifii *fp = (ifii*)fff;



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


		int width = HuiConfig.getInt("Window.Width", 800);
		int height = HuiConfig.getInt("Window.Height", 600);
		bool maximized = HuiConfig.getBool("Window.Maximized", false);

		MainWin = new HuiWindow(AppTitle, -1, -1, width, height);

		MainWin->eventS("about", &OnAbout);
		MainWin->eventS("exit", &OnExit);
		MainWin->eventS("hui:close", &OnExit);


		MainWin->setBorderWidth(0);
		MainWin->setIndent(0);
		MainWin->addControlTable("", 0, 0, 1, 2, "table_main");
		MainWin->setTarget("table_main", 0);
		MainWin->addControlTable("", 0, 0, 2, 1, "table_doc");
		MainWin->setTarget("table_doc", 0);
		MainWin->addTabControl("!nobar", 0, 0, 0, 0, "tab");
		MainWin->addControlTable("!noexpandx,width=180", 1, 0, 1, 2, "table_side");
		MainWin->setTarget("table_side", 0);
		MainWin->addGroup("Dokumente", 0, 0, 0, 0, "group_files");
		MainWin->addExpander("Funktionen", 0, 1, 0, 0, "function_expander");
		MainWin->setTarget("group_files", 0);
		MainWin->addListView("!nobar,select-single\\file", 0, 0, 0, 0, "file_list");
		MainWin->setTarget("function_expander", 0);
		MainWin->addTreeView("!nobar\\function", 0, 0, 0, 0, "function_list");
		MainWin->setBorderWidth(5);
		MainWin->hideControl("table_side", true);

		console = new Console;
		MainWin->embed(console, "table_main", 0, 1);
		console->show(false);

		MainWin->toolbar[0]->enable(true);
		MainWin->toolbar[0]->addItem("", "hui:new", "new");
		MainWin->toolbar[0]->addItem("", "hui:open", "open");
		MainWin->toolbar[0]->addItem("", "hui:save", "save");
		MainWin->toolbar[0]->addSeparator();
		MainWin->toolbar[0]->addItem("", "hui:undo", "undo");
		MainWin->toolbar[0]->addItem("", "hui:redo", "redo");
		MainWin->toolbar[0]->addSeparator();
		MainWin->toolbar[0]->addItem("", "hui:find", "compile");
		MainWin->toolbar[0]->addItem("", "hui:media-play", "compile_and_run");

		MainWin->setTooltip("new", _("neue Datei"));
		MainWin->setTooltip("open", _("eine Datei &offnen"));
		MainWin->setTooltip("save", _("Datei speichern"));

		InitParser();
		HighlightScheme::default_scheme = HighlightScheme::get(HuiConfig.getStr("HighlightScheme", "default"));

		MainWin->setMenu(HuiCreateResourceMenu("menu"));
		MainWin->setMaximized(maximized);
		MainWin->show();

		MainWin->eventSX("file_list", "hui:select", &OnFileList);
		MainWin->eventSX("function_list", "hui:select", &OnFunctionList);

		//msg_write(Asm::Disassemble((void*)&TestTest));

		msg_set_verbose(true);

		msg_write("aaa");
//		Asm::Init(Asm::INSTRUCTION_SET_ARM);
		msg_write("bbb");
		CFile *f = FileOpen("arm/arm-test");
		f->SetBinaryMode(true);
		string code = f->ReadComplete();
		delete(f);
		//msg_write(code.hex());
		msg_write(Asm::Disassemble(&code[1024-12], 64, true));

		Asm::InstructionWithParamsList *l = new Asm::InstructionWithParamsList(0);
		l->add_arm(Asm::ARM_COND_ALWAYS, Asm::inst_add, Asm::REG_R0, Asm::REG_R0, Asm::PK_REGISTER, Asm::REG_R1, 0);
		l->add_arm(Asm::ARM_COND_ALWAYS, Asm::inst_add, Asm::REG_R0, Asm::REG_R1, Asm::PK_CONSTANT, -1, 512);//2040);
		l->add_arm(Asm::ARM_COND_ALWAYS, Asm::inst_ldr, Asm::REG_R0, Asm::REG_R2, Asm::PK_DEREF_REGISTER_SHIFT, -1, 512);//2040);
		l->add_arm(Asm::ARM_COND_ALWAYS, Asm::inst_ldr, Asm::REG_R0, Asm::REG_R2, Asm::PK_DEREF_REGISTER, -1, 0);//2040);
	//	l->add_arm(Asm::ARM_COND_ALWAYS, Asm::inst_add, Asm::REG_R0, Asm::REG_R1, Asm::PK_REGISTER, Asm::REG_R2, 0);
		//l->add_arm(Asm::ARM_COND_ALWAYS, Asm::inst_add, Asm::REG_R0, Asm::REG_R1, Asm::PK_REGISTER_SHIFT, Asm::REG_R2, 5);
	//	l->add_arm(Asm::ARM_COND_ALWAYS, Asm::inst_add, Asm::REG_R0, Asm::REG_R1, Asm::PK_CONSTANT, Asm::REG_R2, 2040);
		int ocs = 0;
		l->Compile(fff, ocs);
		fff[ocs / 4] = 0xe12fff1e;
		ocs += 4;
		msg_write(Asm::Disassemble(fff, ocs, true));


		msg_write("kaba");
		try{
			Script::Script *s = Script::CreateForSource("#show\nint f(int a, int b)\n\tint c = a + b");
			msg_write(Asm::Disassemble(s->Opcode, s->OpcodeSize, true));
		}catch(Script::Exception &e){
			e.print();
		}


#ifdef CPU_ARM
		printf("run...\n");
		int r = (*fp)(1,2);
		printf("%d\n", r);
#endif

		exit(0);

		if (arg.num > 1){
			for (int i=1; i<arg.num; i++)
				LoadFromFile(arg[i]);
		}else
			New();
		return true;
	}
};

HuiExecute(SgribthMaker)
