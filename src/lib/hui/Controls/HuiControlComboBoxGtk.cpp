/*
 * HuiControlComboBox.cpp
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#include "HuiControlComboBox.h"

void OnGtkComboboxChange(GtkWidget *widget, gpointer data)
{	((HuiControl*)data)->Notify("hui:change");	}

HuiControlComboBox::HuiControlComboBox(const string &title, const string &id) :
	HuiControl(HuiKindComboBox, id)
{
	GetPartStrings(id, title);
	widget = gtk_combo_box_text_new();
	g_signal_connect(G_OBJECT(widget), "changed", G_CALLBACK(&OnGtkComboboxChange), this);
	if ((PartString.num > 1) || (PartString[0] != ""))
		for (int i=0;i<PartString.num;i++)
			SetString(PartString[i]);
	SetInt(0);
}

HuiControlComboBox::~HuiControlComboBox() {
	// TODO Auto-generated destructor stub
}

string HuiControlComboBox::GetString()
{
}

void HuiControlComboBox::SetString(const string &str)
{
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(widget),sys_str(str));
	//_item_.add(dummy_iter);
}

void HuiControlComboBox::AddString(const string& str)
{
	gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(widget), NULL, sys_str(str));
}

void HuiControlComboBox::SetInt(int i)
{
	gtk_combo_box_set_active(GTK_COMBO_BOX(widget), i);
}

int HuiControlComboBox::GetInt()
{
}

void HuiControlComboBox::Reset()
{
	gtk_combo_box_text_remove_all(GTK_COMBO_BOX_TEXT(widget));
}