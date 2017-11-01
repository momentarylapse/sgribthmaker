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
#include "ParserShader.h"
#include "ParserPython.h"
#include "../SourceView.h"
#include "../HighlightScheme.h"



#define MAX_HIGHLIGHTING_SIZE	100000

struct ParserAssociation
{
	Parser *parser;
	string extension;
	ParserAssociation(){}
	ParserAssociation(Parser *p, const string &ext)
	{
		parser = p;
		extension = ext;
	}
};
static Array<ParserAssociation> ParserAssociations;

Parser::Label::Label(const string &_name, int _line, int _level)
{
	name = _name;
	line = _line;
	level = _level;
}

Parser::Parser()
{
	macro_begin = "-none-";
	line_comment_begin = "-none-";
}

Parser::~Parser()
{
}


Array<Parser::Label> Parser::FindLabels(SourceView *sv)
{
	Array<Parser::Label> labels;
	return labels;
}

int Parser::WordType(const string &name)
{
	if (name.head(macro_begin.num) == macro_begin)
		return IN_MACRO;
	for (string &n : special_words)
		if (name == n)
			return IN_WORD_SPECIAL;
	for (string &n : types)
		if (name == n)
			return IN_WORD_TYPE;
	for (string &n : compiler_functions)
		if (name == n)
			return IN_WORD_COMPILER_FUNCTION;
	for (string &n : globals)
		if (name == n)
			return IN_WORD_GLOBAL_VARIABLE;
	return IN_WORD;
}

void Parser::CreateTextColors(SourceView *sv, int first_line, int last_line)
{
}


#define next_char()	p=g_utf8_next_char(p);pos++
#define set_mark()	p0=p;pos0=pos

void Parser::CreateTextColorsDefault(SourceView *sv, int first_line, int last_line)
{
	msg_db_f("CreateTextColors", 1);
	if (gtk_text_buffer_get_char_count(sv->tb) > MAX_HIGHLIGHTING_SIZE)
		return;

	int comment_level = 0;
	int num_lines = sv->GetNumLines();
	if (first_line < 0)
		first_line = 0;
	if (last_line < 0)
		last_line = num_lines - 1;
	bool in_ml_string = false;

	sv->ClearMarkings(first_line, last_line);

	for (int l=first_line;l<=last_line;l++){
		string s = sv->GetLine(l);

		char *p = (char*)&s[0];
		char *p0 = p;
		int last_type = CHAR_SPACE;
		int in_type = (comment_level > 1) ? IN_COMMENT_LEVEL_2 : ((comment_level > 0) ? IN_COMMENT_LEVEL_1 : IN_SPACE);
		if (in_ml_string)
			in_type = IN_STRING;
		int pos0 = 0;
		int pos = 0;
		int num_uchars = g_utf8_strlen(p, s.num);
		bool prev_was_escape = false;
		while(pos < num_uchars){
			int type = char_type(*p);
			// still in a string?
			if (in_type == IN_STRING){
				if (prev_was_escape){
					prev_was_escape = false;
				}else if (*p == '\\'){
					prev_was_escape = true;
				}else if (*p == '\"'){
					in_type = IN_OPERATOR;
					in_ml_string = false;
					next_char();
					sv->MarkWord(l, pos0, pos, IN_STRING, p0, p);
					set_mark();
					continue;
				}
			// still in a multi-comment?
			}else if (comment_level > 0){
				if ((*p == '/') && (p[1] == '*')){
					sv->MarkWord(l, pos0, pos, in_type, p0, p);
					set_mark();
					next_char();
					in_type = IN_COMMENT_LEVEL_2;
					comment_level ++;
				}else if ((*p == '*') && (p[1] == '/')){
					next_char();
					next_char();
					sv->MarkWord(l, pos0, pos, in_type, p0, p);
					set_mark();
					comment_level --;
					in_type = (comment_level > 0) ? IN_COMMENT_LEVEL_1 : IN_OPERATOR;
					last_type = type;
					continue;
				}
			}else{
				// string starting?
				if (*p == '\"'){
					sv->MarkWord(l, pos0, pos, in_type, p0, p);
					set_mark();
					in_ml_string = true;
					in_type = IN_STRING;
				}else if (last_type != type){
					if ((in_type == IN_NUMBER) && ((*p == '.') || (*p == 'x') || ((*p >= 'a') && (*p <= 'f')))){
						next_char();
						continue;
					}
					if ((in_type == IN_WORD) && (type == CHAR_NUMBER)){
						next_char();
						continue;
					}
					sv->MarkWord(l, pos0, pos, in_type, p0, p);
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
					if (*p == '#'){
						in_type = IN_WORD;
						type = CHAR_LETTER;
					}
				}
				// line comment starting?
				bool line_comment = true;
				for (int j=0; j<line_comment_begin.num; j++)
					if (p[j] != line_comment_begin[j]){
						line_comment = false;
						break;
					}
				if (line_comment){
					sv->MarkWord(l, pos0, pos, in_type, p0, p);
					set_mark();
					in_type = IN_LINE_COMMENT;
					break;
				// multi-comment starting?
				}else if ((*p == '/') && (p[1] == '*')){
					sv->MarkWord(l, pos0, pos, in_type, p0, p);
					set_mark();
					in_type = IN_COMMENT_LEVEL_1;
					comment_level ++;
					next_char();
				}
			}
			last_type = type;
			next_char();
		}
		if (s.num > 0)
			sv->MarkWord(l, pos0, num_uchars, in_type, p0, (char*)&s[s.num]);
	}
}




void InitParser()
{
	ParserAssociations.add(ParserAssociation(new ParserText, "*"));
	ParserAssociations.add(ParserAssociation(new ParserKaba, "kaba"));
	ParserAssociations.add(ParserAssociation(new ParserC, "c"));
	ParserAssociations.add(ParserAssociation(new ParserC, "cpp"));
	ParserAssociations.add(ParserAssociation(new ParserC, "h"));
	ParserAssociations.add(ParserAssociation(new ParserC, "hpp"));
	ParserAssociations.add(ParserAssociation(new ParserShader, "glsl"));
	ParserAssociations.add(ParserAssociation(new ParserShader, "shader"));
	ParserAssociations.add(ParserAssociation(new ParserPython, "py"));
}

Parser *GetParser(const string &filename)
{
	string ext = filename.extension();
	for (ParserAssociation &a : ParserAssociations)
		if (ext == a.extension)
			return a.parser;
	return ParserAssociations[0].parser;
}
