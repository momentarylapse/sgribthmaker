#include "SgribthMakerWindow.h"
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

extern string AppTitle;

namespace nix {
	extern string shader_error;
}

SgribthMakerWindow::SgribthMakerWindow() :
	hui::Window("", 800, 600)
{
	console = nullptr;
	int width = hui::config.get_int("Window.Width", 800);
	int height = hui::config.get_int("Window.Height", 600);
	bool maximized = hui::config.get_bool("Window.Maximized", false);

	set_size(width, height);
	set_maximized(maximized);


	from_resource("main-window");
	set_menu(hui::create_resource_menu("menu", this));
	toolbar[0]->set_by_id("toolbar");


	event("about", [=]{ OnAbout(); });
	event("hui:close", [=]{ OnExit(); });

	event("new", [=]{ New(); });
	set_key_code("new", hui::KEY_N + hui::KEY_CONTROL, "hui:new");
	//hui::HuiAddKeyCode(HMM_NEW_HEX, hui::KEY_F1 + 256);
	event("open", [=]{ OnOpen(); });
	set_key_code("open", hui::KEY_O + hui::KEY_CONTROL, "hui:open");
	//hui::HuiAddKeyCode(HMM_OPEN_HEX, hui::KEY_F9 + 256);
	event("save", [=]{ OnSave(); });
	set_key_code("save", hui::KEY_S + hui::KEY_CONTROL, "hui:save");
	event("save_as", [=]{ OnSaveAs(); });
	set_key_code("save_as", hui::KEY_S + hui::KEY_SHIFT + hui::KEY_CONTROL, "hui:save_as");
	event("close", [=]{ OnCloseDocument(); });
	set_key_code("close", hui::KEY_W + hui::KEY_CONTROL, "hui:close");
	event("exit", [=]{ OnExit(); });
	set_key_code("exit", hui::KEY_Q + hui::KEY_CONTROL, "hui:quit");
	//event("show_data", "", hui::KEY_D + hui::KEY_CONTROL, &ShowData);
	event("execute_command", [=]{ ExecuteCommandDialog(); });
	set_key_code("execute_command", hui::KEY_E + hui::KEY_CONTROL, "");
	event("find", [=]{ ExecuteCommandDialog(); });
	set_key_code("find", hui::KEY_F + hui::KEY_CONTROL, "");
	event("cut", [=]{ on_cut(); });
	set_key_code("cut", hui::KEY_X + hui::KEY_CONTROL, "hui:cut");
	event("copy", [=]{ on_copy(); });
	set_key_code("copy", hui::KEY_C + hui::KEY_CONTROL, "hui:copy");
	event("paste", [=]{ on_paste(); });
	set_key_code("paste", hui::KEY_V + hui::KEY_CONTROL, "hui:paste");
	event("comment", [=]{ on_comment(); });
	set_key_code("comment", hui::KEY_D + hui::KEY_CONTROL, "");
	event("uncomment", [=]{ on_uncomment(); });
	set_key_code("uncomment", hui::KEY_D + hui::KEY_CONTROL + hui::KEY_SHIFT, "");
	event("indent", [=]{ on_indent(); });
	set_key_code("indent", hui::KEY_TAB + hui::KEY_CONTROL, "");
	event("unindent", [=]{ on_unindent(); });
	set_key_code("unindent", hui::KEY_TAB + hui::KEY_CONTROL + hui::KEY_SHIFT, "");
	event("reload", [=]{ OnReload(); });
	set_key_code("reload", hui::KEY_R + hui::KEY_CONTROL, "hui:reload");
	event("undo", [=]{ on_undo(); });
	set_key_code("undo", hui::KEY_Z + hui::KEY_CONTROL, "hui:undo");
	event("redo", [=]{ on_redo(); });
	set_key_code("redo", hui::KEY_Z + hui::KEY_SHIFT + hui::KEY_CONTROL, "hui:redo");
	event("compile", [=]{ Compile(); });
	set_key_code("compile", hui::KEY_F7);
	event("compile_and_run_verbose", [=]{ OnCompileAndRunVerbose(); });
	set_key_code("compile_and_run_verbose", hui::KEY_F6 + hui::KEY_CONTROL);
	event("compile_and_run", [=]{ OnCompileAndRunSilent(); });
	set_key_code("compile_and_run", hui::KEY_F6);
	event("auto-complete", [=]{ OnAutoComplete(); });
	set_key_code("auto-complete", hui::KEY_CONTROL + hui::KEY_SPACE);
	event("settings", [=]{ ExecuteSettingsDialog(); });
	//event("script_help", "hui:help", hui::KEY_F1 + hui::KEY_SHIFT);
	event("next_document", [=]{ OnNextDocument(); });
	set_key_code("next_document", hui::KEY_PAGE_DOWN + hui::KEY_CONTROL, "hui:down");
	event("prev_document", [=]{ OnPreviousDocument(); });
	set_key_code("prev_document", hui::KEY_PAGE_UP + hui::KEY_CONTROL, "hui:up");

	event("show_cur_line", [=]{ ShowCurLine(); });
	set_key_code("show_cur_line", hui::KEY_F2);

	for (int i=0; i<100; i++)
		event("auto-complete-" + i2s(i), [this,i]{ OnInsertAutoComplete(i); });

	//console = new Console;
	//embed(console, "table_main", 0, 1);
	//console->show(false);

	InitParser();
	HighlightScheme::default_scheme = HighlightScheme::get(hui::config.get_str("HighlightScheme", "default"));

	show();

	event_x("file_list", "hui:select", [=]{ OnFileList(); });
	event_x("function_list", "hui:select", [=]{ OnFunctionList(); });
	event("info-close", [this] { hide_control("grid-info", true); });
}

