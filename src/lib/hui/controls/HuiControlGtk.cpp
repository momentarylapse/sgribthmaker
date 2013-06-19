/*
 * HuiControlGtk.cpp
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#include "HuiControlGtk.h"


HuiControlGtk::HuiControlGtk(int _type, const string &_id) :
	HuiControl(_type, _id)
{
	widget = NULL;
	frame = NULL;
}

HuiControlGtk::~HuiControlGtk()
{
}

void HuiControlGtk::Enable(bool _enabled)
{
    enabled = _enabled;
	gtk_widget_set_sensitive(widget, enabled);
}

bool HuiControlGtk::IsEnabled()
{
	return enabled;
}

void HuiControlGtk::Hide(bool hidden)
{
	if (hidden)
		gtk_widget_hide(widget);
	else
		gtk_widget_show(widget);
}

void HuiControlGtk::SetTooltip(const string& str)
{
	gtk_widget_set_tooltip_text(widget, sys_str(str));
}

void HuiControlGtk::Focus()
{
	gtk_widget_grab_focus(widget);
}
