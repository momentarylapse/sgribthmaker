/*
 * HuiControlCheckBoxGtk.cpp
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#include "HuiControlCheckBoxGtk.h"

void OnGtkCheckboxChange(GtkWidget *widget, gpointer data)
{	((HuiControlGtk*)data)->Notify("hui:change");	}

HuiControlCheckBoxGtk::HuiControlCheckBoxGtk(const string &title, const string &id) :
	HuiControlGtk(HuiKindCheckBox, id)
{
	GetPartStrings(id, title);
	widget = gtk_check_button_new_with_label(sys_str(PartString[0]));
	g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(&OnGtkCheckboxChange), this);
}

HuiControlCheckBoxGtk::~HuiControlCheckBoxGtk() {
	// TODO Auto-generated destructor stub
}

string HuiControlCheckBoxGtk::GetString()
{
	return gtk_button_get_label(GTK_BUTTON(widget));
}

void HuiControlCheckBoxGtk::SetString(const string &str)
{
	gtk_button_set_label(GTK_BUTTON(widget), sys_str(str));
}

void HuiControlCheckBoxGtk::Check(bool checked)
{
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), checked);
}

bool HuiControlCheckBoxGtk::IsChecked()
{
	return gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
}
