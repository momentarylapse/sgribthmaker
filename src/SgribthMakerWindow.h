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

	obs::sink in_update;


	void on_about();
	void on_exit();

	void update_status_bar();
	void set_message(const string &str);
	void set_error(const string &str);
	void set_info(const string &str);
	void set_window_title();
	void update_doc_list();
	void update_menu();
	void update_function_list();
	void set_active_view(SourceView *view);
	void allow_termination(const hui::Callback &on_success = nullptr, const hui::Callback &on_fail = nullptr);
	void allow_doc_termination(Document *d, const hui::Callback &on_success = nullptr, const hui::Callback &on_fail = nullptr);
	void on_close_document();
	bool load_from_file(const Path &filename);
	bool write_to_file(Document *doc, const Path &filename);
	void open();

	SourceView* create_new_document();

	void save_as(Document *doc, const hui::Callback &on_success = nullptr, const hui::Callback &on_fail = nullptr);
	void save(Document *doc, const hui::Callback &on_success = nullptr, const hui::Callback &on_fail = nullptr);
	void on_open();
	void on_save();
	void on_save_as();
	void reload(Document *doc);
	void on_reload();
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
	void on_compile();
	void on_auto_complete();
	void on_insert_auto_complete(int n);
	void on_compile_and_run_verbose();
	void on_compile_and_run_silent();
	void show_cur_line();
	void execute_command(const string &cmd);
	void execute_command_dialog();
	void execute_settings_dialog();
	void on_function_list();
	void on_file_list();
	void on_next_document();
	void on_previous_document();


	Array<SourceView*> source_views;
	Console *console;

	SourceView *cur_view = nullptr;
	Document *cur_doc() const;

	int status_count = -1;
};

#endif /* SRC_SGRIBTHMAKERWINDOW_H_ */
