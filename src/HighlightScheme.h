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

enum {
	IN_WORD,
	IN_WORD_SPECIAL,
	IN_WORD_COMPILER_FUNCTION,
	IN_WORD_OPERATOR_FUNCTION,
	IN_WORD_MODIFIER,
	IN_WORD_GLOBAL_VARIABLE,
	IN_WORD_TYPE,
	IN_LINE_COMMENT,
	IN_COMMENT_LEVEL_1,
	IN_COMMENT_LEVEL_2,
	IN_MACRO,
	IN_SPACE,
	IN_STRING,
	IN_STRING_SUBSTITUDE,
	IN_OPERATOR,
	IN_NUMBER,
	NUM_TAG_TYPES
};

struct HighlightContext {
	color fg, bg;
	bool set_bg;
	bool bold, italic;
	HighlightContext();
	HighlightContext(const color &_fg, const color &_bg, bool _set_bg, bool _bold, bool _italic);
};

class HighlightScheme {
public:
	string name;
	color bg;
	HighlightContext context[NUM_TAG_TYPES];
	bool is_default, changed;
	HighlightScheme();
	HighlightScheme *copy(const string &name);

	static void init();
	static HighlightScheme *get(const string &name);
	static Array<HighlightScheme*> get_all();
	static HighlightScheme *default_scheme;
};



#endif /* HIGHLIGHTSCHEME_H_ */
