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
#include "../SourceView.h"
#include "../HighlightSchema.h"



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

Parser::Label::Label(const string &_name, int _line)
{
	name = _name;
	line = _line;
}

Parser::Parser()
{
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
	return InWord;
}

void Parser::CreateTextColors(SourceView *sv, int first_line, int last_line)
{
}


#define next_char()	p=g_utf8_next_char(p);pos++
#define set_mark()	p0=p;pos0=pos

void Parser::CreateTextColorsDefault(SourceView *sv, int first_line, int last_line)
{
	if (gtk_text_buffer_get_char_count(sv->tb) > MAX_HIGHLIGHTING_SIZE)
		return;
	msg_db_f("CreateTextColors", 1);

	int comment_level = 0;
	int num_lines = sv->GetNumLines();
	if (first_line < 0)
		first_line = 0;
	if (last_line < 0)
		last_line = num_lines - 1;

	sv->ClearMarkings(first_line, last_line);

	for (int l=first_line;l<=last_line;l++){
		string s = sv->GetLine(l);

		char *p = &s[0];
		char *p0 = p;
		int last_type = CharSpace;
		int in_type = (comment_level > 1) ? InCommentLevel2 : ((comment_level > 0) ? InCommentLevel1 : InSpace);
		int pos0 = 0;
		int pos = 0;
		int num_uchars = g_utf8_strlen(p, s.num);
		while(pos < num_uchars){
			int type = char_type(*p);
			// still in a string?
			if (in_type == InString){
				if (*p == '\"'){
					in_type = InOperator;
					next_char();
					sv->MarkWord(l, pos0, pos, InString, p0, p);
					set_mark();
					continue;
				}
			// still in a multi-comment?
			}else if (comment_level > 0){
				if ((*p == '/') && (p[1] == '*')){
					sv->MarkWord(l, pos0, pos, in_type, p0, p);
					set_mark();
					next_char();
					in_type = InCommentLevel2;
					comment_level ++;
				}else if ((*p == '*') && (p[1] == '/')){
					next_char();
					next_char();
					sv->MarkWord(l, pos0, pos, in_type, p0, p);
					set_mark();
					comment_level --;
					in_type = (comment_level > 0) ? InCommentLevel1 : InOperator;
					last_type = type;
					continue;
				}
			}else{
				// string starting?
				if (*p == '\"'){
					sv->MarkWord(l, pos0, pos, in_type, p0, p);
					set_mark();
					in_type = InString;
				}else if (last_type != type){
					if ((in_type == InNumber) && ((*p == '.') || (*p == 'x') || ((*p >= 'a') && (*p <= 'f')))){
						next_char();
						continue;
					}
					if ((in_type == InWord) && (type == CharNumber)){
						next_char();
						continue;
					}
					sv->MarkWord(l, pos0, pos, in_type, p0, p);
					set_mark();
					if (type == CharSpace)
						in_type = InSpace;
					else if (type == CharLetter)
						in_type = InWord;
					else if (type == CharNumber)
						in_type = InNumber;
					else if (type == CharSign)
						in_type = InOperator;
					// # -> macro...
					if (*p == '#'){
						in_type = InWord;
						type = CharLetter;
					}
				}
				// line comment starting?
				if ((*p == '/') && (p[1] == '/')){
					sv->MarkWord(l, pos0, pos, in_type, p0, p);
					set_mark();
					in_type = InLineComment;
					break;
				// multi-comment starting?
				}else if ((*p == '/') && (p[1] == '*')){
					sv->MarkWord(l, pos0, pos, in_type, p0, p);
					set_mark();
					in_type = InCommentLevel1;
					comment_level ++;
					next_char();
				}
			}
			last_type = type;
			next_char();
		}
		if (s.num > 0)
			sv->MarkWord(l, pos0, num_uchars, in_type, p0, &s[s.num]);
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
}

Parser *GetParser(const string &filename)
{
	string ext = filename.extension();
	foreach(ParserAssociation &a, ParserAssociations)
		if (ext == a.extension)
			return a.parser;
	return ParserAssociations[0].parser;
}
