#include "SgribthMakerWindow.h"
#include "SgribthMaker.h"
#include "lib/base/base.h"
#include "lib/hui/hui.h"
#include "lib/os/filesystem.h"
#include "lib/os/msg.h"
#include "dialog/SettingsDialog.h"
#include "dialog/CommandDialog.h"
#include "Console.h"
#include "History.h"
#include "HighlightScheme.h"
#include "SourceView.h"
#include "parser/BaseParser.h"
#include "Document.h"
#include "AutoComplete.h"
#include "CodeCompiler.h"

extern string AppTitle;

SgribthMakerWindow::SgribthMakerWindow() :
	obs::Node<hui::Window>("", 800, 600),
	in_update(this, [this] { update_menu(); })
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


	event("about", [this] { on_about(); });
	event("hui:close", [this] { on_exit(); });

	event("new", [this] { create_new_document(); });
	set_key_code("new", hui::KEY_N + hui::KEY_CONTROL, "hui:new");
	//hui::HuiAddKeyCode(HMM_NEW_HEX, hui::KEY_F1 + 256);
	event("open", [this] { on_open(); });
	set_key_code("open", hui::KEY_O + hui::KEY_CONTROL, "hui:open");
	//hui::HuiAddKeyCode(HMM_OPEN_HEX, hui::KEY_F9 + 256);
	event("save", [this] { on_save(); });
	set_key_code("save", hui::KEY_S + hui::KEY_CONTROL, "hui:save");
	event("save_as", [this] { on_save_as(); });
	set_key_code("save_as", hui::KEY_S + hui::KEY_SHIFT + hui::KEY_CONTROL, "hui:save_as");
	event("close", [this] { on_close_document(); });
	set_key_code("close", hui::KEY_W + hui::KEY_CONTROL, "hui:close");
	event("exit", [this] { on_exit(); });
	set_key_code("exit", hui::KEY_Q + hui::KEY_CONTROL, "hui:quit");
	//event("show_data", "", hui::KEY_D + hui::KEY_CONTROL, &ShowData);
	event("execute_command", [this] { execute_command_dialog(); });
	set_key_code("execute_command", hui::KEY_E + hui::KEY_CONTROL, "");
	event("find", [this] { execute_command_dialog(); });
	set_key_code("find", hui::KEY_F + hui::KEY_CONTROL, "");
	event("cut", [this] { on_cut(); });
	set_key_code("cut", hui::KEY_X + hui::KEY_CONTROL, "hui:cut");
	event("copy", [this] { on_copy(); });
	set_key_code("copy", hui::KEY_C + hui::KEY_CONTROL, "hui:copy");
	event("paste", [this] { on_paste(); });
	set_key_code("paste", hui::KEY_V + hui::KEY_CONTROL, "hui:paste");
	event("comment", [this] { on_comment(); });
	set_key_code("comment", hui::KEY_D + hui::KEY_CONTROL, "");
	event("uncomment", [this] { on_uncomment(); });
	set_key_code("uncomment", hui::KEY_D + hui::KEY_CONTROL + hui::KEY_SHIFT, "");
	event("indent", [this] { on_indent(); });
	set_key_code("indent", hui::KEY_TAB + hui::KEY_CONTROL, "");
	event("unindent", [this] { on_unindent(); });
	set_key_code("unindent", hui::KEY_TAB + hui::KEY_CONTROL + hui::KEY_SHIFT, "");
	event("reload", [this] { on_reload(); });
	set_key_code("reload", hui::KEY_R + hui::KEY_CONTROL, "hui:reload");
	event("undo", [this] { on_undo(); });
	set_key_code("undo", hui::KEY_Z + hui::KEY_CONTROL, "hui:undo");
	event("redo", [this] { on_redo(); });
	set_key_code("redo", hui::KEY_Z + hui::KEY_SHIFT + hui::KEY_CONTROL, "hui:redo");
	event("compile", [this] { on_compile(); });
	set_key_code("compile", hui::KEY_F7);
	event("compile_and_run_verbose", [this] { on_compile_and_run_verbose(); });
	set_key_code("compile_and_run_verbose", hui::KEY_F6 + hui::KEY_CONTROL);
	event("compile_and_run", [this] { on_compile_and_run_silent(); });
	set_key_code("compile_and_run", hui::KEY_F6);
	event("auto-complete", [this] { on_auto_complete(); });
	set_key_code("auto-complete", hui::KEY_CONTROL + hui::KEY_SPACE);
	event("settings", [this] { execute_settings_dialog(); });
	//event("script_help", "hui:help", hui::KEY_F1 + hui::KEY_SHIFT);
	event("next_document", [this] { on_next_document(); });
	set_key_code("next_document", hui::KEY_PAGE_DOWN + hui::KEY_CONTROL, "hui:down");
	event("prev_document", [this] { on_previous_document(); });
	set_key_code("prev_document", hui::KEY_PAGE_UP + hui::KEY_CONTROL, "hui:up");

	event("show_cur_line", [this] { show_cur_line(); });
	set_key_code("show_cur_line", hui::KEY_F2);

	for (int i=0; i<100; i++)
		event("auto-complete-" + i2s(i), [this,i]{ on_insert_auto_complete(i); });

	//console = new Console;
	//embed(console, "table_main", 0, 1);
	//console->show(false);

	show();

	event_x("file_list", "hui:select", [this] { on_file_list(); });
	event_x("function_list", "hui:select", [this] { on_function_list(); });
	event("info-close", [this] { hide_control("grid-info", true); });
}

