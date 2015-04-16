/*
 * ParserPython.cpp
 *
 *  Created on: 25.06.2013
 *      Author: michi
 */

#include "ParserPython.h"
#include "../HighlightScheme.h"

ParserPython::ParserPython()
{
	line_comment_begin = "#";
	special_words.add("enum");
	special_words.add("class");
	special_words.add("import");
	special_words.add("from");
	special_words.add("as");
	special_words.add("def");
	special_words.add("if");
	special_words.add("else");
	special_words.add("elif");
	special_words.add("while");
	special_words.add("for");
	special_words.add("in");
	special_words.add("return");
	special_words.add("break");
	special_words.add("continue");
	special_words.add("and");
	special_words.add("or");
	special_words.add("const");
	special_words.add("self");
	special_words.add("namespace");
	types.add("set");
	types.add("map");
	types.add("str");
	types.add("list");
	types.add("tuple");
	compiler_functions.add("print");
	compiler_functions.add("len");
	compiler_functions.add("range");
	compiler_functions.add("enumerate");
	compiler_functions.add("dir");
	compiler_functions.add("zip");
	compiler_functions.add("max");
	compiler_functions.add("min");
	globals.add("True");
	globals.add("False");
	globals.add("nil");
	globals.add("None");
}

ParserPython::~ParserPython()
{
}

void ParserPython::CreateTextColors(SourceView *sv, int first_line, int last_line)
{
	CreateTextColorsDefault(sv, first_line, last_line);
}

