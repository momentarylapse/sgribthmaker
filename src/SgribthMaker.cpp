#include "SgribthMaker.h"

#include "lib/base/base.h"
#include "lib/hui/hui.h"
#include "lib/nix/nix.h"
#include "lib/file/file.h"
#include "SettingsDialog.h"
#include "CommandDialog.h"
#include "Console.h"
#include "History.h"
#include "HighlightScheme.h"
#include "SourceView.h"
#include "Parser/BaseParser.h"
#include "Document.h"
#include "AutoComplete.h"
#include "lib/kaba/kaba.h"



string AppTitle = "SgribthMaker";
string AppVersion = "0.4.8.3";

//#define ALLOW_LOGGING			true
#define ALLOW_LOGGING			false

namespace nix{
	extern string shader_error;
}

//------------------------------------------------------------------------------
// Highlighting



int status_count = 0;

void SgribthMaker::UpdateStatusBar()
{
	status_count --;
	if (status_count == 0)
		MainWin->enable_statusbar(false);
}

void SgribthMaker::SetMessage(const string &str)
{
	MainWin->set_status_text(str);
	MainWin->enable_statusbar(true);
	status_count ++;
	hui::RunLater(5, std::bind(&SgribthMaker::UpdateStatusBar, this));
}

void SgribthMaker::SetWindowTitle()
{
	if (!cur_doc)
		return;
	MainWin->set_title(cur_doc->name(true) + " - " + AppTitle);
}

void SgribthMaker::UpdateDocList()
{
	MainWin->reset("file_list");
	foreachi(Document *d, documents, i){
		MainWin->add_string("file_list", d->name(false));
		if (cur_doc == d)
			MainWin->set_int("file_list", i);
	}
}

void SgribthMaker::UpdateMenu()
{
	MainWin->enable("undo", cur_doc->history->Undoable());
	MainWin->enable("redo", cur_doc->history->Redoable());
	MainWin->enable("save", cur_doc->history->changed);
	UpdateDocList();
	SetWindowTitle();
}

void SgribthMaker::UpdateFunctionList()
{
	MainWin->reset("function_list");
	if (!cur_doc->parser)
		return;
	Array<Parser::Label> labels = cur_doc->parser->FindLabels(cur_doc->source_view);
	int last_parent = -1;
	foreachi(Parser::Label &l, labels, i){
		if (l.level > 0){
			MainWin->add_child_string("function_list", last_parent, l.name);
		}else{
			last_parent = i;
			MainWin->add_string("function_list", l.name);
		}
	}
}

void SgribthMaker::SetActiveDocument(Document *d)
{
	foreachi(Document *dd, documents, i)
		if (dd == d){
			MainWin->set_int("tab", i);
			MainWin->activate(dd->source_view->id);
			break;
		}
	cur_doc = d;
	UpdateMenu();
	UpdateFunctionList();
}

bool SgribthMaker::AllowTermination()
{
	for (Document *d: documents)
	if (d->history->changed){
		SetActiveDocument(d);
		string answer = QuestionBox(MainWin, _("respectful question"), _("You increased entropy. Do you wish to save your work?"), true);
		if (answer == "hui:cancel")
			return false;
		if (answer == "hui:yes")
			return Save(d);
		return true;
	}
	return true;
}

bool SgribthMaker::AllowDocTermination(Document *d)
{
	if (d->history->changed){
		SetActiveDocument(d);
		string answer = QuestionBox(MainWin, _("respectful question"), _("You increased entropy. Do you wish to save your work?"), true);
		if (answer == "hui:cancel")
			return false;
		if (answer == "hui:yes")
			return Save(d);
		return true;
	}
	return true;
}

void SgribthMaker::New()
{
	if (documents.num > 0)
		MainWin->hide_control("table_side", false);

	string id = format("edit-%06d", randi(1000000));

	MainWin->set_border_width(0);
	MainWin->add_string("tab", i2s(documents.num));
	MainWin->set_target("tab");
	MainWin->add_grid("", documents.num, 0, id + "-grid");
	MainWin->set_target(id + "-grid");
	if (hui::Config.get_bool("ShowLineNumbers", false)){
		MainWin->add_multiline_edit("!noframe,disabled,width=70,noexpandx", 0, 0, id + "-lines");
		MainWin->enable(id + "-lines", false);
	}
	MainWin->add_multiline_edit("!handlekeys,noframe", 1, 0, id);

	documents.add(new Document(this));
	SourceView *sv = new SourceView(MainWin, id, documents.back());

	sv->ApplyScheme(HighlightScheme::default_scheme);
	source_view.add(sv);

	SetActiveDocument(documents.back());
	UpdateMenu();
}

