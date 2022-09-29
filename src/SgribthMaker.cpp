#include "SgribthMaker.h"

#include "lib/base/base.h"
#include "lib/hui/hui.h"
#include "lib/nix/nix.h"
#include "lib/os/filesystem.h"
#include "lib/os/msg.h"
#include "lib/os/time.h"
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
string AppVersion = "0.4.10.2";

//#define ALLOW_LOGGING			true
#define ALLOW_LOGGING			false

namespace nix {
	extern string shader_error;
}

//------------------------------------------------------------------------------
// Highlighting



int status_count = -1;

void SgribthMaker::UpdateStatusBar() {
	status_count --;
	if (status_count == 0)
		MainWin->set_info_text("", {"clear"});
		//MainWin->enable_statusbar(false);
}

void SgribthMaker::SetMessage(const string &str) {
	/*MainWin->set_status_text(str);
	MainWin->enable_statusbar(true);
	status_count ++;
	hui::RunLater(5, [=]{ UpdateStatusBar(); });*/
	if (str.num >= 20)
		SetInfo(str);
}

void SgribthMaker::SetError(const string &str) {
	//hui::error_box(MainWin, "error", str);
	MainWin->set_info_text(str, {"error", "allow-close"});
	status_count = -1;
}

void SgribthMaker::SetInfo(const string &str) {
	MainWin->set_info_text(str, {"info", "allow-close"});
	status_count = max(0, status_count) + 1;
	hui::run_later(5, [=]{ UpdateStatusBar(); });
}

void SgribthMaker::SetWindowTitle() {
	if (!cur_doc)
		return;
	MainWin->set_title(cur_doc->name(true) + " - " + AppTitle);
}

void SgribthMaker::UpdateDocList() {
	MainWin->reset("file_list");
	foreachi(Document *d, documents, i) {
		MainWin->add_string("file_list", d->name(false));
		if (cur_doc == d)
			MainWin->set_int("file_list", i);
	}
}

void SgribthMaker::UpdateMenu() {
	MainWin->enable("undo", cur_doc->history->Undoable());
	MainWin->enable("redo", cur_doc->history->Redoable());
	//MainWin->enable("save", cur_doc->history->changed);
	UpdateDocList();
	SetWindowTitle();
}

void SgribthMaker::UpdateFunctionList() {
	MainWin->reset("function_list");
	if (!cur_doc->parser)
		return;
	auto labels = cur_doc->parser->FindLabels(cur_doc->source_view);
	int last_parent = -1;
	foreachi(Parser::Label &l, labels, i) {
		if (l.level > 0) {
			MainWin->add_child_string("function_list", last_parent, l.name);
		} else {
			last_parent = i;
			MainWin->add_string("function_list", l.name);
		}
	}
}

void SgribthMaker::SetActiveDocument(Document *d) {
	foreachi(Document *dd, documents, i)
		if (dd == d) {
			MainWin->set_int("tab", i);
			MainWin->activate(dd->source_view->id);
			break;
		}
	cur_doc = d;
	UpdateMenu();
	UpdateFunctionList();
}

void SgribthMaker::AllowTermination(const hui::Callback &on_success, const hui::Callback &on_fail) {
	Array<Document*> unsaved;
	for (auto d: documents)
		if (d->history->changed)
			unsaved.add(d);
	if (unsaved.num == 0) {
		on_success();
	} else if (unsaved.num == 1) {
		SetActiveDocument(unsaved[0]);
		hui::question_box(MainWin, _("respectful question"), _("You increased entropy. Do you wish to save your work?"), [this,unsaved,on_success,on_fail] (const string &answer) {
			if (answer == "hui:cancel")
				on_fail();
			else if (answer == "hui:yes")
				Save(unsaved[0], on_success, on_fail);
			else
				on_success();
		}, true);
	} else {
		hui::question_box(MainWin, _("respectful question"), _("Several unsaved documents. Do you want to save before quitting?"), [this,unsaved,on_success,on_fail] (const string &answer) {
			if (answer == "hui:cancel")
				on_fail();
			else if (answer == "hui:yes")
				on_fail();
			else
				on_success();
		}, true);
	}
}

