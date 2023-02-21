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
	keywords.add(kaba::Identifier::ENUM);
	keywords.add(kaba::Identifier::CLASS);
	keywords.add(kaba::Identifier::STRUCT);
	keywords.add(kaba::Identifier::INTERFACE);
	keywords.add(kaba::Identifier::FUNC);
	keywords.add(kaba::Identifier::EXTENDS);
	keywords.add(kaba::Identifier::USE);
	keywords.add(kaba::Identifier::ASM);
	keywords.add(kaba::Identifier::IMPORT);
	keywords.add(kaba::Identifier::IF);
	keywords.add(kaba::Identifier::ELSE);
	keywords.add(kaba::Identifier::WHILE);
	keywords.add(kaba::Identifier::FOR);
	keywords.add(kaba::Identifier::IN);
	keywords.add(kaba::Identifier::RETURN);
	keywords.add(kaba::Identifier::BREAK);
	keywords.add(kaba::Identifier::CONTINUE);
	keywords.add(kaba::Identifier::NOT);
	keywords.add(kaba::Identifier::AND);
	keywords.add(kaba::Identifier::OR);
	keywords.add(kaba::Identifier::NEW);
	keywords.add(kaba::Identifier::DELETE);
	modifiers.add(kaba::Identifier::EXTERN);
	modifiers.add(kaba::Identifier::VIRTUAL);
	modifiers.add(kaba::Identifier::OVERRIDE);
	modifiers.add(kaba::Identifier::STATIC);
	keywords.add(kaba::Identifier::CONST);
	modifiers.add(kaba::Identifier::MUTABLE);
	modifiers.add(kaba::Identifier::SELFREF);
	modifiers.add(kaba::Identifier::REF);
	modifiers.add(kaba::Identifier::OUT);
	modifiers.add(kaba::Identifier::SHARED);
	modifiers.add(kaba::Identifier::OWNED);
	modifiers.add(kaba::Identifier::XFER);
	keywords.add(kaba::Identifier::SELF);
	keywords.add(kaba::Identifier::SUPER);
	keywords.add(kaba::Identifier::NAMESPACE);
	keywords.add(kaba::Identifier::RAISE);
	keywords.add(kaba::Identifier::TRY);
	keywords.add(kaba::Identifier::EXCEPT);
	keywords.add(kaba::Identifier::PASS);
	keywords.add(kaba::Identifier::LET);
	keywords.add(kaba::Identifier::VAR);
	keywords.add(kaba::Identifier::LAMBDA);
	keywords.add(kaba::Identifier::RAW_FUNCTION_POINTER);
	compiler_functions.add(kaba::Identifier::DYN);
	compiler_functions.add(kaba::Identifier::WEAK);
	compiler_functions.add(kaba::Identifier::GIVE);
	compiler_functions.add(kaba::Identifier::LEN);
	compiler_functions.add(kaba::Identifier::SIZEOF);
	compiler_functions.add(kaba::Identifier::STR);
	compiler_functions.add(kaba::Identifier::TYPEOF);
	compiler_functions.add(kaba::Identifier::SORT);
	compiler_functions.add(kaba::Identifier::FILTER);
	operator_functions.add(kaba::Identifier::Func::INIT);
	operator_functions.add(kaba::Identifier::Func::DELETE);
	operator_functions.add(kaba::Identifier::Func::DELETE_OVERRIDE);
	operator_functions.add(kaba::Identifier::Func::ASSIGN);
	operator_functions.add(kaba::Identifier::Func::CONTAINS);
	operator_functions.add(kaba::Identifier::Func::STR);
	operator_functions.add(kaba::Identifier::Func::REPR);
	operator_functions.add(kaba::Identifier::Func::SET);
	operator_functions.add(kaba::Identifier::Func::GET);
	operator_functions.add(kaba::Identifier::Func::SUBARRAY);
	operator_functions.add(kaba::Identifier::Func::ADD);
	operator_functions.add(kaba::Identifier::Func::ADD_ASSIGN);
	operator_functions.add(kaba::Identifier::Func::SUBTRACT);
	operator_functions.add(kaba::Identifier::Func::SUBTRACT_ASSIGN);
	operator_functions.add(kaba::Identifier::Func::MULTIPLY);
	operator_functions.add(kaba::Identifier::Func::MULTIPLY_ASSIGN);
	operator_functions.add(kaba::Identifier::Func::DIVIDE);
	operator_functions.add(kaba::Identifier::Func::DIVIDE_ASSIGN);
	operator_functions.add(kaba::Identifier::Func::SMALLER);
	operator_functions.add(kaba::Identifier::Func::SMALLER_EQUAL);
	operator_functions.add(kaba::Identifier::Func::GREATER);
	operator_functions.add(kaba::Identifier::Func::GREATER_EQUAL);
	operator_functions.add(kaba::Identifier::Func::EQUAL);
	operator_functions.add(kaba::Identifier::Func::NOT_EQUAL);
	operator_functions.add(kaba::Identifier::Func::MODULO);
	operator_functions.add(kaba::Identifier::Func::INCREASE);
	operator_functions.add(kaba::Identifier::Func::DECREASE);
	operator_functions.add(kaba::Identifier::Func::EXPONENT);
	operator_functions.add(kaba::Identifier::Func::SHIFT_LEFT);
	operator_functions.add(kaba::Identifier::Func::SHIFT_RIGHT);
	operator_functions.add(kaba::Identifier::Func::AND);
	operator_functions.add(kaba::Identifier::Func::OR);
	operator_functions.add(kaba::Identifier::Func::NOT);
	operator_functions.add(kaba::Identifier::Func::NEGATIVE);
	operator_functions.add(kaba::Identifier::Func::BIT_AND);
	operator_functions.add(kaba::Identifier::Func::BIT_OR);
	operator_functions.add(kaba::Identifier::Func::MAPS_TO);
	operator_functions.add(kaba::Identifier::Func::CALL);
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
	auto context = ownify(kaba::Context::create());

	try {
		kaba::config.default_filename = sv->doc->filename;
		//msg_write(kaba::config.directory.str());
		auto m = context->create_module_for_source(sv->get_all(), true);

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
		//msg_error(e.message());
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
			if (s.find(kaba::Identifier::EXTERN) >= 0)
				continue;
			labels.add(Label(s, l, 0));
		} else if ((last_class.num > 0) && (s[0] == '\t') && (char_type(s[1]) == CHAR_LETTER)) {
			if (s.find("(") < 0)
				continue;
			s = s.replace(kaba::Identifier::VIRTUAL + " ", "").replace(kaba::Identifier::OVERRIDE + " ", "").trim();
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

