/*
 * HuiControlProgressBarGtk.cpp
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#include "HuiControlProgressBarGtk.h"

HuiControlProgressBarGtk::HuiControlProgressBarGtk(const string &title, const string &id) :
	HuiControlGtk(HuiKindProgressBar, id)
{
	GetPartStrings(id, title);
	widget = gtk_progress_bar_new();
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(widget), sys_str(PartString[0]));
	//g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(&OnGtkButtonPress), this);
}

HuiControlProgressBarGtk::~HuiControlProgressBarGtk() {
	// TODO Auto-generated destructor stub
}

string HuiControlProgressBarGtk::GetString()
{
}

void HuiControlProgressBarGtk::SetString(const string &str)
{
	gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(widget), true);
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(widget), sys_str(str));
}

float HuiControlProgressBarGtk::GetFloat()
{
}

void HuiControlProgressBarGtk::SetFloat(float f)
{
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(widget), min(max(f, 0), 1));
}