void SgribthMaker::AllowDocTermination(Document *d, const hui::Callback &on_success, const hui::Callback &on_fail) {
	if (d->history->changed) {
		SetActiveDocument(d);
		hui::question_box(MainWin, _("respectful question"), _("You increased entropy. Do you wish to save your work?"), [this,d,on_success,on_fail] (const string &answer) {
			if (answer == "hui:cancel")
				on_fail();
			else if (answer == "hui:yes")
				Save(d, on_success, on_fail);
			else
				on_success();
		}, true);
	} else {
		on_success();
	}
}

void SgribthMaker::New() {
	if (documents.num > 0)
		MainWin->hide_control("table_side", false);

	string id = format("edit-%06d", randi(1000000));

	MainWin->set_border_width(0);
	MainWin->add_string("tab", i2s(documents.num));
	MainWin->set_target("tab");
	MainWin->add_grid("", documents.num, 0, id + "-grid");
	MainWin->set_target(id + "-grid");
	if (hui::config.get_bool("ShowLineNumbers", false)) {
		MainWin->add_multiline_edit("!noframe,disabled,width=70,noexpandx", 0, 0, id + "-lines");
		MainWin->enable(id + "-lines", false);
	}
	MainWin->add_multiline_edit("!handlekeys,noframe", 1, 0, id);

	documents.add(new Document(this));
	SourceView *sv = new SourceView(MainWin, id, documents.back());

	sv->apply_scheme(HighlightScheme::default_scheme);
	source_view.add(sv);

	SetActiveDocument(documents.back());
	UpdateMenu();
}

void SgribthMaker::OnCloseDocument() {
	AllowDocTermination(cur_doc, [this] {
		if (documents.num <= 1) {
			OnExit();
			return;
		}

		int n = MainWin->get_int("tab");
		hui::run_later(0.001f, [=] {
			MainWin->remove_string("tab", n);
			delete documents[n];
			documents.erase(n);
			delete source_view[n];
			source_view.erase(n);

			SetActiveDocument(documents.back());
			UpdateMenu();
		});
	}, [] {});
}

bool SgribthMaker::LoadFromFile(const Path &filename) {
	New();
	return documents.back()->load(filename);
}

bool SgribthMaker::WriteToFile(Document *doc, const Path &filename) {
	bool ok = doc->save(filename);
	if (ok)
		SetMessage(_("saved"));
	return ok;
}

Path working_dir_from_doc(Document *d) {
	if (d->filename.is_empty())
		return os::fs::current_directory();
	return d->filename.parent();
}

void SgribthMaker::Open() {
	hui::file_dialog_open(MainWin, _("Open file"), working_dir_from_doc(cur_doc), {"showfilter="+_("All (*.*)"), "filter=*"}, [this] (const Path &filename) {
		if (filename)
			LoadFromFile(filename);
	});
}

void SgribthMaker::SaveAs(Document *doc, const hui::Callback &on_success, const hui::Callback &on_fail) {
	hui::file_dialog_save(MainWin, _("Save file"), working_dir_from_doc(doc), {"showfilter=" + _("All (*.*)"), "filter=*"}, [this,doc,on_success,on_fail] (const Path &filename) {
		if (!filename.is_empty()) {
			if (WriteToFile(doc, filename))
				on_success();
			else
				on_fail();
		}
	});
}

void SgribthMaker::Save(Document *doc, const hui::Callback &on_success, const hui::Callback &on_fail) {
	if (!doc->filename.is_empty()) {
		if (WriteToFile(doc, doc->filename))
			on_success();
		else
			on_fail();
	} else {
		SaveAs(doc, on_success, on_fail);
	}
}

void SgribthMaker::OnOpen()
{	Open();	}

void SgribthMaker::OnSave()
{	Save(cur_doc, []{}, []{});	}

void SgribthMaker::OnSaveAs()
{	SaveAs(cur_doc, []{}, []{});	}

void SgribthMaker::Reload() {
	AllowDocTermination(cur_doc, [this] {
		if (!cur_doc->filename.is_empty()) {
			bool r = cur_doc->load(cur_doc->filename);
			if (r)
				SetMessage(_("reloaded"));
		}
	}, []{});
}

void SgribthMaker::OnReload()
{	Reload();	}

void SgribthMaker::on_undo()
{	cur_doc->history->Undo();	}

void SgribthMaker::on_redo()
{	cur_doc->history->Redo();	}

void SgribthMaker::on_copy() {
	hui::Clipboard::copy(cur_doc->source_view->get_selection_content());
	SetMessage(_("copied"));
}

