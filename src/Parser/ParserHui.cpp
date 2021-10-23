/*
 * ParserHui.cpp
 *
 *  Created on: 27.03.2021
 *      Author: michi
 */

#include "ParserHui.h"
#include "../HighlightScheme.h"

ParserHui::ParserHui() : Parser("Hui") {
	macro_begin = "#";
	line_comment_begin = "//";
	multi_comment_begin = "/*";
	multi_comment_end = "*/";
	special_words.add("width");
	special_words.add("height");
	special_words.add("expandx");
	special_words.add("expandy");
	special_words.add("noexpandx");
	special_words.add("noexpandy");
	special_words.add("bold");
	special_words.add("italic");
	special_words.add("disabled");
	special_words.add("right");
	special_words.add("flat");
	special_words.add("big");
	special_words.add("huge");
	special_words.add("small");
	special_words.add("image");
	special_words.add("vertical");
	special_words.add("horizontal");
	special_words.add("range");
	special_words.add("margin");
	special_words.add("marginleft");
	special_words.add("marginright");
	special_words.add("margintop");
	special_words.add("marginbottom");
	special_words.add("padding");
	special_words.add("hgroup");
	special_words.add("vgroup");
	special_words.add("indent");
	special_words.add("noindent");
	special_words.add("grabfocus");
	special_words.add("ignorefocus");
	special_words.add("link");
	special_words.add("danger");
	special_words.add("margin");
	special_words.add("alpha");
	special_words.add("editable");
	special_words.add("opengl");
	special_words.add("clearplaceholder");
	special_words.add("clearcompletion");
	special_words.add("placeholder");
	special_words.add("completion");
	special_words.add("actionbar");
	special_words.add("wrap");
	special_words.add("nowrap");
	special_words.add("center");
	special_words.add("angle");
	special_words.add("multiline");
	special_words.add("selectmulti");
	special_words.add("reorderable");
	special_words.add("format");
	special_words.add("noframe");
	special_words.add("handlekeys");
	special_words.add("monospace");
	special_words.add("tabsize");
	special_words.add("slide");
	special_words.add("up");
	special_words.add("down");
	special_words.add("left");
	special_words.add("right");
	special_words.add("crossfade");
	special_words.add("scrollx");
	special_words.add("scrolly");
	special_words.add("origin");
	special_words.add("showvalue");
	special_words.add("mark");
	special_words.add("clearmarks");
	special_words.add("important");
	special_words.add("checkable");
	special_words.add("style");
	special_words.add("text");
	special_words.add("icons");
	special_words.add("both");
	special_words.add("default");
	special_words.add("homogenousx");
	special_words.add("buttonbar");
	special_words.add("headerbar");
	special_words.add("closebutton");
	special_words.add("resizable");
	special_words.add("closable");
	special_words.add("cursor");
	special_words.add("borderwidth");
	special_words.add("yes");
	special_words.add("no");
	special_words.add("true");
	special_words.add("false");
	special_words.add("bar");
	special_words.add("nobar");
	special_words.add("buttonbar");
	types.add("Dialog");
	types.add("Window");
	types.add("Menu");
	types.add("Label");
	types.add("Group");
	types.add("Button");
	types.add("Grid");
	types.add("Edit");
	types.add("MultilineEdit");
	types.add("ColorButton");
	types.add("CheckBox");
	types.add("RadioButton");
	types.add("DrawingArea");
	types.add("ComboBox");
	types.add("SpinButton");
	types.add("Slider");
	types.add("ProgressBar");
	types.add("TabControl");
	types.add("ListView");
	types.add("TreeView");
	types.add("Item");
	types.add("Separator");
	types.add("Scroller");
	types.add("ToggleButton");
	types.add("Paned");
	types.add("Revealer");
	types.add("Expander");
}

void ParserHui::CreateTextColors(SourceView *sv, int first_line, int last_line) {
	CreateTextColorsDefault(sv, first_line, last_line);
}
