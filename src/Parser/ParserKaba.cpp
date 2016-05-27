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
	special_words.add("enum");
	special_words.add("class");
	special_words.add("use");
	special_words.add("import");
	special_words.add("if");
	special_words.add("else");
	special_words.add("while");
	special_words.add("for");
	special_words.add("in");
	special_words.add("return");
	special_words.add("break");
	special_words.add("continue");
	special_words.add("and");
	special_words.add("or");
	special_words.add("new");
	special_words.add("delete");
	special_words.add("extern");
	special_words.add("virtual");
	special_words.add("override");
	special_words.add("static");
	special_words.add("const");
	special_words.add("self");
	special_words.add("super");
	special_words.add("namespace");
	special_words.add("asm");
	foreach(Script::Package &p, Script::Packages){
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
			if (s.find("extern") >= 0)
				continue;
			labels.add(Label(s, l, 0));
		}else if ((last_class.num > 0) && (s[0] == '\t') && (char_type(s[1]) == CHAR_LETTER)){
			if (s.find("(") < 0)
				continue;
			s = s.replace("virtual ", "").replace("override ", "").trim();
			labels.add(Label(s, l, 1));
		}
	}
	return labels;
}


void ParserKaba::CreateTextColors(SourceView *sv, int first_line, int last_line)
{
	CreateTextColorsDefault(sv, first_line, last_line);
}