void SgribthMaker::on_paste() {
	msg_write("SM.on paste");
	cur_doc->source_view->delete_selection();
	msg_write("x");
	auto p = hui::Clipboard::paste();
	msg_write("y " + i2s(p.num));
	cur_doc->source_view->insert_at_cursor(p);//hui::Clipboard::paste());
	msg_write("/SM.on paste");
	SetMessage(_("pasted"));
}

void SgribthMaker::prepend_selected_lines(const string &s) {
	auto sv = cur_doc->source_view;
	int pos0, pos1;
	sv->get_selection(pos0, pos1);
	int l0 = sv->get_line_no_at(pos0);
	int l1 = sv->get_line_no_at(pos1);
	for (int l = l0; l<=l1; l++) {
		sv->insert_at(sv->get_line_offset(l), s);
	}
}

void SgribthMaker::unprepend_selected_lines(const string &s) {
	auto sv = cur_doc->source_view;
	int pos0, pos1;
	sv->get_selection(pos0, pos1);
	int l0 = sv->get_line_no_at(pos0);
	int l1 = sv->get_line_no_at(pos1);
	for (int l = l0; l<=l1; l++) {
		int offset = sv->get_line_offset(l);
		string ss = sv->get_content(offset, offset + 1);
		if (s == ss)
			sv->delete_content(offset, offset + 1);
	}
}

void SgribthMaker::on_comment() {
	prepend_selected_lines(cur_doc->parser->line_comment_begin);
}

void SgribthMaker::on_uncomment() {
	unprepend_selected_lines(cur_doc->parser->line_comment_begin);
}

void SgribthMaker::on_indent() {
	auto sv = cur_doc->source_view;
	if (sv->has_selection())
		prepend_selected_lines("\t");
	else
		sv->insert_at_cursor("\t");
}

void SgribthMaker::on_unindent() {
	unprepend_selected_lines("\t");
}

void SgribthMaker::on_delete() {
	cur_doc->source_view->delete_selection();
}

void SgribthMaker::on_cut() {
	on_copy();
	on_delete();
}

string get_time_str(float t) {
	if (t < 0.3f)
		return format("%.2fms", t * 1000.0f);
	else
		return format("%.2fs", t);
}

void SgribthMaker::CompileKaba() {

	//HuiSetDirectory(SgribthDir);
	//msg_set_verbose(true);

	os::Timer CompileTimer;

	kaba::config.compile_silently = true;
	//kaba::config.verbose = true;

	try {
		auto compile_module = kaba::load(cur_doc->filename, true);

		float dt = CompileTimer.get();

		//compile_module->Show();

		SetMessage(format(_("Script compilable without errors!         (in %s)"), get_time_str(dt).c_str()));

	} catch (const kaba::Exception &e) {
		e.print();
		//ErrorBox(MainWin, _("Error"), e.message());
		SetError(e.message());
		cur_doc->source_view->move_cursor_to(e.line, e.column);
	}

	//RemoveModule(compile_module);
	kaba::delete_all_modules(true, true);

	//msg_set_verbose(ALLOW_LOGGING);
}

void SgribthMaker::CompileShader() {
	return;

	// NOPE, not working
	hui::run_later(0.01f, [=] {

	auto *w = new hui::NixWindow("nix", 640, 480);
	//w->add_drawing_area("!opengl", 0, 0, "nix-area");
	w->event_x("nix-area", "realize", [=] {
		nix::init();
	//	nix::init("OpenGL", w, "nix-area");

		auto *shader = nix::Shader::load(cur_doc->filename);
		if (!shader) {
			SetError(nix::shader_error);
		} else {
			SetMessage(_("Shader compiles without error!"));
			//shader->unref();
		}
	});


	w->show();
	delete w;

	//msg_set_verbose(ALLOW_LOGGING);
	});
}

void SgribthMaker::Compile() {
	Save(cur_doc, [this] {
		string ext = cur_doc->filename.extension();
		if (ext == "kaba")
			CompileKaba();
		else if (ext == "shader")
			CompileShader();
		else
			SetError(_("only *.kaba and *.shader files can be compiled!"));
	}, []{});
}

