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
		if (char_type(s[0]) == CharLetter){
			if (s.find("class ") >= 0){
				last_class = s.replace("\t", " ").replace(":", " ").explode(" ")[1];
				s = "class " + last_class;
			}else if (s.find("(") >= 0){
				last_class = "";
			}else
				continue;
			if (s.find("extern") >= 0)
				continue;
			labels.add(Label(s, l));
		}else if ((last_class.num > 0) && (s[0] == '\t') && (char_type(s[1]) == CharLetter)){
			if (s.find("(") < 0)
				continue;
			s = s.replace("virtual ", "").replace("overwrite ", "");
			labels.add(Label(">" + s, l));
		}
	}
	return labels;
}



int ParserKaba::WordType(const string &name)
{
	if (name[0] == '#')
		return InMacro;
	if ((name == "enum") ||
	    (name == "class") ||
		(name == "use") ||
		(name == "import") ||
		(name == "if") ||
		(name == "else") ||
		(name == "while") ||
		(name == "for") ||
		(name == "forall") ||
		(name == "in") ||
		(name == "return") ||
		(name == "break") ||
		(name == "continue") ||
		(name == "and") ||
		(name == "or") ||
		(name == "extern") ||
		(name == "virtual") ||
		(name == "overwrite") ||
		(name == "const") ||
		(name == "this") ||
		(name == "self") ||
		(name == "super") ||
		(name == "new") ||
		(name == "delete") ||
		(name == "asm"))
		return InWordSpecial;
	foreach(Script::Package &p, Script::Packages){
		for (int i=0;i<p.script->syntax->Types.num;i++)
			if (name == p.script->syntax->Types[i]->name)
				return InWordType;
		for (int i=0;i<p.script->syntax->RootOfAllEvil.var.num;i++)
			if (name == p.script->syntax->RootOfAllEvil.var[i].name)
				return InWordGameVariable;
		for (int i=0;i<p.script->syntax->Constants.num;i++)
			if (name == p.script->syntax->Constants[i].name)
				return InWordGameVariable;
		for (int i=0;i<p.script->syntax->Functions.num;i++)
			if (name == p.script->syntax->Functions[i]->name)
				return InWordCompilerFunction;
	}
	for (int i=0;i<Script::PreCommands.num;i++)
		if (name == Script::PreCommands[i].name)
			return InWordCompilerFunction;
	return -1;
}


void ParserKaba::CreateTextColors(SourceView *sv, int first_line, int last_line)
{
	CreateTextColorsDefault(sv, first_line, last_line);
}