void SgribthMaker::OnCloseDocument()
{
	if (!AllowDocTermination(cur_doc))
		return;

	if (documents.num <= 1){
		OnExit();
		return;
	}

	int n = MainWin->get_int("tab");
	MainWin->remove_string("tab", n);
	delete(documents[n]);
	documents.erase(n);
	delete(source_view[n]);
	source_view.erase(n);

	SetActiveDocument(documents.back());
	UpdateMenu();
}

bool SgribthMaker::LoadFromFile(const string &filename)
{
	New();
	return documents.back()->load(filename);
}

bool SgribthMaker::WriteToFile(Document *doc, const string &filename)
{
	bool ok = doc->save(filename);
	if (ok)
		SetMessage(_("saved"));
	return ok;
}

bool SgribthMaker::Open()
{
	if (hui::FileDialogOpen(MainWin, _("Open file"), cur_doc->filename.dirname(), _("All (*.*)"), "*"))
		return LoadFromFile(hui::Filename);
	return false;
}

bool SgribthMaker::SaveAs(Document *doc)
{
	if (hui::FileDialogSave(MainWin, _("Save file"), doc->filename.dirname(), _("All (*.*)"), "*"))
		return WriteToFile(doc, hui::Filename);
	return false;
}

bool SgribthMaker::Save(Document *doc)
{
	if (doc->filename.num > 0)
		return WriteToFile(doc, doc->filename);
	else
		return SaveAs(doc);
}

void SgribthMaker::OnOpen()
{	Open();	}

void SgribthMaker::OnSave()
{	Save(cur_doc);	}

void SgribthMaker::OnSaveAs()
{	SaveAs(cur_doc);	}

bool SgribthMaker::Reload()
{
	if (!AllowTermination())
		return false;
	if (cur_doc->filename.num > 0){
		bool r = cur_doc->load(cur_doc->filename);
		if (r)
			SetMessage(_("reloaded"));
		return r;
	}
	return true;
}

void SgribthMaker::OnReload()
{	Reload();	}

void SgribthMaker::OnUndo()
{	cur_doc->history->Undo();	}

void SgribthMaker::OnRedo()
{	cur_doc->history->Redo();	}

void SgribthMaker::OnCopy()
{
	hui::Clipboard::Copy(cur_doc->source_view->GetSelection());
	SetMessage(_("copied"));
}

void SgribthMaker::OnPaste()
{
	cur_doc->source_view->InsertAtCursor(hui::Clipboard::Paste());
	SetMessage(_("pasted"));
}

void SgribthMaker::OnDelete()
{
	cur_doc->source_view->DeleteSelection();
}

void SgribthMaker::OnCut()
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

void SgribthMaker::CompileKaba()
{

	//HuiSetDirectory(SgribthDir);
	msg_set_verbose(true);

	hui::Timer CompileTimer;

	Kaba::config.compile_silently = true;
	//Kaba::config.verbose = true;

	try{
		Kaba::Script *compile_script = Kaba::Load(cur_doc->filename, true);

		float dt = CompileTimer.get();

		//compile_script->Show();

		SetMessage(format(_("Script compilable without errors!         (in %s)"), get_time_str(dt).c_str()));

	}catch(const Kaba::Exception &e){
		e.print();
		ErrorBox(MainWin, _("Error"), e.message());
		cur_doc->source_view->MoveCursorTo(e.line, e.column);
	}

	//RemoveScript(compile_script);
	Kaba::DeleteAllScripts(true, true);

	msg_set_verbose(ALLOW_LOGGING);
}

void SgribthMaker::CompileShader()
{
	return;

	hui::Window *w = new hui::Window("nix", 640, 480);
	w->add_drawing_area("!opengl", 0, 0, "nix-area");
	w->show();
//	nix::init("OpenGL", w, "nix-area");

	nix::Shader *shader = nix::LoadShader(cur_doc->filename);
	if (!shader){
		ErrorBox(MainWin, _("Error"), nix::shader_error);
	}else{
		SetMessage(_("Shader compiles without error!"));
		shader->unref();
	}
	delete(w);

	msg_set_verbose(ALLOW_LOGGING);
}

