/*
 * BaseParser.cpp
 *
 *  Created on: 06.06.2013
 *      Author: michi
 */

#include "BaseParser.h"
#include "ParserText.h"
#include "ParserKaba.h"
#include "ParserC.h"
#include "ParserCmake.h"
#include "ParserShader.h"
#include "ParserPython.h"
#include "ParserHui.h"
#include "ParserIni.h"
#include "../SourceView.h"
#include "../HighlightScheme.h"



#define MAX_HIGHLIGHTING_SIZE	100000

struct ParserAssociation {
	Parser *parser = nullptr;
	string extension;
	string base_name;
	ParserAssociation() = default;
	ParserAssociation(Parser *p, const string &ext, const string& _base_name = "") {
		parser = p;
		extension = ext;
		base_name = _base_name;
	}
	bool is_applicable(const Path& filename) const {
		if (extension.num > 0)
			return filename.extension() == extension;
		if (base_name.num > 0)
			return filename.basename() == base_name;
		return false;
	}
};
static Array<ParserAssociation> ParserAssociations;

Parser::Label::Label(const string &_name, int _line, int _level) {
	name = _name;
	line = _line;
	level = _level;
}

Parser::Parser(const string &_name) {
	name = _name;
	macro_begin = "-none-";
	line_comment_begin = "-none-";
	multi_comment_begin = "-none-";
	multi_comment_end = "-none-";
	string_sub_begin = "-none-";
	string_sub_end = "-none-";
}

Parser::~Parser() = default;


Array<Parser::Label> Parser::FindLabels(SourceView *sv) {
	return {};
}

int Parser::WordType(const string &name) {
	if (name.head(macro_begin.num) == macro_begin)
		return IN_MACRO;
	for (string &n : keywords)
		if (name == n)
			return IN_WORD_SPECIAL;
	for (string &n : modifiers)
		if (name == n)
			return IN_WORD_MODIFIER;
	for (string &n : types)
		if (name == n)
			return IN_WORD_TYPE;
	for (string &n : compiler_functions)
		if (name == n)
			return IN_WORD_COMPILER_FUNCTION;
	for (string &n : operator_functions)
		if (name == n)
			return IN_WORD_OPERATOR_FUNCTION;
	for (string &n : functions)
		if (name == n)
			return IN_WORD_COMPILER_FUNCTION;
	for (string &n : global_variables)
		if (name == n)
			return IN_WORD_GLOBAL_VARIABLE;
	for (string &n : constants)
		if (name == n)
			return IN_WORD_GLOBAL_VARIABLE;
	return IN_WORD;
}

void Parser::CreateTextColors(SourceView *sv, int first_line, int last_line) {
}


#define next_char()	p=g_utf8_next_char(p);pos++
#define set_mark()	sv->mark_word(l, pos0, pos, in_type, p0, p);p0=p;pos0=pos
#define begin_token(t) (string(p, t.num) == t)
#define skip_token_almost(t) p+=(t.num-1);pos+=(t.num-1)
#define skip_token(t) p+=t.num;pos+=t.num

