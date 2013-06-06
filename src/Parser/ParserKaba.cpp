/*
 * ParserKaba.cpp
 *
 *  Created on: 06.06.2013
 *      Author: michi
 */

#include "ParserKaba.h"
#include "../SourceView.h"
#include "../HighlightSchema.h"
#include "../lib/script/script.h"


#define MAX_HIGHLIGHTING_SIZE	100000

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
			bool ok = false;
			if (s.find("class ") >= 0){
				ok = true;
				last_class = s.replace("\t", " ").replace(":", " ").explode(" ")[1];
				s = "class " + last_class;
			}else if (s.find("(") >= 0){
				ok = true;
				last_class = "";
			}
			if (s.find("extern") >= 0)
				ok = false;
			if (ok)
				labels.add(Label(s, l));
		}else if ((last_class.num > 0) && (s[0] == '\t') && (char_type(s[1]) == CharLetter)){
			if (s.find("(") >= 0)
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
		(name == "const") ||
		(name == "this") ||
		(name == "self") ||
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



#define next_char()	p=g_utf8_next_char(p);pos++
#define set_mark()	p0=p;pos0=pos

void ParserKaba::CreateTextColors(SourceView *sv, int first_line, int last_line)
{
	if (gtk_text_buffer_get_char_count(sv->tb) > MAX_HIGHLIGHTING_SIZE)
		return;
	/*if (!allow_highlighting(Filename))
		return;*/
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

		//msg_write("a");
		char *p = &s[0];
		char *p0 = p;
		int last_type = CharSpace;
		int in_type = (comment_level > 1) ? InCommentLevel2 : ((comment_level > 0) ? InCommentLevel1 : InSpace);
		int pos0 = 0;
		int pos = 0;
		while(pos < s.num){
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
			sv->MarkWord(l, pos0, s.num, in_type, p0, &s[s.num]);
	}
}

