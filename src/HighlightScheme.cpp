/*
 * Highlightscheme.cpp
 *
 *  Created on: 06.06.2013
 *      Author: michi
 */

#include "HighlightScheme.h"
#include "SourceView.h"
#include "lib/hui/hui.h"

static Array<HighlightScheme*> HighlightSchemas;
HighlightScheme *HighlightScheme::default_scheme;

HighlightContext::HighlightContext()
{
	fg = Black;
	bg = White;
	set_bg = false;
	bold = false;
	italic = false;
}

HighlightContext::HighlightContext(const color &_fg, const color &_bg, bool _set_bg, bool _bold, bool _italic)
{
	fg = _fg;
	bg = _bg;
	set_bg = _set_bg;
	bold = _bold;
	italic = _italic;
}

HighlightScheme::HighlightScheme()
{
	bg = White;
	is_default = false;
	changed = false;
}

HighlightScheme *HighlightScheme::copy(const string &name)
{
	HighlightScheme *s = new HighlightScheme;
	*s = *this;
	s->name = name;
	s->is_default = false;
	s->changed = true;
	HighlightSchemas.add(s);
	return s;
}

void HighlightScheme::init()
{
	HighlightScheme *schema = new HighlightScheme;
	schema->name = "default";
	schema->is_default = true;
	schema->bg = White;
	schema->context[IN_LINE_COMMENT] = HighlightContext(color(1, 0.5f, 0.5f, 0.5f), Black, false, false, true);
	schema->context[IN_COMMENT_LEVEL_1] = HighlightContext(color(1, 0.5f, 0.5f, 0.5f), Black, false, false, true);
	schema->context[IN_COMMENT_LEVEL_2] = HighlightContext(color(1, 0.7f, 0.7f, 0.7f), Black, false, false, true);
	schema->context[IN_SPACE] = HighlightContext(Black, Black, false, false, false);
	schema->context[IN_WORD] = HighlightContext(Black, Black, false, false, false);
	schema->context[IN_WORD_TYPE] = HighlightContext(color(1, 0.125f, 0, 0.875f), Black, false, true, false);
	schema->context[IN_WORD_GLOBAL_VARIABLE] = HighlightContext(color(1, 0.625f, 0.625f, 0), Black, false, false, false);
	schema->context[IN_WORD_COMPILER_FUNCTION] = HighlightContext(color(1, 0.065f, 0, 0.625f), Black, false, false, false);
	schema->context[IN_WORD_SPECIAL] = HighlightContext(color(1, 0.625f, 0, 0.625f), Black, false, true, false);
	schema->context[IN_NUMBER] = HighlightContext(color(1, 0, 0.5f, 0), Black, false, false, false);
	schema->context[IN_OPERATOR] = HighlightContext(color(1, 0.25f, 0.25f, 0), Black, false, false, false);
	schema->context[IN_STRING] = HighlightContext(color(1, 1, 0, 0), Black, false, false, false);
	schema->context[IN_MACRO] = HighlightContext(color(1, 0, 0.5f, 0.5f), Black, false, false, false);
	default_scheme = schema;
	HighlightSchemas.add(schema);

	schema = new HighlightScheme;
	schema->name = "dark";
	schema->is_default = true;
	schema->bg = Black;
	schema->context[IN_LINE_COMMENT] = HighlightContext(color(1, 0.5f, 0.5f, 0.5f), Black, false, false, true);
	schema->context[IN_COMMENT_LEVEL_1] = HighlightContext(color(1, 0.5f, 0.5f, 0.5f), Black, false, false, true);
	schema->context[IN_COMMENT_LEVEL_2] = HighlightContext(color(1, 0.3f, 0.7f, 0.7f), Black, false, false, true);
	schema->context[IN_SPACE] = HighlightContext(White, Black, false, false, false);
	schema->context[IN_WORD] = HighlightContext(White, Black, false, false, false);
	schema->context[IN_WORD_TYPE] = HighlightContext(color(1, 0.125f, 0, 0.875f), Black, false, true, false);
	schema->context[IN_WORD_GLOBAL_VARIABLE] = HighlightContext(color(1, 0.625f, 0.625f, 0), Black, false, false, false);
	schema->context[IN_WORD_COMPILER_FUNCTION] = HighlightContext(color(1, 0.3f, 0.2f, 1), Black, false, false, false);
	schema->context[IN_WORD_SPECIAL] = HighlightContext(color(1, 0.625f, 0, 0.625f), Black, false, true, false);
	schema->context[IN_NUMBER] = HighlightContext(color(1, 0.3f, 1, 0.3f), Black, false, false, false);
	schema->context[IN_OPERATOR] = HighlightContext(color(1, 1, 1, 0.75f), Black, false, false, false);
	schema->context[IN_STRING] = HighlightContext(color(1, 1, 0, 0), Black, false, false, false);
	schema->context[IN_MACRO] = HighlightContext(color(1, 0, 0.5f, 0.5f), Black, false, false, false);
	HighlightSchemas.add(schema);
}

HighlightScheme *HighlightScheme::get(const string &name)
{
	if (HighlightSchemas.num == 0)
		init();
	foreach(HighlightScheme *s, HighlightSchemas)
		if (s->name == name)
			return s;
	return HighlightSchemas[0];
}

Array<HighlightScheme*> HighlightScheme::get_all()
{
	if (HighlightSchemas.num == 0)
		init();
	return HighlightSchemas;
}


