/*
 * HuiControlButtonGtk.cpp
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#include "HuiControlButtonGtk.h"

void *get_gtk_image(const string &image, bool large); // -> hui_menu_gtk.cpp

void OnGtkButtonPress(GtkWidget *widget, gpointer data)
{	((HuiControlGtk*)data)->Notify("hui:click");	}

HuiControlButtonGtk::HuiControlButtonGtk(const string &title, const string &id) :
	HuiControlGtk(HuiKindButton, id)
{
	GetPartStrings(id, title);
	widget = gtk_button_new_with_label(sys_str(PartString[0]));
	g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(&OnGtkButtonPress), this);

//	SetImageById(this, id);
}

HuiControlButtonGtk::~HuiControlButtonGtk() {
	// TODO Auto-generated destructor stub
}

string HuiControlButtonGtk::GetString()
{
	return gtk_button_get_label(GTK_BUTTON(widget));
}

void HuiControlButtonGtk::SetString(const string &str)
{
	gtk_button_set_label(GTK_BUTTON(widget), sys_str(str));
}

void HuiControlButtonGtk::SetImage(const string& str)
{
	GtkWidget *im = (GtkWidget*)get_gtk_image(str, false);
	gtk_button_set_image(GTK_BUTTON(widget), im);
}