void SgribthMakerWindow::update_status_bar() {
	status_count --;
	if (status_count == 0)
		hide_control("grid-info", true);
		//enable_statusbar(false);
}

void SgribthMakerWindow::set_message(const string &str) {
	/*set_status_text(str);
	enable_statusbar(true);
	status_count ++;
	hui::RunLater(5, [this] { UpdateStatusBar(); });*/
	if (str.num >= 20)
		set_info(str);
}

void SgribthMakerWindow::set_error(const string &str) {
	//hui::error_box(MainWin, "error", str);
	//set_info_text(str, {"error", "allow-close"});
	//set_string("info", "!bold\\\u26A0 " + str);
	set_string("error", "!bold\\Error: " + str);
	hide_control("grid-info", false);
	hide_control("info", true);
	hide_control("error", false);
	status_count = -1;
}

void SgribthMakerWindow::set_info(const string &str) {
	set_string("info", "!bold\\" + str);
	hide_control("grid-info", false);
	hide_control("info", false);
	hide_control("error", true);
	//set_info_text(str, {"info", "allow-close"});
	status_count = max(0, status_count) + 1;
	hui::run_later(5, [this] { update_status_bar(); });
}

void SgribthMakerWindow::set_window_title() {
	if (!cur_view)
		return;
	set_title(cur_doc()->name(true) + " - " + AppTitle);
}

void SgribthMakerWindow::update_doc_list() {
	reset("file_list");
	foreachi(SourceView *v, source_views, i) {
		add_string("file_list", v->doc->name(false));
		if (cur_view == v)
			set_int("file_list", i);
	}
}

void SgribthMakerWindow::update_menu() {
	enable("undo", cur_doc()->history->undoable());
	enable("redo", cur_doc()->history->redoable());
	//enable("save", cur_doc()->history->changed);
	update_doc_list();
	set_window_title();
}

