/*
 * ParserC.cpp
 *
 *  Created on: 06.06.2013
 *      Author: michi
 */

#include "ParserC.h"
#include "../HighlightScheme.h"

ParserC::ParserC() : Parser("C/C++") {
	macro_begin = "#";
	line_comment_begin = "//";
	multi_comment_begin = "/*";
	multi_comment_end = "*/";
	special_words.add("enum");
	special_words.add("class");
	special_words.add("if");
	special_words.add("else");
	special_words.add("while");
	special_words.add("for");
	special_words.add("return");
	special_words.add("break");
	special_words.add("continue");
	special_words.add("and");
	special_words.add("or");
	special_words.add("new");
	special_words.add("delete");
	special_words.add("extern");
	special_words.add("virtual");
	special_words.add("static");
	special_words.add("public");
	special_words.add("private");
	special_words.add("protected");
	special_words.add("const");
	special_words.add("this");
	special_words.add("namespace");
	special_words.add("struct");
	special_words.add("template");
	types.add("int");
	types.add("void");
	types.add("float");
	types.add("double");
	types.add("long");
	types.add("char");
	types.add("bool");
	types.add("signed");
	types.add("unsigned");
	types.add("short");
	compiler_functions.add("printf");
	compiler_functions.add("sin");
	compiler_functions.add("cos");
	globals.add("null");
	globals.add("true");
	globals.add("false");
}

void ParserC::CreateTextColors(SourceView *sv, int first_line, int last_line) {
	CreateTextColorsDefault(sv, first_line, last_line);
}