void Parser::CreateTextColorsDefault(SourceView *sv, int first_line, int last_line) {
	if (gtk_text_buffer_get_char_count(sv->tb) > MAX_HIGHLIGHTING_SIZE)
		return;

	//update_symbols(sv);

	int comment_level = 0;
	int num_lines = sv->get_num_lines();
	if (first_line < 0)
		first_line = 0;
	if (last_line < 0)
		last_line = num_lines - 1;
	bool in_ml_string = false; // ".."
	bool in_alt_string = false; // '..'

	sv->clear_markings(first_line, last_line);


	for (int l=first_line; l<=last_line; l++) {
		string s = sv->get_line(l);

		char *p = (char*)&s[0];
		char *p0 = p;
		int last_type = CHAR_SPACE;
		int in_type = (comment_level > 1) ? IN_COMMENT_LEVEL_2 : ((comment_level > 0) ? IN_COMMENT_LEVEL_1 : IN_SPACE);
		if (in_ml_string or in_alt_string)
			in_type = IN_STRING;
		int pos0 = 0;
		int pos = 0;
		int num_uchars = g_utf8_strlen(p, s.num);
		bool prev_was_escape = false;
		while (pos < num_uchars) {
			int type = char_type(*p);
			// still in a string?
			if (in_type == IN_STRING) {
				if (prev_was_escape) {
					prev_was_escape = false;
				} else if (*p == '\\') {
					prev_was_escape = true;
				} else if ((in_ml_string and (*p == '\"')) or (in_alt_string and (*p == '\''))) {
					next_char();
					set_mark();
					in_type = IN_OPERATOR;
					in_ml_string = false;
					in_alt_string = false;
					continue;
				} else if (begin_token(string_sub_begin)) {
					set_mark();
					in_type = IN_STRING_SUBSTITUDE;
					skip_token(string_sub_begin);
					continue;
				}
			} else if (in_type == IN_STRING_SUBSTITUDE) {
				if (begin_token(string_sub_end)) {
					skip_token(string_sub_end);
					set_mark();
					in_type = IN_STRING;
					continue;
				}
			// still in a multi-comment?
			} else if (comment_level > 0) {
				if (begin_token(multi_comment_begin)) {
					set_mark();
					//next_char();
					skip_token_almost(multi_comment_begin);
					in_type = IN_COMMENT_LEVEL_2;
					comment_level ++;
				} else if (begin_token(multi_comment_end)) {
					skip_token(multi_comment_end);
					set_mark();
					comment_level --;
					in_type = (comment_level > 0) ? IN_COMMENT_LEVEL_1 : IN_OPERATOR;
					last_type = type;
					continue;
				}
			}else{
				// string starting?
				if (*p == '\"') {
					set_mark();
					in_ml_string = true;
					in_type = IN_STRING;
				} else if (*p == '\'') {
					set_mark();
					in_alt_string = true;
					in_type = IN_STRING;
				} else if (last_type != type) {
					if ((in_type == IN_NUMBER) and ((*p == '.') or (*p == 'x') or ((*p >= 'a') and (*p <= 'f')))) {
						next_char();
						continue;
					}
					if ((in_type == IN_WORD) and (type == CHAR_NUMBER)) {
						next_char();
						continue;
					}
					set_mark();
					if (type == CHAR_SPACE)
						in_type = IN_SPACE;
					else if (type == CHAR_LETTER)
						in_type = IN_WORD;
					else if (type == CHAR_NUMBER)
						in_type = IN_NUMBER;
					else if (type == CHAR_SIGN)
						in_type = IN_OPERATOR;
					// # -> macro...
					if (begin_token(macro_begin)) {
						skip_token_almost(macro_begin);
						in_type = IN_WORD;
						type = CHAR_LETTER;
					}
				}
				// line comment starting?
				if (begin_token(line_comment_begin)) {
					[[maybe_unused]] bool line_comment = true;
					set_mark();
					in_type = IN_LINE_COMMENT;
					break;
				// multi-comment starting?
				} else if (begin_token(multi_comment_begin)) {
					set_mark();
					in_type = IN_COMMENT_LEVEL_1;
					comment_level ++;
					//next_char();
					skip_token_almost(multi_comment_begin);
				}
			}
			last_type = type;
			next_char();
		}
		if (s.num > 0)
			sv->mark_word(l, pos0, num_uchars, in_type, p0, (char*)&s[s.num]);
	}
}




void InitParser() {
	ParserAssociations.add(ParserAssociation(new ParserText, "*"));
	ParserAssociations.add(ParserAssociation(new ParserKaba, "kaba"));
	ParserAssociations.add(ParserAssociation(new ParserC, "c"));
	ParserAssociations.add(ParserAssociation(new ParserC, "cpp"));
	ParserAssociations.add(ParserAssociation(new ParserC, "h"));
	ParserAssociations.add(ParserAssociation(new ParserC, "hpp"));
	ParserAssociations.add(ParserAssociation(new ParserCmake, "", "CMakeLists.txt"));
	ParserAssociations.add(ParserAssociation(new ParserCmake, "cmake"));
	ParserAssociations.add(ParserAssociation(new ParserShader, "glsl"));
	ParserAssociations.add(ParserAssociation(new ParserShader, "shader"));
	ParserAssociations.add(ParserAssociation(new ParserPython, "py"));
	ParserAssociations.add(ParserAssociation(new ParserHui, "hui"));
	ParserAssociations.add(ParserAssociation(new ParserIni, "ini"));
	ParserAssociations.add(ParserAssociation(new ParserIni, "conf"));
	ParserAssociations.add(ParserAssociation(new ParserIni, "material"));
}

Parser *GetParser(const Path &filename) {
	for (auto &a: ParserAssociations)
		if (a.is_applicable(filename))
			return a.parser;
	return ParserAssociations[0].parser;
}