void SgribthMaker::Compile()
{
	string ext = cur_doc->filename.extension();

	if (!Save(cur_doc))
		return;

	if (ext == "kaba")
		CompileKaba();
	else if (ext == "shader")
		CompileShader();
	else
		SetMessage(_("only *.kaba and *.shader files can be compiled!"));
}

void SgribthMaker::CompileAndRun(bool verbose)
{
	if (cur_doc->filename.extension() != "kaba"){
		SetMessage(_("only *.kaba files can be executed!"));
		return;
	}

	if (!Save(cur_doc))
		return;

	hui::SetDirectory(cur_doc->filename.dirname());
	//if (verbose)
		msg_set_verbose(true);

	// compile
	hui::Timer CompileTimer;
	Kaba::config.compile_silently = true;
	//Kaba::config.verbose = true;

	try{
		Kaba::Script *compile_script = Kaba::Load(cur_doc->filename);
		float dt_compile = CompileTimer.get();

		if (!verbose)
			msg_set_verbose(true);

		int msg_size0 = msg_get_buffer_size();


		float dt_execute = 0;
		CompileTimer.reset();
		typedef void void_func();
		void_func *f = (void_func*)compile_script->MatchFunction("main", "void", 0);
		if (f)
			f();
		//compile_script->ShowVars(false);
		dt_execute = CompileTimer.get();
		
		SetMessage(format(_("Compiling: %s         opcode: %db         execution: %s"), get_time_str(dt_compile).c_str(), compile_script->opcode_size, get_time_str(dt_execute).c_str()));
		//if (verbose)
		//	HuiInfoBox(MainWin,"Speicher",string("nicht freigegebener Speicher des Scriptes: ",i2s(script->MemoryUsed),"b"));}

		// messages?
		int msg_size = msg_get_buffer_size();
		if (msg_size > msg_size0)
			console->set(msg_get_buffer(msg_size - msg_size0));

	}catch(const Kaba::Exception &e){
		e.print();
		ErrorBox(MainWin, _("Error"), e.message());
		cur_doc->source_view->MoveCursorTo(e.line, e.column);
	}
	

	//RemoveScript(compile_script);
	Kaba::DeleteAllScripts(true, true);

	msg_set_verbose(ALLOW_LOGGING);
}

void SgribthMaker::OnCompileAndRunVerbose()
{	CompileAndRun(true);	}

void SgribthMaker::OnCompileAndRunSilent()
{	CompileAndRun(false);	}


static AutoComplete::Data _auto_complete_data_;

void SgribthMaker::OnInsertAutoComplete(int n)
{
	if ((n >= 0) and (n < _auto_complete_data_.suggestions.num))
		cur_doc->source_view->InsertAtCursor(_auto_complete_data_.suggestions[n].name.substr(_auto_complete_data_.offset, -1));

}

void SgribthMaker::OnAutoComplete()
{
	if (cur_doc->filename.extension() != "kaba"){
		SetMessage(_("auto-completion only available for *.kaba files!"));
		return;
	}

	hui::SetDirectory(cur_doc->filename.dirname());

	int line, pos;
	cur_doc->source_view->GetCurLinePos(line, pos);
	auto data = AutoComplete::run(cur_doc->source_view->GetAll(), line, pos);
	_auto_complete_data_ = data;

	if (data.suggestions.num == 1){
		cur_doc->source_view->InsertAtCursor(data.suggestions[0].name.substr(data.offset, -1));
		SetMessage(data.suggestions[0].context);

	}else if (data.suggestions.num > 1){
		auto *m = new hui::Menu;
		foreachi (auto &s, data.suggestions, i)
			m->add(s.name, "auto-complete-" + i2s(i));
		m->open_popup(MainWin);
	}else{
		SetMessage(_("????"));
	}
}


void SgribthMaker::ShowCurLine()
{
	int line, off;
	cur_doc->source_view->GetCurLinePos(line, off);
	SetMessage(format(_("Line  %d : %d"), line + 1, off + 1));
}

void SgribthMaker::ExecuteCommand(const string &cmd)
{
	bool found = cur_doc->source_view->Find(cmd);
	if (!found)
		SetMessage(format(_("\"%s\" not found"), cmd.c_str()));
}

