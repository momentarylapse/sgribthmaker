/*
 * ParserIni.cpp
 *
 *  Created on: 2 Oct 2023
 *      Author: michi
 */

#include "ParserIni.h"

#include "../HighlightScheme.h"
#include "../SourceView.h"

ParserIni::ParserIni() : Parser("Ini") {
	macro_begin = "???";
	line_comment_begin = "#";
	multi_comment_begin = "/*";
	multi_comment_end = "*/";
}

#define next_char()	p=g_utf8_next_char(p);pos++
#define set_mark()	sv->mark_word(l, pos0, pos, in_type, p0, p);p0=p;pos0=pos
#define begin_token(t) (string(p, t.num) == t)
#define skip_token_almost(t) p+=(t.num-1);pos+=(t.num-1)
#define skip_token(t) p+=t.num;pos+=t.num


void ParserIni::CreateTextColorsHeader(SourceView *sv, int line_no, const string& line) {
	int num_uchars = line.utf8len();
	char *p = (char*)line.data;
	char *p0 = p;
	sv->mark_word(line_no, 0, num_uchars, IN_MACRO, p0, p0 + line.num);
}

bool is_numeric(const string& s) {
	if (s.num == 0)
		return false;
	for (auto c: s)
		if ((c < '0' or c > '9') and c != '.')
			return false;
	return true;
}

void ParserIni::CreateTextColorsKeyValue(SourceView *sv, int line_no, const string& line) {
//	int num_uchars = line.utf8len();
	char *p = (char*)line.data;//(const char*)&line[0];
	char *p0 = p;

	int i0 = line.find("=");
	if (i0 < 0)
		return;
	int i0u = line.sub_ref(0, i0).utf8len();

	sv->mark_word(line_no, 0, i0u, IN_WORD_SPECIAL, p0, p0 + i0);
	string rest = line.sub_ref(i0 + 1, line.num);
	string value = rest.trim();
	i0 += 1;
	i0u += 1;
	if (value.head(1) == "[" and value.tail(1) == "]") {
		auto xx = value.sub_ref(1, value.num - 1).explode(",");
		i0u += 1 + rest.find("[");
		i0 += 1 + rest.find("[");
		for (auto &x: xx) {
			sv->mark_word(line_no, i0u, i0u + x.utf8len(), IN_WORD_MODIFIER, p0 + i0, p0 + i0 + x.num);
			i0 += x.num + 1;
			i0u += x.utf8len() + 1;
		}

	} else {
		int type = IN_WORD_MODIFIER;
		if (value == "true" or value == "false")
			type = IN_WORD_GLOBAL_VARIABLE;
		else if (is_numeric(value))
			type = IN_NUMBER;
		else if (value.head(1) == "\"" and value.tail(1) == "\"")
			type = IN_STRING;
		sv->mark_word(line_no, i0u, i0u + rest.utf8len(), type, p0 + i0, p0 + line.num);
	}
}

void ParserIni::CreateTextColors(SourceView *sv, int first_line, int last_line) {
	int num_lines = sv->get_num_lines();
	if (first_line < 0)
		first_line = 0;
	if (last_line < 0)
		last_line = num_lines - 1;

	sv->clear_markings(first_line, last_line);

	for (int l=first_line; l<=last_line; l++) {
		string s = sv->get_line(l);
		if (s.trim().head(1) == "[")
			CreateTextColorsHeader(sv, l, s);
		else if (s.trim().head(1) == "#")
			sv->mark_word(l, 0, s.utf8len(), IN_LINE_COMMENT, (char*)&s[0], (char*)&s.back());
		else
			CreateTextColorsKeyValue(sv, l, s);
	}
}

