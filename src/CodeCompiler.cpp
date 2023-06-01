/*
 * CodeCompiler.cpp
 *
 *  Created on: 1 Jun 2023
 *      Author: michi
 */

#include "CodeCompiler.h"
#include "Document.h"
#include "SgribthMakerWindow.h"
#include "SourceView.h"
#include "Console.h"
#include "lib/os/time.h"
#include "lib/os/filesystem.h"
#include "lib/kaba/kaba.h"
#include "lib/nix/nix.h"


namespace nix {
	extern string shader_error;
}

CodeCompiler::CodeCompiler(Document *_doc) {
	doc = _doc;
}

CodeCompiler::~CodeCompiler() {
}

void CodeCompiler::compile() {
	string ext = doc->filename.extension();
	if (ext == "kaba")
		compile_kaba();
	else if (ext == "shader")
		compile_shader();
	else
		doc->win->set_error(_("only *.kaba and *.shader files can be compiled!"));
}

string get_time_str(float t) {
	if (t < 0.3f)
		return format("%.2fms", t * 1000.0f);
	else
		return format("%.2fs", t);
}

void CodeCompiler::compile_kaba() {

	//HuiSetDirectory(SgribthDir);
	//msg_set_verbose(true);

	os::Timer timer;

	kaba::config.compile_silently = true;
	//kaba::config.verbose = true;

	auto context = ownify(kaba::Context::create());

	try {
		auto module = context->load_module(doc->filename, true);

		float dt = timer.get();

		//compile_module->Show();

		doc->win->set_message(format(_("Script compiles without errors!         (in %s)"), get_time_str(dt)));

	} catch (const kaba::Exception &e) {
		e.print();
		//ErrorBox(MainWin, _("Error"), e.message());
		doc->win->set_error(e.message());
		doc->source_view->move_cursor_to(e.line, e.column);
	}

	//msg_set_verbose(ALLOW_LOGGING);
}

void CodeCompiler::compile_shader() {
	return;

	// NOPE, not working
	hui::run_later(0.01f, [this] {

	auto *w = new hui::NixWindow("nix", 640, 480);
	//w->add_drawing_area("!opengl", 0, 0, "nix-area");
	w->event_x("nix-area", "realize", [this] {
		nix::init();
	//	nix::init("OpenGL", w, "nix-area");

		auto *shader = nix::Shader::load(doc->filename);
		if (!shader) {
			doc->win->set_error(nix::shader_error);
		} else {
			doc->win->set_message(_("Shader compiles without error!"));
			//shader->unref();
		}
	});


	w->show();
	delete w;

	//msg_set_verbose(ALLOW_LOGGING);
	});
}


void CodeCompiler::compile_and_run(bool verbose) {
	if (doc->filename.extension() != "kaba") {
		doc->win->set_error(_("only *.kaba files can be executed!"));
		return;
	}

	doc->win->save(doc, [this,verbose] {

		os::fs::set_current_directory(doc->filename.parent());
		//if (verbose)
		//	msg_set_verbose(true);

		// compile
		os::Timer timer;
		kaba::config.compile_silently = true;
		//kaba::config.verbose = true;

		auto context = ownify(kaba::Context::create());

		try {
			auto module = context->load_module(doc->filename);
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
				doc->win->set_message(format(_("Compiling: %s         opcode: %db         execution: %s"), get_time_str(dt_compile), module->opcode_size, get_time_str(dt_execute)));
			} else {
				doc->win->set_error(_("no 'void main()' or 'void main(string[])' found"));
			}
			//if (verbose)
			//	HuiInfoBox(MainWin,"Speicher",string("nicht freigegebener Speicher des Scriptes: ",i2s(script->MemoryUsed),"b"));}

			// messages? (not working anymore, since kaba is not writing to log)
			int msg_size = msg_get_buffer_size();
			if (msg_size > msg_size0)
				doc->win->console->set(msg_get_buffer(msg_size - msg_size0));

		} catch(const kaba::Exception &e) {
			e.print();

			doc->win->set_error(e.message());
			//ErrorBox(MainWin, _("Error"), e.message());
			doc->source_view->move_cursor_to(e.line, e.column);
		}

		//msg_set_verbose(ALLOW_LOGGING);
	}, []{});
}