void SgribthMaker::CompileAndRun(bool verbose) {
	if (cur_doc->filename.extension() != "kaba") {
		SetError(_("only *.kaba files can be executed!"));
		return;
	}

	Save(cur_doc, [this,verbose]{

		hui::SetDirectory(cur_doc->filename.parent());
		//if (verbose)
		//	msg_set_verbose(true);

		// compile
		os::Timer CompileTimer;
		kaba::config.compile_silently = true;
		//kaba::config.verbose = true;

		try {
			auto compile_module = kaba::load(cur_doc->filename);
			float dt_compile = CompileTimer.get();

			if (!verbose)
				msg_set_verbose(true);

			int msg_size0 = msg_get_buffer_size();


			float dt_execute = 0;
			CompileTimer.reset();
			typedef void void_func();
			typedef void strings_func(const Array<string>);
			auto f = (void_func*)compile_module->match_function("main", "void", {});
			auto fsa = (strings_func*)compile_module->match_function("main", "void", {"string[]"});
			if (f) {
				f();
			} else if (fsa) {
				fsa({});
			}
			//compile_module->ShowVars(false);
			dt_execute = CompileTimer.get();

			if (f or fsa) {
				SetMessage(format(_("Compiling: %s         opcode: %db         execution: %s"), get_time_str(dt_compile).c_str(), compile_module->opcode_size, get_time_str(dt_execute).c_str()));
			} else {
				SetError(_("no 'void main()' or 'void main(string[])' found"));
			}
			//if (verbose)
			//	HuiInfoBox(MainWin,"Speicher",string("nicht freigegebener Speicher des Scriptes: ",i2s(script->MemoryUsed),"b"));}

			// messages? (not working anymore, since kaba is not writing to log)
			int msg_size = msg_get_buffer_size();
			if (msg_size > msg_size0)
				console->set(msg_get_buffer(msg_size - msg_size0));

		} catch(const kaba::Exception &e) {
			e.print();

			SetError(e.message());
			//ErrorBox(MainWin, _("Error"), e.message());
			cur_doc->source_view->move_cursor_to(e.line, e.column);
		}
		
	
		//RemoveScript(compile_script);
		kaba::delete_all_modules(true, true);

		//msg_set_verbose(ALLOW_LOGGING);
	}, []{});
}

void SgribthMaker::OnCompileAndRunVerbose()
{	CompileAndRun(true);	}

void SgribthMaker::OnCompileAndRunSilent()
{	CompileAndRun(false);	}


static AutoComplete::Data _auto_complete_data_;

void SgribthMaker::OnInsertAutoComplete(int n) {
	if ((n >= 0) and (n < _auto_complete_data_.suggestions.num))
		cur_doc->source_view->insert_at_cursor(_auto_complete_data_.suggestions[n].name.sub(_auto_complete_data_.offset));
}

void SgribthMaker::OnAutoComplete() {
	if (cur_doc->filename.extension() != "kaba") {
		SetError(_("auto-completion only available for *.kaba files!"));
		return;
	}

	hui::SetDirectory(cur_doc->filename.parent());

	int line, pos;
	cur_doc->source_view->get_cur_line_pos(line, pos);
	auto data = AutoComplete::run(cur_doc->source_view->get_all(), cur_doc->filename, line, pos);
	_auto_complete_data_ = data;

	if (data.suggestions.num == 1) {
		cur_doc->source_view->insert_at_cursor(data.suggestions[0].name.sub(data.offset));
		SetMessage(data.suggestions[0].context);

	} else if (data.suggestions.num > 1) {
		auto *m = new hui::Menu(MainWin);
		foreachi (auto &s, data.suggestions, i)
			m->add(s.name, "auto-complete-" + i2s(i));
		m->open_popup(MainWin);
	} else {
		SetMessage(_("????"));
	}
}


void SgribthMaker::ShowCurLine() {
	int line, off;
	cur_doc->source_view->get_cur_line_pos(line, off);
	SetMessage(format(_("Line  %d : %d"), line + 1, off + 1));
}

void SgribthMaker::ExecuteCommand(const string &cmd) {
	bool found = cur_doc->source_view->find(cmd);
	if (!found)
		SetError(format(_("\"%s\" not found"), cmd.c_str()));
}

void SgribthMaker::ExecuteCommandDialog() {
	hui::fly(new CommandDialog(this));
}

void SgribthMaker::ExecuteSettingsDialog() {
	hui::fly(new SettingsDialog(this));
}



