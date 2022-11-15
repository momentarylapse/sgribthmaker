/*
 * ParserKaba.cpp
 *
 *  Created on: 06.06.2013
 *      Author: michi
 */

#include "ParserKaba.h"
#include "../HighlightScheme.h"
#include "../lib/kaba/kaba.h"
#include "../SourceView.h"
#include "../Document.h"

void add_class(ParserKaba *p, const kaba::Class *c, const string &ns);

bool allowed(const string &s) {
	if (s == "filename" or s == "config")
		return false;
	return true;
}

void add_class_content(ParserKaba *p, const kaba::Class *c, const string &ns) {
	for (auto v: c->static_variables)
		if (allowed(ns + v->name))
			p->global_variables.add(ns + v->name);
	for (auto cc: c->constants)
		p->constants.add(ns + cc->name);
	for (auto f: c->functions)
		p->functions.add(ns + f->name);
	for (auto cc: weak(c->classes))
		add_class(p, cc, ns);
}

void add_class(ParserKaba *p, const kaba::Class *c, const string &ns) {
	p->types.add(ns + c->name);
	add_class_content(p, c, ns + c->name + ".");
}

ParserKaba::ParserKaba() : Parser("Kaba") {
	macro_begin = "#define";
	line_comment_begin = "#";
	string_sub_begin = "{{";
	string_sub_end = "}}";
	keywords.add(kaba::IDENTIFIER_ENUM);
	keywords.add(kaba::IDENTIFIER_CLASS);
	keywords.add(kaba::IDENTIFIER_FUNC);
	keywords.add(kaba::IDENTIFIER_EXTENDS);
	keywords.add(kaba::IDENTIFIER_USE);
	keywords.add(kaba::IDENTIFIER_ASM);
	keywords.add(kaba::IDENTIFIER_IMPORT);
	keywords.add(kaba::IDENTIFIER_IF);
	keywords.add(kaba::IDENTIFIER_ELSE);
	keywords.add(kaba::IDENTIFIER_WHILE);
	keywords.add(kaba::IDENTIFIER_FOR);
	keywords.add(kaba::IDENTIFIER_IN);
	keywords.add(kaba::IDENTIFIER_RETURN);
	keywords.add(kaba::IDENTIFIER_BREAK);
	keywords.add(kaba::IDENTIFIER_CONTINUE);
	keywords.add(kaba::IDENTIFIER_NOT);
	keywords.add(kaba::IDENTIFIER_AND);
	keywords.add(kaba::IDENTIFIER_OR);
	keywords.add(kaba::IDENTIFIER_NEW);
	keywords.add(kaba::IDENTIFIER_DELETE);
	modifiers.add(kaba::IDENTIFIER_EXTERN);
	modifiers.add(kaba::IDENTIFIER_VIRTUAL);
	modifiers.add(kaba::IDENTIFIER_OVERRIDE);
	modifiers.add(kaba::IDENTIFIER_STATIC);
	keywords.add(kaba::IDENTIFIER_CONST);
	modifiers.add(kaba::IDENTIFIER_MUTABLE);
	modifiers.add(kaba::IDENTIFIER_SELFREF);
	modifiers.add(kaba::IDENTIFIER_REF);
	modifiers.add(kaba::IDENTIFIER_OUT);
	modifiers.add(kaba::IDENTIFIER_SHARED);
	modifiers.add(kaba::IDENTIFIER_OWNED);
	keywords.add(kaba::IDENTIFIER_SELF);
	keywords.add(kaba::IDENTIFIER_SUPER);
	keywords.add(kaba::IDENTIFIER_NAMESPACE);
	keywords.add(kaba::IDENTIFIER_RAISE);
	keywords.add(kaba::IDENTIFIER_TRY);
	keywords.add(kaba::IDENTIFIER_EXCEPT);
	keywords.add(kaba::IDENTIFIER_PASS);
	keywords.add(kaba::IDENTIFIER_LET);
	keywords.add(kaba::IDENTIFIER_VAR);
	keywords.add(kaba::IDENTIFIER_LAMBDA);
	keywords.add(kaba::IDENTIFIER_RAW_FUNCTION_POINTER);
	compiler_functions.add(kaba::IDENTIFIER_DYN);
	compiler_functions.add(kaba::IDENTIFIER_WEAK);
	compiler_functions.add(kaba::IDENTIFIER_LEN);
	compiler_functions.add(kaba::IDENTIFIER_SIZEOF);
	compiler_functions.add(kaba::IDENTIFIER_STR);
	compiler_functions.add(kaba::IDENTIFIER_TYPEOF);
	compiler_functions.add(kaba::IDENTIFIER_MAP);
	compiler_functions.add(kaba::IDENTIFIER_SORTED);
	operator_functions.add(kaba::IDENTIFIER_FUNC_INIT);
	operator_functions.add(kaba::IDENTIFIER_FUNC_DELETE);
	operator_functions.add(kaba::IDENTIFIER_FUNC_DELETE_OVERRIDE);
	operator_functions.add(kaba::IDENTIFIER_FUNC_ASSIGN);
	operator_functions.add(kaba::IDENTIFIER_FUNC_CONTAINS);
	operator_functions.add(kaba::IDENTIFIER_FUNC_STR);
	operator_functions.add(kaba::IDENTIFIER_FUNC_SET);
	operator_functions.add(kaba::IDENTIFIER_FUNC_GET);
	operator_functions.add(kaba::IDENTIFIER_FUNC_SUBARRAY);
	operator_functions.add("__add__");
	operator_functions.add("__iadd__");
	operator_functions.add("__sub__");
	operator_functions.add("__isub__");
	operator_functions.add("__mul__");
	operator_functions.add("__imul__");
	operator_functions.add("__div__");
	operator_functions.add("__idiv__");
	operator_functions.add("__and__");
	keywords.add("as");
	//for (auto &s: kaba::Statements)
	//	special_words.add(s.name);
}

