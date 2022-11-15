/*----------------------------------------------------------------------------*\
| Kaba                                                                         |
| -> C-like scripting system                                                   |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last updated: 2010.07.07 (c) by MichiSoft TM                                 |
\*----------------------------------------------------------------------------*/
#include "../os/file.h"
#include "../os/msg.h"
#include "../base/algo.h"
#include "kaba.h"
#include "parser/Parser.h"
#include "parser/Concretifier.h"
#include "Interpreter.h"

namespace kaba {

string Version = "0.19.22.11";

//#define ScriptDebug


shared_array<Module> public_modules;




Path absolute_module_path(const Path &filename) {
	if (filename.is_relative())
		return (config.directory << filename).absolute().canonical();
	else
		return filename.absolute().canonical();
}


shared<Module> load(const Path &filename, bool just_analyse) {
	//msg_write("loading " + filename.str());

	auto _filename = absolute_module_path(filename);

	// already loaded?
	for (auto ps: public_modules)
		if (ps->filename == _filename)
			return ps;
	
	// load
	shared<Module> s = new Module();
	s->load(filename, just_analyse);

	// store module in database
	public_modules.add(s);
	return s;
}

shared<Module> create_for_source(const string &buffer, bool just_analyse) {
	shared<Module> s = new Module;
	s->just_analyse = just_analyse;
	s->filename = config.default_filename;
	s->syntax->parser = new Parser(s->syntax);
	s->syntax->default_import();
	s->syntax->parser->parse_buffer(buffer, just_analyse);

	if (!just_analyse)
		s->compile();
	return s;
}

void remove_module(Module *s) {

	// remove from normal list
	for (int i=0;i<public_modules.num;i++)
		if (public_modules[i] == s)
			public_modules.erase(i);
}

void delete_all_modules(bool even_immortal, bool force) {
	public_modules.clear();

#if 0
	// try to erase them...
	auto to_del = public_modules;
	foreachb(Module *s, to_del)
		if ((!s->syntax->flag_immortal) or even_immortal)
			Remove(s);

	// undead... really KILL!
	if (force){
		foreachb(Module *s, _dead_scripts_)
			delete(s);
		_dead_scripts_.clear();
	}

	//ScriptResetSemiExternalData();

	
	/*msg_write("------------------------------------------------------------------");
	msg_write(mem_used);
	for (int i=0;i<num_ps;i++)
		msg_write(string2("  fehlt:   %s  %p  (%d)",ppn[i],ppp[i],pps[i]));
	*/
#endif
}

VirtualTable* get_vtable(const VirtualBase *p) {
	return *(VirtualTable**)p;
}

const Class *_dyn_type_in_namespace(const VirtualTable *p, const Class *ns) {
	for (auto *c: weak(ns->classes)) {
		if (c->_vtable_location_target_ == p)
			return c;
		auto t = _dyn_type_in_namespace(p, c);
		if (t)
			return t;
	}
	return nullptr;
}

// TODO...namespace
const Class *get_dynamic_type(const VirtualBase *p) {
	auto *pp = get_vtable(p);
	for (auto s: public_modules) {
		auto t = _dyn_type_in_namespace(pp, s->syntax->base_class);
		if (t)
			return t;
	}
	return nullptr;
}


// bad:  should clean up in case of errors!
void execute_single_command(const string &cmd) {
	if (cmd.num < 1)
		return;
	//msg_write("command: " + cmd);

	// empty module
	shared<Module> s = new Module();
	s->filename = "-command line-";
	auto tree = s->syntax;
	tree->default_import();
	auto parser = new Parser(tree);
	tree->parser = parser;

// find expressions
	parser->Exp.analyse(tree, cmd);
	if (parser->Exp.empty()) {
		//clear_exp_buffer(&ps->Exp);
		return;
	}
	
	for (auto p: packages)
		if (!p->used_by_default)
			tree->import_data(p, true, "");

// analyse syntax

	// create a main() function
	Function *func = tree->add_function("--command-func--", TypeVoid, tree->base_class, Flags::STATIC);
	func->_var_size = 0; // set to -1...

	parser->Exp.reset_walker();

	// parse
	func->block->type = TypeUnknown;
	parser->parse_abstract_complete_command(func->block.get());
	if (config.verbose) {
		msg_write("ABSTRACT SINGLE:");
		func->block->show();
	}
	parser->con.concretify_node(func->block.get(), func->block.get(), func->name_space);
	
	// implicit print(...)?
	if (func->block->params.num > 0 and func->block->params[0]->type != TypeVoid) {
		auto n = parser->con.add_converter_str(func->block->params[0], true);
		
		auto f = tree->required_func_global("print");

		auto cmd = add_node_call(f);
		cmd->set_param(0, n);
		func->block->params[0] = cmd;
	}
	for (auto *c: tree->owned_classes)
		parser->auto_implementer.auto_implement_functions(c);
	//ps->show("aaaa");


	if (config.verbose)
		tree->show("parse:a");

// compile
	s->compile();


	if (config.interpreted) {
		s->interpreter->run("--command-func--");
		return;
	}

// execute
	if (config.abi == config.native_abi) {
		typedef void void_func();
		void_func *f = (void_func*)func->address;
		if (f)
			f();
	}
}

};