void SgribthMaker::OnFunctionList() {
	int n = MainWin->get_int("");
	auto labels = cur_doc->parser->FindLabels(cur_doc->source_view);
	if ((n >= 0) and (n < labels.num)) {
		cur_doc->source_view->show_line_on_screen(labels[n].line);
		MainWin->activate(cur_doc->source_view->id);
	}
}

void SgribthMaker::SgribthMaker::OnFileList() {
	int s = MainWin->get_int("");
	if (s >= 0)
		SetActiveDocument(documents[s]);
}

void SgribthMaker::OnNextDocument() {
	foreachi(Document *d, documents, i)
		if (d == cur_doc) {
			if (i < documents.num - 1)
				SetActiveDocument(documents[i + 1]);
			else
				SetActiveDocument(documents[0]);
			break;
		}
}

void SgribthMaker::OnPreviousDocument() {
	foreachi(Document *d, documents, i)
		if (d == cur_doc) {
			if (i > 0)
				SetActiveDocument(documents[i - 1]);
			else
				SetActiveDocument(documents.back());
			break;
		}
}

SgribthMaker::SgribthMaker() :
	hui::Application("sgribthmaker", "English", hui::FLAG_SILENT)
{
	set_property("name", AppTitle);
	set_property("version", AppVersion);
	set_property("comment", _("Text editor and kaba compiler"));
	set_property("website", "http://michi.is-a-geek.org/michisoft");
	set_property("copyright", "© 2006-2018 by MichiSoft TM");
	set_property("author", "Michael Ankele <michi@lupina.de>");

//	hui::RegisterFileType("kaba","MichiSoft Script Datei", directory + "Data/kaba.ico", filename,"open",true);

	kaba::init();

	console = NULL;
	MainWin = NULL;
}


