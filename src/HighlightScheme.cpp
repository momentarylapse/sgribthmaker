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
	schema->context[InLineComment] = HighlightContext(color(1, 0.5f, 0.5f, 0.5f), Black, false, false, true);
	schema->context[InCommentLevel1] = HighlightContext(color(1, 0.5f, 0.5f, 0.5f), Black, false, false, true);
	schema->context[InCommentLevel2] = HighlightContext(color(1, 0.7f, 0.7f, 0.7f), Black, false, false, true);
	schema->context[InSpace] = HighlightContext(Black, Black, false, false, false);
	schema->context[InWord] = HighlightContext(Black, Black, false, false, false);
	schema->context[InWordType] = HighlightContext(color(1, 0.125f, 0, 0.875f), Black, false, true, false);
	schema->context[InWordGameVariable] = HighlightContext(color(1, 0.625f, 0.625f, 0), Black, false, false, false);
	schema->context[InWordCompilerFunction] = HighlightContext(color(1, 0.065f, 0, 0.625f), Black, false, false, false);
	schema->context[InWordSpecial] = HighlightContext(color(1, 0.625f, 0, 0.625f), Black, false, true, false);
	schema->context[InNumber] = HighlightContext(color(1, 0, 0.5f, 0), Black, false, false, false);
	schema->context[InOperator] = HighlightContext(color(1, 0.25f, 0.25f, 0), Black, false, false, false);
	schema->context[InString] = HighlightContext(color(1, 1, 0, 0), Black, false, false, false);
	schema->context[InMacro] = HighlightContext(color(1, 0, 0.5f, 0.5f), Black, false, false, false);
	HighlightSchemas.add(schema);

	schema = new HighlightScheme;
	schema->name = "dark";
	schema->is_default = true;
	schema->bg = Black;
	schema->context[InLineComment] = HighlightContext(color(1, 0.5f, 0.5f, 0.5f), Black, false, false, true);
	schema->context[InCommentLevel1] = HighlightContext(color(1, 0.5f, 0.5f, 0.5f), Black, false, false, true);
	schema->context[InCommentLevel2] = HighlightContext(color(1, 0.3f, 0.7f, 0.7f), Black, false, false, true);
	schema->context[InSpace] = HighlightContext(White, Black, false, false, false);
	schema->context[InWord] = HighlightContext(White, Black, false, false, false);
	schema->context[InWordType] = HighlightContext(color(1, 0.125f, 0, 0.875f), Black, false, true, false);
	schema->context[InWordGameVariable] = HighlightContext(color(1, 0.625f, 0.625f, 0), Black, false, false, false);
	schema->context[InWordCompilerFunction] = HighlightContext(color(1, 0.3f, 0.2f, 1), Black, false, false, false);
	schema->context[InWordSpecial] = HighlightContext(color(1, 0.625f, 0, 0.625f), Black, false, true, false);
	schema->context[InNumber] = HighlightContext(color(1, 0.3f, 1, 0.3f), Black, false, false, false);
	schema->context[InOperator] = HighlightContext(color(1, 1, 1, 0.75f), Black, false, false, false);
	schema->context[InString] = HighlightContext(color(1, 1, 0, 0), Black, false, false, false);
	schema->context[InMacro] = HighlightContext(color(1, 0, 0.5f, 0.5f), Black, false, false, false);
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