void SgribthMaker::ExecuteCommandDialog()
{
	CommandDialog *dlg = new CommandDialog(this);
	dlg->run();
	delete dlg;
}

void SgribthMaker::ExecuteSettingsDialog()
{
	SettingsDialog *dlg = new SettingsDialog(this);
	dlg->run();
	delete dlg;
}



void SgribthMaker::OnFunctionList()
{
	int n = MainWin->get_int("");
	Array<Parser::Label> labels = cur_doc->parser->FindLabels(cur_doc->source_view);
	if ((n >= 0) and (n < labels.num)){
		cur_doc->source_view->ShowLineOnScreen(labels[n].line);
		MainWin->activate(cur_doc->source_view->id);
	}
}

void SgribthMaker::SgribthMaker::OnFileList()
{
	int s = MainWin->get_int("");
	if (s >= 0)
		SetActiveDocument(documents[s]);
}

void SgribthMaker::OnNextDocument()
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

void SgribthMaker::OnPreviousDocument()
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

SgribthMaker::SgribthMaker() :
	hui::Application("sgribthmaker", "English", hui::FLAG_LOAD_RESOURCE | hui::FLAG_SILENT)
{
	set_property("name", AppTitle);
	set_property("version", AppVersion);
	set_property("comment", _("Text editor and kaba compiler"));
	set_property("website", "http://michi.is-a-geek.org/michisoft");
	set_property("copyright", "© 2006-2018 by MichiSoft TM");
	set_property("author", "Michael Ankele <michi@lupina.de>");

	hui::RegisterFileType("kaba","MichiSoft Script Datei", directory + "Data/kaba.ico", filename,"open",true);

	Kaba::Init();

	console = NULL;
	MainWin = NULL;
}

