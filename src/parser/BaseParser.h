/*
 * BaseParser.h
 *
 *  Created on: 06.06.2013
 *      Author: michi
 */

#ifndef BASEPARSER_H_
#define BASEPARSER_H_

#include "../lib/base/base.h"

class SourceView;
class Path;

class Parser {
public:
	explicit Parser(const string &name);
	virtual ~Parser();

	string name;
	string macro_begin;
	string line_comment_begin;
	string multi_comment_begin;
	string multi_comment_end;
	string string_sub_begin;
	string string_sub_end;
	Array<string> keywords;
	Array<string> modifiers;
	Array<string> compiler_functions;
	Array<string> operator_functions;
	Array<string> types;
	Array<string> global_variables;
	Array<string> functions;
	Array<string> constants;


	struct Label {
		string name;
		int line;
		int level;
		Label(){}
		Label(const string &name, int line, int level);
	};

	virtual Array<Label> FindLabels(SourceView *sv);
	virtual int WordType(const string &name);
	virtual void CreateTextColors(SourceView *sv, int first_line = -1, int last_line = -1);

	virtual void clear_symbols() {};
	virtual void update_symbols(SourceView *sv) {};

	void CreateTextColorsDefault(SourceView *sv, int first_line, int last_line);
};

void InitParser();
Parser *GetParser(const Path &filename);


enum {
	CHAR_SPACE,
	CHAR_LETTER,
	CHAR_NUMBER,
	CHAR_SIGN
};

inline int char_type(char c) {
	if ((c >= '0') and (c <= '9'))
		return CHAR_NUMBER;
	if ((c == ' ') or (c == '\n') or (c == '\t'))
		return CHAR_SPACE;
	if (((c >= 'a') and (c <= 'z')) or ((c >= 'A') and (c <= 'Z')) or (c == '_'))
		return CHAR_LETTER;
	return CHAR_SIGN;
}


#endif /* BASEPARSER_H_ */
