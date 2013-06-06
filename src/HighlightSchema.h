/*
 * HighlightSchema.h
 *
 *  Created on: 06.06.2013
 *      Author: michi
 */

#ifndef HIGHLIGHTSCHEMA_H_
#define HIGHLIGHTSCHEMA_H_

#include "lib/image/color.h"

class SourceView;

enum{
	InWordSpecial,
	InWordCompilerFunction,
	InWordGameVariable,
	InWordType,
	InWord,
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
	HighlightContext(){}
	HighlightContext(const color &_fg, const color &_bg, bool _set_bg, bool _bold, bool _italic)
	{
		fg = _fg;
		bg = _bg;
		set_bg = _set_bg;
		bold = _bold;
		italic = _italic;
	}
};

struct HighlightSchema
{
	string name;
	color fg, bg;
	HighlightContext context[NumTagTypes];
	void apply(SourceView *sv);
};

HighlightSchema GetDefaultSchema();


#endif /* HIGHLIGHTSCHEMA_H_ */
