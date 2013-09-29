/*
 * HuiControlCheckBox.cpp
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#include "HuiControlCheckBox.h"

#ifdef HUI_API_GTK

void OnGtkCheckboxChange(GtkWidget *widget, gpointer data)
{	((HuiControl*)data)->Notify("hui:change");	}

HuiControlCheckBox::HuiControlCheckBox(const string &title, const string &id) :
	HuiControl(HuiKindCheckBox, id)
{
	GetPartStrings(id, title);
	widget = gtk_check_button_new_with_label(sys_str(PartString[0]));
	g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(&OnGtkCheckboxChange), this);
	SetOptions(OptionString);
}

HuiControlCheckBox::~HuiControlCheckBox()
{
}

string HuiControlCheckBox::GetString()
{
	return gtk_button_get_label(GTK_BUTTON(widget));
}

void HuiControlCheckBox::__SetString(const string &str)
{
	gtk_button_set_label(GTK_BUTTON(widget), sys_str(str));
}

void HuiControlCheckBox::__Check(bool checked)
{
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), checked);
}

bool HuiControlCheckBox::IsChecked()
{
	return gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
}

#endif
