/*
 * HighlightSchema.cpp
 *
 *  Created on: 06.06.2013
 *      Author: michi
 */

#include "HighlightSchema.h"
#include "SourceView.h"
#include "lib/hui/hui.h"


string color_to_hex(const color &c)
{
	int r = int(255.0f * c.r);
	int g = int(255.0f * c.g);
	int b = int(255.0f * c.b);
	return "#" + string((char*)&r, 1).hex() + string((char*)&g, 1).hex() + string((char*)&b, 1).hex();
}

void HighlightSchema::apply(SourceView *sv)
{
	for (int i=0; i<NumTagTypes; i++){
		if (context[i].set_bg)
			sv->SetTag(i, color_to_hex(context[i].fg).c_str(), color_to_hex(context[i].bg).c_str(), context[i].bold, context[i].italic);
		else
			sv->SetTag(i, color_to_hex(context[i].fg).c_str(), NULL, context[i].bold, context[i].italic);
	}
	GdkColor _color;
	gdk_color_parse(color_to_hex(fg).c_str(), &_color);
	gtk_widget_modify_base(sv->tv, GTK_STATE_NORMAL, &_color);
}

HighlightSchema GetDefaultSchema()
{
	HighlightSchema schema;
	schema.name = "default";
	schema.fg = Black;
	schema.bg = White;
	schema.context[InLineComment] = HighlightContext(color(1, 0.5f, 0.5f, 0.5f), Black, false, false, true);
	schema.context[InCommentLevel1] = HighlightContext(color(1, 0.5f, 0.5f, 0.5f), Black, false, false, true);
	schema.context[InCommentLevel2] = HighlightContext(color(1, 0.7f, 0.7f, 0.7f), Black, false, false, true);
	schema.context[InSpace] = HighlightContext(Black, Black, false, false, false);
	schema.context[InWord] = HighlightContext(Black, Black, false, false, false);
	schema.context[InWordType] = HighlightContext(color(1, 0.125f, 0, 0.875f), Black, false, true, false);
	schema.context[InWordGameVariable] = HighlightContext(color(1, 0.625f, 0.625f, 0), Black, false, false, false);
	schema.context[InWordCompilerFunction] = HighlightContext(color(1, 0.065f, 0, 0.625f), Black, false, false, false);
	schema.context[InWordSpecial] = HighlightContext(color(1, 0.625f, 0, 0.625f), Black, false, true, false);
	schema.context[InNumber] = HighlightContext(color(1, 0, 0.5f, 0), Black, false, false, false);
	schema.context[InOperator] = HighlightContext(color(1, 0.25f, 0.25f, 0), Black, false, false, false);
	schema.context[InString] = HighlightContext(color(1, 1, 0, 0), Black, false, false, false);
	schema.context[InMacro] = HighlightContext(color(1, 0, 0.5f, 0.5f), Black, false, false, false);
	return schema;
}



