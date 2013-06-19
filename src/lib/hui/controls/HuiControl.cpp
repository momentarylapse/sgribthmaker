/*
 * HuiControl.cpp
 *
 *  Created on: 18.06.2013
 *      Author: michi
 */

#include "HuiControl.h"
#include "../hui.h"


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

