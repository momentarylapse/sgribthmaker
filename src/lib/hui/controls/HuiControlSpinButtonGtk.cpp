/*
 * HuiControlSpinButtonGtk.cpp
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#include "HuiControlSpinButtonGtk.h"

void OnGtkEditChange(GtkWidget *widget, gpointer data);

HuiControlSpinButtonGtk::HuiControlSpinButtonGtk(const string &title, const string &id) :
	HuiControlGtk(HuiKindSpinButton, id)
{
	GetPartStrings(id, title);
	float vmin = -100000000000.0f;
	float vmax = 100000000000.0f;
	float step = 1;
	if (PartString.num >= 2)
		vmin = s2f(PartString[1]);
	if (PartString.num >= 3)
		vmax = s2f(PartString[2]);
	if (PartString.num >= 4)
		step = s2f(PartString[3]);
	widget = gtk_spin_button_new_with_range(vmin, vmax, step);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget), s2f(PartString[0]));
	gtk_entry_set_activates_default(GTK_ENTRY(widget), true);
	g_signal_connect(G_OBJECT(widget), "changed", G_CALLBACK(&OnGtkEditChange), this);
}

HuiControlSpinButtonGtk::~HuiControlSpinButtonGtk() {
	// TODO Auto-generated destructor stub
}

string HuiControlSpinButtonGtk::GetString()
{
	return f2s(gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget)), gtk_spin_button_get_digits(GTK_SPIN_BUTTON(widget)));
	//return de_sys_str(gtk_entry_get_text(GTK_ENTRY(widget)));
}

void HuiControlSpinButtonGtk::SetString(const string &str)
{
	gtk_entry_set_text(GTK_ENTRY(widget), sys_str(str));
}

void HuiControlSpinButtonGtk::SetInt(int i)
{
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget), i);
}

int HuiControlSpinButtonGtk::GetInt()
{
	return gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
}

float HuiControlSpinButtonGtk::GetFloat()
{
	return gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget));
}

void HuiControlSpinButtonGtk::SetFloat(float f)
{
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget), f);
}
