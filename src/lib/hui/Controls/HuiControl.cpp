/*
 * HuiControl.cpp
 *
 *  Created on: 18.06.2013
 *      Author: michi
 */

#include "HuiControl.h"
#include "../hui.h"

HuiControl::HuiControl(int _type, const string &_id)
{
	type = _type;
	id = _id;
	win = NULL;
	enabled = true;
#ifdef HUI_API_WIN
	hWND = NULL;
#endif
#ifdef HUI_API_GTK
	widget = NULL;
	frame = NULL;
#endif
}

#ifdef HUI_API_GTK

void HuiControl::Enable(bool _enabled)
{
    enabled = _enabled;
	gtk_widget_set_sensitive(widget, enabled);
}

void HuiControl::Hide(bool hidden)
{
	if (hidden)
		gtk_widget_hide(widget);
	else
		gtk_widget_show(widget);
}

void HuiControl::SetTooltip(const string& str)
{
	gtk_widget_set_tooltip_text(widget, sys_str(str));
}

void HuiControl::Focus()
{
	gtk_widget_grab_focus(widget);
}

#endif

bool HuiControl::IsEnabled()
{
	return enabled;
}

void HuiControl::Notify(const string &message, bool is_default)
{
	if (allow_signal_level > 0)
		return;
	msg_db_m("Control.Notify", 2);
	win->_SetCurID_(id);
	if (id.num > 0){
		HuiEvent e = HuiEvent(id, message);
		_HuiSendGlobalCommand_(&e);
		e.is_default = is_default;
		win->_SendEvent_(&e);
	}
}