void SgribthMakerWindow::UpdateStatusBar() {
	status_count --;
	if (status_count == 0)
		hide_control("grid-info", true);
		//enable_statusbar(false);
}

void SgribthMakerWindow::SetMessage(const string &str) {
	/*set_status_text(str);
	enable_statusbar(true);
	status_count ++;
	hui::RunLater(5, [=]{ UpdateStatusBar(); });*/
	if (str.num >= 20)
		SetInfo(str);
}

void SgribthMakerWindow::SetError(const string &str) {
	//hui::error_box(MainWin, "error", str);
	//set_info_text(str, {"error", "allow-close"});
	//set_string("info", "!bold\\\u26A0 " + str);
	set_string("error", "!bold\\Error: " + str);
	hide_control("grid-info", false);
	hide_control("info", true);
	hide_control("error", false);
	status_count = -1;
}

void SgribthMakerWindow::SetInfo(const string &str) {
	set_string("info", "!bold\\" + str);
	hide_control("grid-info", false);
	hide_control("info", false);
	hide_control("error", true);
	//set_info_text(str, {"info", "allow-close"});
	status_count = max(0, status_count) + 1;
	hui::run_later(5, [this] { UpdateStatusBar(); });
}

void SgribthMakerWindow::SetWindowTitle() {
	if (!cur_doc)
		return;
	set_title(cur_doc->name(true) + " - " + AppTitle);
}

void SgribthMakerWindow::UpdateDocList() {
	reset("file_list");
	foreachi(Document *d, documents, i) {
		add_string("file_list", d->name(false));
		if (cur_doc == d)
			set_int("file_list", i);
	}
}

void SgribthMakerWindow::UpdateMenu() {
	enable("undo", cur_doc->history->undoable());
	enable("redo", cur_doc->history->redoable());
	//enable("save", cur_doc->history->changed);
	UpdateDocList();
	SetWindowTitle();
}

void SgribthMakerWindow::UpdateFunctionList() {
	reset("function_list");
	if (!cur_doc->parser)
		return;
	auto labels = cur_doc->parser->FindLabels(cur_doc->source_view);
	int last_parent = -1;
	foreachi(Parser::Label &l, labels, i) {
		if (l.level > 0) {
			add_child_string("function_list", last_parent, l.name);
		} else {
			last_parent = i;
			add_string("function_list", l.name);
		}
	}
}

void SgribthMakerWindow::SetActiveDocument(Document *d) {
	foreachi(Document *dd, documents, i)
		if (dd == d) {
			set_int("tab", i);
			activate(dd->source_view->id);
			break;
		}
	cur_doc = d;
	UpdateMenu();
	UpdateFunctionList();
}

