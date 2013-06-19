/*
 * HuiControlMultilineEditGtk.cpp
 *
 *  Created on: 18.06.2013
 *      Author: michi
 */

#include "HuiControlMultilineEditGtk.h"

void OnGtkMultilineEditChange(GtkWidget *widget, gpointer data)
{	((HuiControlGtk*)data)->Notify("hui:change");	}

HuiControlMultilineEditGtk::HuiControlMultilineEditGtk(const string &title, const string &id) :
	HuiControlGtk(HuiKindMultilineEdit, id)
{
	GetPartStrings(id, title);
	GtkTextBuffer *tb = gtk_text_buffer_new(NULL);
	widget = gtk_text_view_new_with_buffer(tb);
	GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_widget_show(scroll);
	gtk_container_add(GTK_CONTAINER(scroll), widget);

	// frame
	frame = scroll;
//	if (border_width > 0){
		frame = gtk_frame_new(NULL);
		gtk_container_add(GTK_CONTAINER(frame), scroll);
//	}
	g_signal_connect(G_OBJECT(widget), "changed", G_CALLBACK(&OnGtkMultilineEditChange), this);
}

HuiControlMultilineEditGtk::~HuiControlMultilineEditGtk()
{
	// TODO Auto-generated destructor stub
}

void HuiControlMultilineEditGtk::SetString(const string &str)
{
	GtkTextBuffer *tb = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
	const char *str2 = sys_str(str);
	gtk_text_buffer_set_text(tb, str2, strlen(str2));
}

string HuiControlMultilineEditGtk::GetString()
{
	GtkTextBuffer *tb = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
	GtkTextIter is, ie;
	gtk_text_buffer_get_iter_at_offset(tb, &is, 0);
	gtk_text_buffer_get_iter_at_offset(tb, &ie, -1);
	return de_sys_str(gtk_text_buffer_get_text(tb, &is, &ie, false));
}

void HuiControlMultilineEditGtk::AddString(const string& str)
{
}
