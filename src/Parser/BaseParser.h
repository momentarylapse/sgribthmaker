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

class Parser
{
public:
	Parser();
	virtual ~Parser();
	virtual string GetName() = 0;

	string macro_begin;
	string line_comment_begin;
	Array<string> special_words;
	Array<string> types;
	Array<string> compiler_functions;
	Array<string> globals;


	struct Label
	{
		string name;
		int line;
		int level;
		Label(){}
		Label(const string &name, int line, int level);
	};

	virtual Array<Label> FindLabels(SourceView *sv);
	virtual int WordType(const string &name);
	virtual void CreateTextColors(SourceView *sv, int first_line = -1, int last_line = -1);


	void CreateTextColorsDefault(SourceView *sv, int first_line, int last_line);
};

void InitParser();
Parser *GetParser(const string &filename);


enum{
	CHAR_SPACE,
	CHAR_LETTER,
	CHAR_NUMBER,
	CHAR_SIGN
};

inline int char_type(char c)
{
	if ((c >= '0') && (c <= '9'))
		return CHAR_NUMBER;
	if ((c == ' ') || (c == '\n') || (c == '\t'))
		return CHAR_SPACE;
	if (((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')) || (c == '_'))
		return CHAR_LETTER;
	return CHAR_SIGN;
}


#endif /* BASEPARSER_H_ */