void ParserKaba::clear_symbols() {
	types.clear();
	global_variables.clear();
	functions.clear();
	constants.clear();
}

void ParserKaba::update_symbols(SourceView *sv) {
	auto context = kaba::Context::create();

	try {
		kaba::config.default_filename = sv->doc->filename;
		//msg_write(kaba::config.directory.str());
		msg_write("update...");
		auto m = context->create_for_source(sv->get_all(), true);
		msg_write(p2s(m.get()));

		clear_symbols();

		//m->syntax->

		/*for (auto c: weak(m->syntax->imported_symbols->classes)) {
			if (c->name.tail(1) == "*" or c->name.tail(2) == "[]")
				continue;
			add_class(this, c, "");
			add_class_content(this, c, "");
		}*/

		add_class_content(this, m->syntax->imported_symbols.get(), "");
		add_class_content(this, m->syntax->base_class, "");

	} catch (Exception &e) {
		msg_error(e.message());
	}

	/*for (auto p: kaba::packages) {
		add_class(this, p->base_class(), "");
		//if (p->used_by_default)
			add_class_content(this, p->base_class(), "");
	}*/
}


Array<Parser::Label> ParserKaba::FindLabels(SourceView *sv) {
	Array<Parser::Label> labels;

	int num_lines = sv->get_num_lines();
	string last_class;
	for (int l=0;l<num_lines;l++) {
		string s = sv->get_line(l);
		if (s.num < 4)
			continue;
		if (char_type(s[0]) == CHAR_LETTER) {
			if (s.find("class ") >= 0) {
				last_class = s.replace("\t", " ").replace(":", " ").explode(" ")[1];
				s = "class " + last_class;
			} else if (s.find("(") >= 0) {
				last_class = "";
			} else {
				continue;
			}
			if (s.find(kaba::IDENTIFIER_EXTERN) >= 0)
				continue;
			labels.add(Label(s, l, 0));
		} else if ((last_class.num > 0) && (s[0] == '\t') && (char_type(s[1]) == CHAR_LETTER)) {
			if (s.find("(") < 0)
				continue;
			s = s.replace(kaba::IDENTIFIER_VIRTUAL + " ", "").replace(kaba::IDENTIFIER_OVERRIDE + " ", "").trim();
			labels.add(Label(s, l, 1));
		}
	}
	return labels;
}


void ParserKaba::CreateTextColors(SourceView *sv, int first_line, int last_line) {

	/*try {
		auto m = kaba::create_for_source(sv->get_all());
		msg_write("ok");
	} catch (Exception &e) {
		msg_error(e.message());
	}*/



	CreateTextColorsDefault(sv, first_line, last_line);
}

