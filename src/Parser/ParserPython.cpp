/*
 * ParserPython.cpp
 *
 *  Created on: 25.06.2013
 *      Author: michi
 */

#include "ParserPython.h"
#include "../HighlightScheme.h"

ParserPython::ParserPython() : Parser("Python") {
	line_comment_begin = "#";
	string_sub_begin = "{";
	string_sub_end = "}";
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
	special_words.add("is");
	special_words.add("return");
	special_words.add("break");
	special_words.add("continue");
	special_words.add("and");
	special_words.add("or");
	special_words.add("not");
	special_words.add("pass");
	special_words.add("raise");
	special_words.add("except");
	special_words.add("const");
	special_words.add("self");
	special_words.add("with");
	special_words.add("global");
	special_words.add("namespace");
	types.add("set");
	types.add("dict");
	types.add("str");
	types.add("int");
	types.add("float");
	types.add("list");
	types.add("tuple");
	types.add("Exception");
	compiler_functions.add("print");
	compiler_functions.add("len");
	compiler_functions.add("type");
	compiler_functions.add("range");
	compiler_functions.add("enumerate");
	compiler_functions.add("dir");
	compiler_functions.add("zip");
	compiler_functions.add("map");
	compiler_functions.add("max");
	compiler_functions.add("min");
	compiler_functions.add("sum");
	compiler_functions.add("sorted");
	compiler_functions.add("open");
	compiler_functions.add("sin");
	compiler_functions.add("cos");
	compiler_functions.add("sqrt");
	compiler_functions.add("pow");
	compiler_functions.add("exp");
	compiler_functions.add("log");
	globals.add("True");
	globals.add("False");
	globals.add("nil");
	globals.add("None");
}

void ParserPython::CreateTextColors(SourceView *sv, int first_line, int last_line) {
	CreateTextColorsDefault(sv, first_line, last_line);
}

