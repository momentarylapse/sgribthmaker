/*
 * HuiControlToggleButtonGtk.cpp
 *
 *  Created on: 18.06.2013
 *      Author: michi
 */

#include "HuiControlToggleButtonGtk.h"

void *get_gtk_image(const string &image, bool large); // -> hui_menu_gtk.cpp

void OnGtkToggleButtonToggle(GtkWidget *widget, gpointer data)
{	((HuiControl*)data)->Notify("hui:change");	}

HuiControlToggleButtonGtk::HuiControlToggleButtonGtk(const string &title, const string &id) :
	HuiControlGtk(HuiKindToggleButton, id)
{
	GetPartStrings(id, title);
	widget = gtk_toggle_button_new_with_label(sys_str(PartString[0]));
	g_signal_connect(G_OBJECT(widget), "toggled", G_CALLBACK(&OnGtkToggleButtonToggle), this);
	//SetInt(id, 0);

}

HuiControlToggleButtonGtk::~HuiControlToggleButtonGtk()
{
	// TODO Auto-generated destructor stub
}

string HuiControlToggleButtonGtk::GetString()
{
}

void HuiControlToggleButtonGtk::SetString(const string &str)
{
	gtk_button_set_label(GTK_BUTTON(widget), sys_str(str));
}

void HuiControlToggleButtonGtk::SetImage(const string& str)
{
	GtkWidget *im = (GtkWidget*)get_gtk_image(str, false);
	gtk_button_set_image(GTK_BUTTON(widget), im);
}

void HuiControlToggleButtonGtk::Check(bool checked)
{
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), checked);
}

bool HuiControlToggleButtonGtk::IsChecked()
{
	return gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
}
