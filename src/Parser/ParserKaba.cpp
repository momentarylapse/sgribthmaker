/*
 * ParserKaba.cpp
 *
 *  Created on: 06.06.2013
 *      Author: michi
 */

#include "ParserKaba.h"
#include "../HighlightScheme.h"
#include "../SourceView.h"
#include "../lib/script/script.h"


ParserKaba::ParserKaba()
{
	macro_begin = "#";
	line_comment_begin = "//";
	special_words.add(Script::IDENTIFIER_ENUM);
	special_words.add(Script::IDENTIFIER_CLASS);
	special_words.add(Script::IDENTIFIER_EXTENDS);
	special_words.add(Script::IDENTIFIER_USE);
	special_words.add("import");
	special_words.add(Script::IDENTIFIER_IF);
	special_words.add(Script::IDENTIFIER_ELSE);
	special_words.add(Script::IDENTIFIER_WHILE);
	special_words.add(Script::IDENTIFIER_FOR);
	special_words.add(Script::IDENTIFIER_IN);
	special_words.add(Script::IDENTIFIER_RETURN);
	special_words.add(Script::IDENTIFIER_BREAK);
	special_words.add(Script::IDENTIFIER_CONTINUE);
	special_words.add(Script::IDENTIFIER_AND);
	special_words.add(Script::IDENTIFIER_OR);
	special_words.add(Script::IDENTIFIER_NEW);
	special_words.add(Script::IDENTIFIER_DELETE);
	special_words.add(Script::IDENTIFIER_EXTERN);
	special_words.add(Script::IDENTIFIER_VIRTUAL);
	special_words.add(Script::IDENTIFIER_OVERRIDE);
	special_words.add(Script::IDENTIFIER_STATIC);
	special_words.add(Script::IDENTIFIER_CONST);
	special_words.add(Script::IDENTIFIER_SELF);
	special_words.add(Script::IDENTIFIER_SUPER);
	special_words.add(Script::IDENTIFIER_NAMESPACE);
	special_words.add(Script::IDENTIFIER_ASM);
	for (Script::Package &p : Script::Packages){
		for (int i=0;i<p.script->syntax->types.num;i++)
			types.add(p.script->syntax->types[i]->name);
		for (int i=0;i<p.script->syntax->root_of_all_evil.var.num;i++)
			globals.add(p.script->syntax->root_of_all_evil.var[i].name);
		for (int i=0;i<p.script->syntax->constants.num;i++)
			globals.add(p.script->syntax->constants[i].name);
		for (int i=0;i<p.script->syntax->functions.num;i++)
			compiler_functions.add(p.script->syntax->functions[i]->name);
	}
	for (int i=0;i<Script::PreCommands.num;i++)
		compiler_functions.add(Script::PreCommands[i].name);
}

ParserKaba::~ParserKaba()
{
}


Array<Parser::Label> ParserKaba::FindLabels(SourceView *sv)
{
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
			if (s.find(Script::IDENTIFIER_EXTERN) >= 0)
				continue;
			labels.add(Label(s, l, 0));
		}else if ((last_class.num > 0) && (s[0] == '\t') && (char_type(s[1]) == CHAR_LETTER)){
			if (s.find("(") < 0)
				continue;
			s = s.replace(Script::IDENTIFIER_VIRTUAL + " ", "").replace(Script::IDENTIFIER_OVERRIDE + " ", "").trim();
			labels.add(Label(s, l, 1));
		}
	}
	return labels;
}


void ParserKaba::CreateTextColors(SourceView *sv, int first_line, int last_line)
{
	CreateTextColorsDefault(sv, first_line, last_line);
}

