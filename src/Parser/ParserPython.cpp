/*
 * ParserPython.cpp
 *
 *  Created on: 25.06.2013
 *      Author: michi
 */

#include "ParserPython.h"
#include "../HighlightSchema.h"

ParserPython::ParserPython()
{
}

ParserPython::~ParserPython()
{
}

int ParserPython::WordType(const string &name)
{
	if ((name == "enum") ||
		(name == "class") ||
		(name == "def") ||
		(name == "import") ||
		(name == "if") ||
		(name == "else") ||
		(name == "elif") ||
		(name == "while") ||
		(name == "for") ||
		(name == "in") ||
		(name == "return") ||
		(name == "break") ||
		(name == "continue") ||
		(name == "and") ||
		(name == "or") ||
		(name == "delete") ||
		(name == "const") ||
		(name == "self"))
		return InWordSpecial;
	if ((name == "set") ||
	    (name == "map") ||
		(name == "str"))
		return InWordType;
	if ((name == "print") ||
		(name == "len") ||
		(name == "range") ||
		(name == "enumerate"))
		return InWordCompilerFunction;
	return -1;
}

void ParserPython::CreateTextColors(SourceView *sv, int first_line, int last_line)
{
	CreateTextColorsDefault(sv, first_line, last_line);
}

