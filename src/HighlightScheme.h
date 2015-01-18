/*
 * HighlightScheme.h
 *
 *  Created on: 06.06.2013
 *      Author: michi
 */

#ifndef HIGHLIGHTSCHEME_H_
#define HIGHLIGHTSCHEME_H_

#include "lib/image/color.h"

class SourceView;

enum{
	InWord,
	InWordSpecial,
	InWordCompilerFunction,
	InWordGameVariable,
	InWordType,
	InLineComment,
	InCommentLevel1,
	InCommentLevel2,
	InMacro,
	InSpace,
	InString,
	InOperator,
	InNumber,
	NumTagTypes
};

struct HighlightContext
{
	color fg, bg;
	bool set_bg;
	bool bold, italic;
	HighlightContext();
	HighlightContext(const color &_fg, const color &_bg, bool _set_bg, bool _bold, bool _italic);
};

class HighlightScheme
{
public:
	string name;
	color bg;
	HighlightContext context[NumTagTypes];
	bool is_default, changed;
	HighlightScheme();
	HighlightScheme *copy(const string &name);

	static void init();
	static HighlightScheme *get(const string &name);
	static Array<HighlightScheme*> get_all();
	static HighlightScheme *default_scheme;
};



#endif /* HIGHLIGHTSCHEME_H_ */