void SgribthMakerWindow::update_function_list() {
	reset("function_list");
	if (!cur_doc()->parser)
		return;
	auto labels = cur_doc()->parser->FindLabels(cur_view);
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

void SgribthMakerWindow::set_active_view(SourceView *view) {
	foreachi(SourceView *vv, source_views, i)
		if (vv == view) {
			set_int("tab", i);
			activate(view->id);
			break;
		}
	cur_view = view;
	update_menu();
	update_function_list();
}

void SgribthMakerWindow::allow_termination(const hui::Callback &on_success, const hui::Callback &on_fail) {
	Array<Document*> unsaved;
	for (auto v: source_views)
		if (v->history->changed)
			unsaved.add(v->doc);
	if (unsaved.num == 0) {
		on_success();
	} else if (unsaved.num == 1) {
		set_active_view(unsaved[0]->source_view);
		hui::question_box(this, _("respectful question"), _("You increased entropy. Do you wish to save your work?"), [this,unsaved,on_success,on_fail] (const string &answer) {
			if (answer == "hui:cancel")
				on_fail();
			else if (answer == "hui:yes")
				save(unsaved[0], on_success, on_fail);
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

void SgribthMakerWindow::allow_doc_termination(Document *d, const hui::Callback &on_success, const hui::Callback &on_fail) {
	if (d->history->changed) {
		set_active_view(d->source_view);
		hui::question_box(this, _("respectful question"), _("You increased entropy. Do you wish to save your work?"), [this,d,on_success,on_fail] (const string &answer) {
			if (answer == "hui:cancel")
				on_fail();
			else if (answer == "hui:yes")
				save(d, on_success, on_fail);
			else
				on_success();
		}, true);
	} else {
		on_success();
	}
}

SourceView* SgribthMakerWindow::create_new_document() {
	if (source_views.num > 0)
		hide_control("table_side", false);

	string id = format("edit-%06d", randi(1000000));

	set_border_width(0);
	add_string("tab", i2s(source_views.num));
	set_target("tab");
	add_grid("", source_views.num, 0, id + "-grid");
	set_target(id + "-grid");
	if (hui::config.get_bool("ShowLineNumbers", false)) {
		add_multiline_edit("!noframe,disabled,width=70,noexpandx", 0, 0, id + "-lines");
		enable(id + "-lines", false);
	}
	add_multiline_edit("!handlekeys,noframe", 1, 0, id);

	auto doc = new Document(this);
	doc->compiler = new CodeCompiler(doc);
	SourceView *view = new SourceView(this, id, doc);

	view->history->out_changed >> in_update;
	doc->out_changed >> in_update;
	doc->out_not_utf8 >> create_sink([this] { set_error("File is not utf8 compatible"); });

	view->apply_scheme(HighlightScheme::default_scheme);
	source_views.add(view);

	set_active_view(view);
	update_menu();
	return view;
}

Document* SgribthMakerWindow::cur_doc() const {
	return cur_view->doc;
}

void SgribthMakerWindow::on_close_document() {
	allow_doc_termination(cur_doc(), [this] {
		if (source_views.num <= 1) {
			on_exit();
			return;
		}

		int n = get_int("tab");
		hui::run_later(0.001f, [this, n] {
			remove_string("tab", n);
			delete source_views[n];
			source_views.erase(n);

			set_active_view(source_views.back());
			update_menu();
		});
	}, [] {});
}

bool SgribthMakerWindow::load_from_file(const Path &filename) {
	auto view = create_new_document();
	auto ok = view->doc->load(filename);
	if (!ok)
		set_message(_("Can not open file"));
	return ok;
}

bool SgribthMakerWindow::write_to_file(Document *doc, const Path &filename) {
	bool ok = doc->save(filename);
	if (ok)
		set_message(_("saved"));
	else
		set_message(_("Can not save file"));
	return ok;
}

Path working_dir_from_doc(Document *d) {
	if (d->filename.is_empty())
		return os::fs::current_directory();
	return d->filename.parent();
}

void SgribthMakerWindow::open() {
	hui::file_dialog_open(this, _("Open file"), working_dir_from_doc(cur_doc()), {"showfilter="+_("All (*.*)"), "filter=*"}, [this] (const Path &filename) {
		if (filename)
			load_from_file(filename);
	});
}

void SgribthMakerWindow::save_as(Document *doc, const hui::Callback &on_success, const hui::Callback &on_fail) {
	hui::file_dialog_save(this, _("Save file"), working_dir_from_doc(doc), {"showfilter=" + _("All (*.*)"), "filter=*"}, [this,doc,on_success,on_fail] (const Path &filename) {
		if (filename) {
			if (write_to_file(doc, filename))
				on_success();
			else
				on_fail();
		}
	});
}

void SgribthMakerWindow::save(Document *doc, const hui::Callback &on_success, const hui::Callback &on_fail) {
	if (!doc->filename.is_empty()) {
		if (write_to_file(doc, doc->filename))
			on_success();
		else
			on_fail();
	} else {
		save_as(doc, on_success, on_fail);
	}
}

void SgribthMakerWindow::on_open() {
	open();
}

void SgribthMakerWindow::on_save() {
	save(cur_doc(), []{}, []{});
}

void SgribthMakerWindow::on_save_as() {
	save_as(cur_doc(), []{}, []{});
}

void SgribthMakerWindow::reload(Document *doc) {
	allow_doc_termination(doc, [this, doc] {
		if (doc->filename) {
			if (doc->load(doc->filename))
				set_message(_("reloaded"));
			else
				set_message(_("failed reloading"));
		}
	}, []{});
}

void SgribthMakerWindow::on_reload() {
	reload(cur_doc());
}

void SgribthMakerWindow::on_undo() {
	cur_doc()->history->undo();
}

void SgribthMakerWindow::on_redo()
{	cur_doc()->history->redo();	}

void SgribthMakerWindow::on_copy() {
	hui::clipboard::copy(cur_view->get_selection_content());
	set_message(_("copied"));
}

void SgribthMakerWindow::on_paste() {
	msg_write("SM.on paste");
	cur_view->delete_selection();
	msg_write("x");
	auto p = hui::clipboard::paste();
	msg_write("y " + i2s(p.num));
	cur_view->insert_at_cursor(p);//hui::clipboard::paste());
	msg_write("/SM.on paste");
	set_message(_("pasted"));
}

void SgribthMakerWindow::prepend_selected_lines(const string &s) {
	auto sv = cur_view;
	int pos0, pos1;
	sv->get_selection(pos0, pos1);
	int l0 = sv->get_line_no_at(pos0);
	int l1 = sv->get_line_no_at(pos1);
	for (int l = l0; l<=l1; l++) {
		sv->insert_at(sv->get_line_offset(l), s);
	}
}

void SgribthMakerWindow::unprepend_selected_lines(const string &s) {
	auto sv = cur_view;
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
	prepend_selected_lines(cur_doc()->parser->line_comment_begin);
}

void SgribthMakerWindow::on_uncomment() {
	unprepend_selected_lines(cur_doc()->parser->line_comment_begin);
}

void SgribthMakerWindow::on_indent() {
	auto sv = cur_view;
	if (sv->has_selection())
		prepend_selected_lines("\t");
	else
		sv->insert_at_cursor("\t");
}

void SgribthMakerWindow::on_unindent() {
	unprepend_selected_lines("\t");
}

void SgribthMakerWindow::on_delete() {
	cur_view->delete_selection();
}

void SgribthMakerWindow::on_cut() {
	on_copy();
	on_delete();
}

void SgribthMakerWindow::on_compile() {
	save(cur_doc(), [this] {
		cur_doc()->compiler->compile();
	}, []{});
}

void SgribthMakerWindow::on_compile_and_run_verbose() {
	cur_doc()->compiler->compile_and_run(true);
}

void SgribthMakerWindow::on_compile_and_run_silent() {
	cur_doc()->compiler->compile_and_run(false);
}


static AutoComplete::Data _auto_complete_data_;

void SgribthMakerWindow::on_insert_auto_complete(int n) {
	if ((n >= 0) and (n < _auto_complete_data_.suggestions.num))
		cur_view->insert_at_cursor(_auto_complete_data_.suggestions[n].name.sub(_auto_complete_data_.offset));
}

void SgribthMakerWindow::on_auto_complete() {
	if (cur_doc()->filename.extension() != "kaba") {
		set_error(_("auto-completion only available for *.kaba files!"));
		return;
	}

	os::fs::set_current_directory(cur_doc()->filename.parent());

	int line, pos;
	cur_view->get_cur_line_pos(line, pos);
	auto data = AutoComplete::run(cur_view->get_all(), cur_doc()->filename, line, pos);
	_auto_complete_data_ = data;

	if (data.suggestions.num == 1) {
		cur_view->insert_at_cursor(data.suggestions[0].name.sub(data.offset));
		set_message(data.suggestions[0].context);

	} else if (data.suggestions.num > 1) {
		auto *m = new hui::Menu(this);
		foreachi (auto &s, data.suggestions, i)
			m->add(s.name, "auto-complete-" + i2s(i));
		m->open_popup(this);
	} else {
		set_message(_("????"));
	}
}


void SgribthMakerWindow::show_cur_line() {
	int line, off;
	cur_view->get_cur_line_pos(line, off);
	set_message(format(_("Line  %d : %d"), line + 1, off + 1));
}

void SgribthMakerWindow::execute_command(const string &cmd) {
	bool found = cur_view->find(cmd);
	if (!found)
		set_error(format(_("\"%s\" not found"), cmd.c_str()));
}

void SgribthMakerWindow::execute_command_dialog() {
	hui::fly(new CommandDialog(this));
}

void SgribthMakerWindow::execute_settings_dialog() {
	hui::fly(new SettingsDialog(this));
}



void SgribthMakerWindow::on_function_list() {
	int n = get_int("");
	auto labels = cur_doc()->parser->FindLabels(cur_view);
	if ((n >= 0) and (n < labels.num)) {
		cur_view->show_line_on_screen(labels[n].line);
		activate(cur_view->id);
	}
}

void SgribthMakerWindow::on_file_list() {
	int s = get_int("");
	if (s >= 0)
		set_active_view(source_views[s]);
}

void SgribthMakerWindow::on_next_document() {
	foreachi(auto v, source_views, i)
		if (v == cur_view) {
			if (i < source_views.num - 1)
				set_active_view(source_views[i + 1]);
			else
				set_active_view(source_views[0]);
			break;
		}
}

void SgribthMakerWindow::on_previous_document() {
	foreachi(auto v, source_views, i)
		if (v == cur_view) {
			if (i > 0)
				set_active_view(source_views[i - 1]);
			else
				set_active_view(source_views.back());
			break;
		}
}

void SgribthMakerWindow::on_about() {
	about_box(this);
}

void SgribthMakerWindow::on_exit() {
	allow_termination([this] {
		int w, h;
		get_size_desired(w, h);
		hui::config.set_int("Window.Width", w);
		hui::config.set_int("Window.Height", h);
		hui::config.set_bool("Window.Maximized", is_maximized());
		request_destroy();
		sgribthmaker->end();
	}, []{});
}

