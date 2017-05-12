/*
 * ControlGrid.cpp
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#include "ControlGrid.h"
#include "../hui.h"

#ifdef HUI_API_GTK

namespace hui
{

ControlGrid::ControlGrid(const string &title, const string &id, int num_x, int num_y, Panel *panel) :
	Control(CONTROL_GRID, id)
{
	GetPartStrings(title);
	widget = gtk_grid_new();
	gtk_grid_set_row_spacing(GTK_GRID(widget), panel->border_width);
	gtk_grid_set_column_spacing(GTK_GRID(widget), panel->border_width);
	setOptions(OptionString);
	button_bar = (OptionString.find("buttonbar") >= 0);
	if (button_bar){
		gtk_widget_set_margin_top(widget, 7);
		gtk_widget_set_margin_bottom(widget, 4);
		gtk_widget_set_halign(widget, GTK_ALIGN_END);
	}
}

void ControlGrid::add(Control *child, int x, int y)
{
	GtkWidget *child_widget = child->get_frame();
	gtk_grid_attach(GTK_GRID(widget), child_widget, x, y, 1, 1);
	child->parent = this;
	children.add(child);

	if (button_bar){
		int width, height;
		gtk_widget_get_size_request(child_widget, &width, &height);
		if (width < 0)
			gtk_widget_set_size_request(child_widget, 100, height);
		//gtk_widget_set_halign(child_widget, GTK_ALIGN_END);
	}
}

};

#endif
