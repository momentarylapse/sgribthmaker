/*
 * ParserC.cpp
 *
 *  Created on: 06.06.2013
 *      Author: michi
 */

#include "ParserC.h"
#include "../HighlightSchema.h"

ParserC::ParserC()
{
}

ParserC::~ParserC()
{
}

int ParserC::WordType(const string &name)
{
	if (name[0] == '#')
		return InMacro;
	if ((name == "enum") ||
	    (name == "class") ||
		(name == "if") ||
		(name == "else") ||
		(name == "while") ||
		(name == "for") ||
		(name == "return") ||
		(name == "break") ||
		(name == "continue") ||
		(name == "and") ||
		(name == "or") ||
		(name == "new") ||
		(name == "delete") ||
		(name == "extern") ||
		(name == "virtual") ||
		(name == "const") ||
		(name == "this"))
		return InWordSpecial;
	if ((name == "void") ||
	    (name == "int") ||
		(name == "float") ||
		(name == "char") ||
		(name == "unsigned") ||
		(name == "signed") ||
		(name == "short"))
		return InWordType;
	if ((name == "printf") ||
	    (name == "sin"))
		return InWordCompilerFunction;
	return -1;
}

void ParserC::CreateTextColors(SourceView *sv, int first_line, int last_line)
{
	CreateTextColorsDefault(sv, first_line, last_line);
}
