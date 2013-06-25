/*
 * HuiControlRadioButton.cpp
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#include "HuiControlRadioButton.h"

void OnGtkRadioButtonToggle(GtkWidget *widget, gpointer data)
{	((HuiControl*)data)->Notify("hui:change");	}

HuiControlRadioButton::HuiControlRadioButton(const string &title, const string &id) :
	HuiControl(HuiKindRadioButton, id)
{
	GetPartStrings(id, title);
	string group_id = id.substr(0, id.find(":"));
	GSList *group = NULL;
	/*foreach(HuiControl *c, control)
		if (c->type == HuiKindRadioButton)
			if (c->id.find(":"))
				if (c->id.substr(0, c->id.find(":")) == group_id)
					group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(c->widget));*/

	widget = gtk_radio_button_new_with_label(group, sys_str(PartString[0]));
	g_signal_connect(G_OBJECT(widget), "toggled", G_CALLBACK(&OnGtkRadioButtonToggle), this);
	SetInt(0);
}

HuiControlRadioButton::~HuiControlRadioButton() {
	// TODO Auto-generated destructor stub
}

void HuiControlRadioButton::SetString(const string &str)
{
	gtk_button_set_label(GTK_BUTTON(widget), sys_str(str));
}

void HuiControlRadioButton::Check(bool checked)
{
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), checked);
}

string HuiControlRadioButton::GetString()
{
	return gtk_button_get_label(GTK_BUTTON(widget));
}

bool HuiControlRadioButton::IsChecked()
{
	return gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
}