bool SgribthMaker::on_startup(const Array<string> &arg)
{
	int width = hui::Config.get_int("Window.Width", 800);
	int height = hui::Config.get_int("Window.Height", 600);
	bool maximized = hui::Config.get_bool("Window.Maximized", false);

	MainWin = new hui::Window(AppTitle, width, height);
	MainWin->from_resource("main-window");

	MainWin->event("about", std::bind(&SgribthMaker::OnAbout, this));
	MainWin->event("hui:close", std::bind(&SgribthMaker::OnExit, this));

	MainWin->event("new", std::bind(&SgribthMaker::New, this));
	MainWin->set_key_code("new", hui::KEY_N + hui::KEY_CONTROL, "hui:new");
	//hui::HuiAddKeyCode(HMM_NEW_HEX, hui::KEY_F1 + 256);
	MainWin->event("open", std::bind(&SgribthMaker::OnOpen, this));
	MainWin->set_key_code("open", hui::KEY_O + hui::KEY_CONTROL, "hui:open");
	//hui::HuiAddKeyCode(HMM_OPEN_HEX, hui::KEY_F9 + 256);
	MainWin->event("save", std::bind(&SgribthMaker::OnSave, this));
	MainWin->set_key_code("save", hui::KEY_S + hui::KEY_CONTROL, "hui:save");
	MainWin->event("save_as", std::bind(&SgribthMaker::OnSaveAs, this));
	MainWin->set_key_code("save_as", hui::KEY_S + hui::KEY_SHIFT + hui::KEY_CONTROL, "hui:save_as");
	MainWin->event("close", std::bind(&SgribthMaker::OnCloseDocument, this));
	MainWin->set_key_code("close", hui::KEY_W + hui::KEY_CONTROL, "hui:close");
	MainWin->event("exit", std::bind(&SgribthMaker::OnExit, this));
	MainWin->set_key_code("exit", hui::KEY_Q + hui::KEY_CONTROL, "hui:quit");
	//MainWin->event("show_data", "", hui::KEY_D + hui::KEY_CONTROL, &ShowData);
	MainWin->event("execute_command", std::bind(&SgribthMaker::ExecuteCommandDialog, this));
	MainWin->set_key_code("execute_command", hui::KEY_E + hui::KEY_CONTROL, "");
	MainWin->event("find", std::bind(&SgribthMaker::ExecuteCommandDialog, this));
	MainWin->set_key_code("find", hui::KEY_F + hui::KEY_CONTROL, "");
	MainWin->event("cut", std::bind(&SgribthMaker::OnCut, this));
	MainWin->set_key_code("cut", hui::KEY_X + hui::KEY_CONTROL, "hui:cut");
	MainWin->event("copy", std::bind(&SgribthMaker::OnCopy, this));
	MainWin->set_key_code("copy", hui::KEY_C + hui::KEY_CONTROL, "hui:copy");
	MainWin->event("paste", std::bind(&SgribthMaker::OnPaste, this));
	MainWin->set_key_code("paste", hui::KEY_V + hui::KEY_CONTROL, "hui:paste");
	MainWin->event("reload", std::bind(&SgribthMaker::OnReload, this));
	MainWin->set_key_code("reload", hui::KEY_R + hui::KEY_CONTROL, "hui:reload");
	MainWin->event("undo", std::bind(&SgribthMaker::OnUndo, this));
	MainWin->set_key_code("undo", hui::KEY_Z + hui::KEY_CONTROL, "hui:undo");
	MainWin->event("redo", std::bind(&SgribthMaker::OnRedo, this));
	MainWin->set_key_code("redo", hui::KEY_Z + hui::KEY_SHIFT + hui::KEY_CONTROL, "hui:redo");
	MainWin->event("compile", std::bind(&SgribthMaker::Compile, this));
	MainWin->set_key_code("compile", hui::KEY_F7);
	MainWin->event("compile_and_run_verbose", std::bind(&SgribthMaker::OnCompileAndRunVerbose, this));
	MainWin->set_key_code("compile_and_run_verbose", hui::KEY_F6 + hui::KEY_CONTROL);
	MainWin->event("compile_and_run", std::bind(&SgribthMaker::OnCompileAndRunSilent, this));
	MainWin->set_key_code("compile_and_run", hui::KEY_F6);
	MainWin->event("auto-complete", std::bind(&SgribthMaker::OnAutoComplete, this));
	MainWin->set_key_code("auto-complete", hui::KEY_CONTROL + hui::KEY_SPACE);
	MainWin->event("settings", std::bind(&SgribthMaker::ExecuteSettingsDialog, this));
	//MainWin->event("script_help", "hui:help", hui::KEY_F1 + hui::KEY_SHIFT);
	MainWin->event("next_document", std::bind(&SgribthMaker::OnNextDocument, this));
	MainWin->set_key_code("next_document", hui::KEY_PRIOR + hui::KEY_CONTROL, "hui:down");
	MainWin->event("prev_document", std::bind(&SgribthMaker::OnPreviousDocument, this));
	MainWin->set_key_code("prev_document", hui::KEY_NEXT + hui::KEY_CONTROL, "hui:up");

	MainWin->event("show_cur_line", std::bind(&SgribthMaker::ShowCurLine, this));
	MainWin->set_key_code("show_cur_line", hui::KEY_F2);

	for (int i=0; i<100; i++)
		MainWin->event("auto-complete-" + i2s(i), [this,i]{ OnInsertAutoComplete(i); });


	console = new Console;
	MainWin->embed(console, "table_main", 0, 1);
	console->show(false);

	MainWin->toolbar[0]->set_by_id("toolbar");

	InitParser();
	HighlightScheme::default_scheme = HighlightScheme::get(hui::Config.get_str("HighlightScheme", "default"));

	MainWin->set_menu(hui::CreateResourceMenu("menu"));
	MainWin->set_maximized(maximized);
	MainWin->show();

	MainWin->event_x("file_list", "hui:select", std::bind(&SgribthMaker::OnFileList, this));
	MainWin->event_x("function_list", "hui:select", std::bind(&SgribthMaker::OnFunctionList, this));



	if (arg.num > 1){
		for (int i=1; i<arg.num; i++)
			LoadFromFile(arg[i]);
	}else
		New();
	return true;
}


void SgribthMaker::OnAbout()
{	AboutBox(MainWin);	}

void SgribthMaker::OnExit()
{
	if (AllowTermination()){
		int w, h;
		MainWin->get_size_desired(w, h);
		hui::Config.set_int("Window.Width", w);
		hui::Config.set_int("Window.Height", h);
		hui::Config.set_bool("Window.Maximized", MainWin->is_maximized());
		delete MainWin;
		end();
	}
}

HUI_EXECUTE(SgribthMaker)
