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


ParserKaba::ParserKaba() : Parser("Kaba") {
	macro_begin = "#define";
	line_comment_begin = "#";
	string_sub_begin = "{{";
	string_sub_end = "}}";
	special_words.add(Kaba::IDENTIFIER_ENUM);
	special_words.add(Kaba::IDENTIFIER_CLASS);
	special_words.add(Kaba::IDENTIFIER_EXTENDS);
	special_words.add(Kaba::IDENTIFIER_USE);
	special_words.add(Kaba::IDENTIFIER_ASM);
	special_words.add("import");
	special_words.add(Kaba::IDENTIFIER_IF);
	special_words.add(Kaba::IDENTIFIER_ELSE);
	special_words.add(Kaba::IDENTIFIER_WHILE);
	special_words.add(Kaba::IDENTIFIER_FOR);
	special_words.add(Kaba::IDENTIFIER_IN);
	special_words.add(Kaba::IDENTIFIER_RETURN);
	special_words.add(Kaba::IDENTIFIER_BREAK);
	special_words.add(Kaba::IDENTIFIER_CONTINUE);
	special_words.add(Kaba::IDENTIFIER_AND);
	special_words.add(Kaba::IDENTIFIER_OR);
	special_words.add(Kaba::IDENTIFIER_NEW);
	special_words.add(Kaba::IDENTIFIER_DELETE);
	special_words.add(Kaba::IDENTIFIER_EXTERN);
	special_words.add(Kaba::IDENTIFIER_VIRTUAL);
	special_words.add(Kaba::IDENTIFIER_OVERRIDE);
	special_words.add(Kaba::IDENTIFIER_STATIC);
	special_words.add(Kaba::IDENTIFIER_CONST);
	special_words.add(Kaba::IDENTIFIER_SELF);
	special_words.add(Kaba::IDENTIFIER_SUPER);
	special_words.add(Kaba::IDENTIFIER_NAMESPACE);
	special_words.add(Kaba::IDENTIFIER_RAISE);
	special_words.add(Kaba::IDENTIFIER_TRY);
	special_words.add(Kaba::IDENTIFIER_EXCEPT);
	special_words.add(Kaba::IDENTIFIER_PASS);
	special_words.add(Kaba::IDENTIFIER_LET);
	special_words.add(Kaba::IDENTIFIER_LAMBDA);
	compiler_functions.add(Kaba::IDENTIFIER_LEN);
	compiler_functions.add(Kaba::IDENTIFIER_SIZEOF);
	compiler_functions.add(Kaba::IDENTIFIER_STR);
	compiler_functions.add(Kaba::IDENTIFIER_TYPE);
	compiler_functions.add(Kaba::IDENTIFIER_MAP);
	compiler_functions.add(Kaba::IDENTIFIER_SORTED);
	special_words.add("as");
	for (auto *p: Kaba::Packages){
		for (auto *c: p->syntax->base_class->classes)
			types.add(c->name);
		for (auto *v: p->syntax->root_of_all_evil->var)
			globals.add(v->name);
		for (auto *c: p->syntax->base_class->constants)
			globals.add(c->name);
		for (auto *f: p->syntax->base_class->static_functions)
			compiler_functions.add(f->name);
	}
	//for (auto &s: Kaba::Statements)
	//	special_words.add(s.name);
}


Array<Parser::Label> ParserKaba::FindLabels(SourceView *sv) {
	Array<Parser::Label> labels;
	int num_lines = sv->GetNumLines();
	string last_class;
	for (int l=0;l<num_lines;l++){
		string s = sv->GetLine(l);
		if (s.num < 4)
			continue;
		if (char_type(s[0]) == CHAR_LETTER){
			if (s.find("class ") >= 0){
				last_class = s.replace("\t", " ").replace(":", " ").explode(" ")[1];
				s = "class " + last_class;
			}else if (s.find("(") >= 0){
				last_class = "";
			}else
				continue;
			if (s.find(Kaba::IDENTIFIER_EXTERN) >= 0)
				continue;
			labels.add(Label(s, l, 0));
		}else if ((last_class.num > 0) && (s[0] == '\t') && (char_type(s[1]) == CHAR_LETTER)){
			if (s.find("(") < 0)
				continue;
			s = s.replace(Kaba::IDENTIFIER_VIRTUAL + " ", "").replace(Kaba::IDENTIFIER_OVERRIDE + " ", "").trim();
			labels.add(Label(s, l, 1));
		}
	}
	return labels;
}


void ParserKaba::CreateTextColors(SourceView *sv, int first_line, int last_line) {
	CreateTextColorsDefault(sv, first_line, last_line);
}