void SgribthMakerWindow::AllowTermination(const hui::Callback &on_success, const hui::Callback &on_fail) {
	Array<Document*> unsaved;
	for (auto d: documents)
		if (d->history->changed)
			unsaved.add(d);
	if (unsaved.num == 0) {
		on_success();
	} else if (unsaved.num == 1) {
		SetActiveDocument(unsaved[0]);
		hui::question_box(this, _("respectful question"), _("You increased entropy. Do you wish to save your work?"), [this,unsaved,on_success,on_fail] (const string &answer) {
			if (answer == "hui:cancel")
				on_fail();
			else if (answer == "hui:yes")
				Save(unsaved[0], on_success, on_fail);
			else
				on_success();
		}, true);
	} else {
		hui::question_box(this, _("respectful question"), _("Several unsaved documents. Do you want to save before quitting?"), [this,unsaved,on_success,on_fail] (const string &answer) {
			if (answer == "hui:cancel")
				on_fail();
			else if (answer == "hui:yes")
				on_fail();
			else
				on_success();
		}, true);
	}
}

void SgribthMakerWindow::AllowDocTermination(Document *d, const hui::Callback &on_success, const hui::Callback &on_fail) {
	if (d->history->changed) {
		SetActiveDocument(d);
		hui::question_box(this, _("respectful question"), _("You increased entropy. Do you wish to save your work?"), [this,d,on_success,on_fail] (const string &answer) {
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

void SgribthMakerWindow::New() {
	if (documents.num > 0)
		hide_control("table_side", false);

	string id = format("edit-%06d", randi(1000000));

	set_border_width(0);
	add_string("tab", i2s(documents.num));
	set_target("tab");
	add_grid("", documents.num, 0, id + "-grid");
	set_target(id + "-grid");
	if (hui::config.get_bool("ShowLineNumbers", false)) {
		add_multiline_edit("!noframe,disabled,width=70,noexpandx", 0, 0, id + "-lines");
		enable(id + "-lines", false);
	}
	add_multiline_edit("!handlekeys,noframe", 1, 0, id);

	documents.add(new Document(this));
	SourceView *sv = new SourceView(this, id, documents.back());

	sv->apply_scheme(HighlightScheme::default_scheme);
	source_view.add(sv);

	SetActiveDocument(documents.back());
	UpdateMenu();
}

void SgribthMakerWindow::OnCloseDocument() {
	AllowDocTermination(cur_doc, [this] {
		if (documents.num <= 1) {
			OnExit();
			return;
		}

		int n = get_int("tab");
		hui::run_later(0.001f, [=] {
			remove_string("tab", n);
			delete documents[n];
			documents.erase(n);
			delete source_view[n];
			source_view.erase(n);

			SetActiveDocument(documents.back());
			UpdateMenu();
		});
	}, [] {});
}

bool SgribthMakerWindow::LoadFromFile(const Path &filename) {
	New();
	return documents.back()->load(filename);
}

bool SgribthMakerWindow::WriteToFile(Document *doc, const Path &filename) {
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

void SgribthMakerWindow::Open() {
	hui::file_dialog_open(this, _("Open file"), working_dir_from_doc(cur_doc), {"showfilter="+_("All (*.*)"), "filter=*"}, [this] (const Path &filename) {
		if (filename)
			LoadFromFile(filename);
	});
}

void SgribthMakerWindow::SaveAs(Document *doc, const hui::Callback &on_success, const hui::Callback &on_fail) {
	hui::file_dialog_save(this, _("Save file"), working_dir_from_doc(doc), {"showfilter=" + _("All (*.*)"), "filter=*"}, [this,doc,on_success,on_fail] (const Path &filename) {
		if (!filename.is_empty()) {
			if (WriteToFile(doc, filename))
				on_success();
			else
				on_fail();
		}
	});
}

void SgribthMakerWindow::Save(Document *doc, const hui::Callback &on_success, const hui::Callback &on_fail) {
	if (!doc->filename.is_empty()) {
		if (WriteToFile(doc, doc->filename))
			on_success();
		else
			on_fail();
	} else {
		SaveAs(doc, on_success, on_fail);
	}
}

void SgribthMakerWindow::OnOpen()
{	Open();	}

void SgribthMakerWindow::OnSave()
{	Save(cur_doc, []{}, []{});	}

void SgribthMakerWindow::OnSaveAs()
{	SaveAs(cur_doc, []{}, []{});	}

void SgribthMakerWindow::Reload() {
	AllowDocTermination(cur_doc, [this] {
		if (!cur_doc->filename.is_empty()) {
			bool r = cur_doc->load(cur_doc->filename);
			if (r)
				SetMessage(_("reloaded"));
		}
	}, []{});
}

void SgribthMakerWindow::OnReload()
{	Reload();	}

void SgribthMakerWindow::on_undo()
{	cur_doc->history->undo();	}

void SgribthMakerWindow::on_redo()
{	cur_doc->history->redo();	}

void SgribthMakerWindow::on_copy() {
	hui::clipboard::copy(cur_doc->source_view->get_selection_content());
	SetMessage(_("copied"));
}

void SgribthMakerWindow::on_paste() {
	msg_write("SM.on paste");
	cur_doc->source_view->delete_selection();
	msg_write("x");
	auto p = hui::clipboard::paste();
	msg_write("y " + i2s(p.num));
	cur_doc->source_view->insert_at_cursor(p);//hui::clipboard::paste());
	msg_write("/SM.on paste");
	SetMessage(_("pasted"));
}

void SgribthMakerWindow::prepend_selected_lines(const string &s) {
	auto sv = cur_doc->source_view;
	int pos0, pos1;
	sv->get_selection(pos0, pos1);
	int l0 = sv->get_line_no_at(pos0);
	int l1 = sv->get_line_no_at(pos1);
	for (int l = l0; l<=l1; l++) {
		sv->insert_at(sv->get_line_offset(l), s);
	}
}

void SgribthMakerWindow::unprepend_selected_lines(const string &s) {
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

void SgribthMakerWindow::on_comment() {
	prepend_selected_lines(cur_doc->parser->line_comment_begin);
}

void SgribthMakerWindow::on_uncomment() {
	unprepend_selected_lines(cur_doc->parser->line_comment_begin);
}

void SgribthMakerWindow::on_indent() {
	auto sv = cur_doc->source_view;
	if (sv->has_selection())
		prepend_selected_lines("\t");
	else
		sv->insert_at_cursor("\t");
}

void SgribthMakerWindow::on_unindent() {
	unprepend_selected_lines("\t");
}

void SgribthMakerWindow::on_delete() {
	cur_doc->source_view->delete_selection();
}

void SgribthMakerWindow::on_cut() {
	on_copy();
	on_delete();
}

string get_time_str(float t) {
	if (t < 0.3f)
		return format("%.2fms", t * 1000.0f);
	else
		return format("%.2fs", t);
}

void SgribthMakerWindow::CompileKaba() {

	//HuiSetDirectory(SgribthDir);
	//msg_set_verbose(true);

	os::Timer timer;

	kaba::config.compile_silently = true;
	//kaba::config.verbose = true;

	auto context = ownify(kaba::Context::create());

	try {
		auto module = context->load_module(cur_doc->filename, true);

		float dt = timer.get();

		//compile_module->Show();

		SetMessage(format(_("Script compiles without errors!         (in %s)"), get_time_str(dt)));

	} catch (const kaba::Exception &e) {
		e.print();
		//ErrorBox(MainWin, _("Error"), e.message());
		SetError(e.message());
		cur_doc->source_view->move_cursor_to(e.line, e.column);
	}

	//msg_set_verbose(ALLOW_LOGGING);
}

void SgribthMakerWindow::CompileShader() {
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

void SgribthMakerWindow::Compile() {
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

void SgribthMakerWindow::CompileAndRun(bool verbose) {
	if (cur_doc->filename.extension() != "kaba") {
		SetError(_("only *.kaba files can be executed!"));
		return;
	}

	Save(cur_doc, [this,verbose] {

		os::fs::set_current_directory(cur_doc->filename.parent());
		//if (verbose)
		//	msg_set_verbose(true);

		// compile
		os::Timer timer;
		kaba::config.compile_silently = true;
		//kaba::config.verbose = true;

		auto context = ownify(kaba::Context::create());

		try {
			auto module = context->load_module(cur_doc->filename);
			float dt_compile = timer.get();

			if (!verbose)
				msg_set_verbose(true);

			int msg_size0 = msg_get_buffer_size();


			float dt_execute = 0;
			timer.reset();
			typedef void void_func();
			typedef void strings_func(const Array<string>);
			auto f = (void_func*)module->match_function("main", "void", {});
			auto fsa = (strings_func*)module->match_function("main", "void", {"string[]"});
			if (f) {
				f();
			} else if (fsa) {
				fsa({});
			}
			//compile_module->ShowVars(false);
			dt_execute = timer.get();

			if (f or fsa) {
				SetMessage(format(_("Compiling: %s         opcode: %db         execution: %s"), get_time_str(dt_compile), module->opcode_size, get_time_str(dt_execute)));
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

		//msg_set_verbose(ALLOW_LOGGING);
	}, []{});
}

void SgribthMakerWindow::OnCompileAndRunVerbose()
{	CompileAndRun(true);	}

void SgribthMakerWindow::OnCompileAndRunSilent()
{	CompileAndRun(false);	}


static AutoComplete::Data _auto_complete_data_;

void SgribthMakerWindow::OnInsertAutoComplete(int n) {
	if ((n >= 0) and (n < _auto_complete_data_.suggestions.num))
		cur_doc->source_view->insert_at_cursor(_auto_complete_data_.suggestions[n].name.sub(_auto_complete_data_.offset));
}

void SgribthMakerWindow::OnAutoComplete() {
	if (cur_doc->filename.extension() != "kaba") {
		SetError(_("auto-completion only available for *.kaba files!"));
		return;
	}

	os::fs::set_current_directory(cur_doc->filename.parent());

	int line, pos;
	cur_doc->source_view->get_cur_line_pos(line, pos);
	auto data = AutoComplete::run(cur_doc->source_view->get_all(), cur_doc->filename, line, pos);
	_auto_complete_data_ = data;

	if (data.suggestions.num == 1) {
		cur_doc->source_view->insert_at_cursor(data.suggestions[0].name.sub(data.offset));
		SetMessage(data.suggestions[0].context);

	} else if (data.suggestions.num > 1) {
		auto *m = new hui::Menu(this);
		foreachi (auto &s, data.suggestions, i)
			m->add(s.name, "auto-complete-" + i2s(i));
		m->open_popup(this);
	} else {
		SetMessage(_("????"));
	}
}


void SgribthMakerWindow::ShowCurLine() {
	int line, off;
	cur_doc->source_view->get_cur_line_pos(line, off);
	SetMessage(format(_("Line  %d : %d"), line + 1, off + 1));
}

void SgribthMakerWindow::ExecuteCommand(const string &cmd) {
	bool found = cur_doc->source_view->find(cmd);
	if (!found)
		SetError(format(_("\"%s\" not found"), cmd.c_str()));
}

void SgribthMakerWindow::ExecuteCommandDialog() {
	hui::fly(new CommandDialog(this));
}

void SgribthMakerWindow::ExecuteSettingsDialog() {
	hui::fly(new SettingsDialog(this));
}



void SgribthMakerWindow::OnFunctionList() {
	int n = get_int("");
	auto labels = cur_doc->parser->FindLabels(cur_doc->source_view);
	if ((n >= 0) and (n < labels.num)) {
		cur_doc->source_view->show_line_on_screen(labels[n].line);
		activate(cur_doc->source_view->id);
	}
}

void SgribthMakerWindow::SgribthMakerWindow::OnFileList() {
	int s = get_int("");
	if (s >= 0)
		SetActiveDocument(documents[s]);
}

void SgribthMakerWindow::OnNextDocument() {
	foreachi(Document *d, documents, i)
		if (d == cur_doc) {
			if (i < documents.num - 1)
				SetActiveDocument(documents[i + 1]);
			else
				SetActiveDocument(documents[0]);
			break;
		}
}

void SgribthMakerWindow::OnPreviousDocument() {
	foreachi(Document *d, documents, i)
		if (d == cur_doc) {
			if (i > 0)
				SetActiveDocument(documents[i - 1]);
			else
				SetActiveDocument(documents.back());
			break;
		}
}

void SgribthMakerWindow::OnAbout() {
	about_box(this);
}

void SgribthMakerWindow::OnExit() {
	AllowTermination([this] {
		int w, h;
		get_size_desired(w, h);
		hui::config.set_int("Window.Width", w);
		hui::config.set_int("Window.Height", h);
		hui::config.set_bool("Window.Maximized", is_maximized());
		request_destroy();
		sgribthmaker->end();
	}, []{});
}