bool SgribthMaker::on_startup(const Array<string> &arg) {
	int width = hui::config.get_int("Window.Width", 800);
	int height = hui::config.get_int("Window.Height", 600);
	bool maximized = hui::config.get_bool("Window.Maximized", false);



	MainWin = new hui::Window(AppTitle, width, height);
	MainWin->from_resource("main-window");
	MainWin->set_menu(hui::create_resource_menu("menu", MainWin));
	MainWin->toolbar[0]->set_by_id("toolbar");


	MainWin->event("about", [=]{ OnAbout(); });
	MainWin->event("hui:close", [=]{ OnExit(); });

	MainWin->event("new", [=]{ New(); });
	MainWin->set_key_code("new", hui::KEY_N + hui::KEY_CONTROL, "hui:new");
	//hui::HuiAddKeyCode(HMM_NEW_HEX, hui::KEY_F1 + 256);
	MainWin->event("open", [=]{ OnOpen(); });
	MainWin->set_key_code("open", hui::KEY_O + hui::KEY_CONTROL, "hui:open");
	//hui::HuiAddKeyCode(HMM_OPEN_HEX, hui::KEY_F9 + 256);
	MainWin->event("save", [=]{ OnSave(); });
	MainWin->set_key_code("save", hui::KEY_S + hui::KEY_CONTROL, "hui:save");
	MainWin->event("save_as", [=]{ OnSaveAs(); });
	MainWin->set_key_code("save_as", hui::KEY_S + hui::KEY_SHIFT + hui::KEY_CONTROL, "hui:save_as");
	MainWin->event("close", [=]{ OnCloseDocument(); });
	MainWin->set_key_code("close", hui::KEY_W + hui::KEY_CONTROL, "hui:close");
	MainWin->event("exit", [=]{ OnExit(); });
	MainWin->set_key_code("exit", hui::KEY_Q + hui::KEY_CONTROL, "hui:quit");
	//MainWin->event("show_data", "", hui::KEY_D + hui::KEY_CONTROL, &ShowData);
	MainWin->event("execute_command", [=]{ ExecuteCommandDialog(); });
	MainWin->set_key_code("execute_command", hui::KEY_E + hui::KEY_CONTROL, "");
	MainWin->event("find", [=]{ ExecuteCommandDialog(); });
	MainWin->set_key_code("find", hui::KEY_F + hui::KEY_CONTROL, "");
	MainWin->event("cut", [=]{ on_cut(); });
	MainWin->set_key_code("cut", hui::KEY_X + hui::KEY_CONTROL, "hui:cut");
	MainWin->event("copy", [=]{ on_copy(); });
	MainWin->set_key_code("copy", hui::KEY_C + hui::KEY_CONTROL, "hui:copy");
	MainWin->event("paste", [=]{ on_paste(); });
	MainWin->set_key_code("paste", hui::KEY_V + hui::KEY_CONTROL, "hui:paste");
	MainWin->event("comment", [=]{ on_comment(); });
	MainWin->set_key_code("comment", hui::KEY_D + hui::KEY_CONTROL, "");
	MainWin->event("uncomment", [=]{ on_uncomment(); });
	MainWin->set_key_code("uncomment", hui::KEY_D + hui::KEY_CONTROL + hui::KEY_SHIFT, "");
	MainWin->event("indent", [=]{ on_indent(); });
	MainWin->set_key_code("indent", hui::KEY_TAB + hui::KEY_CONTROL, "");
	MainWin->event("unindent", [=]{ on_unindent(); });
	MainWin->set_key_code("unindent", hui::KEY_TAB + hui::KEY_CONTROL + hui::KEY_SHIFT, "");
	MainWin->event("reload", [=]{ OnReload(); });
	MainWin->set_key_code("reload", hui::KEY_R + hui::KEY_CONTROL, "hui:reload");
	MainWin->event("undo", [=]{ on_undo(); });
	MainWin->set_key_code("undo", hui::KEY_Z + hui::KEY_CONTROL, "hui:undo");
	MainWin->event("redo", [=]{ on_redo(); });
	MainWin->set_key_code("redo", hui::KEY_Z + hui::KEY_SHIFT + hui::KEY_CONTROL, "hui:redo");
	MainWin->event("compile", [=]{ Compile(); });
	MainWin->set_key_code("compile", hui::KEY_F7);
	MainWin->event("compile_and_run_verbose", [=]{ OnCompileAndRunVerbose(); });
	MainWin->set_key_code("compile_and_run_verbose", hui::KEY_F6 + hui::KEY_CONTROL);
	MainWin->event("compile_and_run", [=]{ OnCompileAndRunSilent(); });
	MainWin->set_key_code("compile_and_run", hui::KEY_F6);
	MainWin->event("auto-complete", [=]{ OnAutoComplete(); });
	MainWin->set_key_code("auto-complete", hui::KEY_CONTROL + hui::KEY_SPACE);
	MainWin->event("settings", [=]{ ExecuteSettingsDialog(); });
	//MainWin->event("script_help", "hui:help", hui::KEY_F1 + hui::KEY_SHIFT);
	MainWin->event("next_document", [=]{ OnNextDocument(); });
	MainWin->set_key_code("next_document", hui::KEY_PAGE_DOWN + hui::KEY_CONTROL, "hui:down");
	MainWin->event("prev_document", [=]{ OnPreviousDocument(); });
	MainWin->set_key_code("prev_document", hui::KEY_PAGE_UP + hui::KEY_CONTROL, "hui:up");

	MainWin->event("show_cur_line", [=]{ ShowCurLine(); });
	MainWin->set_key_code("show_cur_line", hui::KEY_F2);

	for (int i=0; i<100; i++)
		MainWin->event("auto-complete-" + i2s(i), [this,i]{ OnInsertAutoComplete(i); });

	//console = new Console;
	//MainWin->embed(console, "table_main", 0, 1);
	//console->show(false);

	InitParser();
	HighlightScheme::default_scheme = HighlightScheme::get(hui::config.get_str("HighlightScheme", "default"));

	MainWin->set_maximized(maximized);
	MainWin->show();

	MainWin->event_x("file_list", "hui:select", [=]{ OnFileList(); });
	MainWin->event_x("function_list", "hui:select", [=]{ OnFunctionList(); });


	if (arg.num > 1) {
		for (int i=1; i<arg.num; i++)
			LoadFromFile(Path(arg[i]).absolute().canonical());
	} else {
		New();
	}
	return true;
}


void SgribthMaker::OnAbout() {
	about_box(MainWin);
}

void SgribthMaker::OnExit() {
	AllowTermination([this] {
		int w, h;
		MainWin->get_size_desired(w, h);
		hui::config.set_int("Window.Width", w);
		hui::config.set_int("Window.Height", h);
		hui::config.set_bool("Window.Maximized", MainWin->is_maximized());
		delete MainWin;
		end();
	}, []{});
}

HUI_EXECUTE(SgribthMaker)
