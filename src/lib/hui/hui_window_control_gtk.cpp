#include "hui.h"
#include "hui_internal.h"
#include "Controls/HuiControlButton.h"
#include "Controls/HuiControlCheckBox.h"
#include "Controls/HuiControlColorButton.h"
#include "Controls/HuiControlComboBox.h"
#include "Controls/HuiControlDrawingArea.h"
#include "Controls/HuiControlEdit.h"
#include "Controls/HuiControlExpander.h"
#include "Controls/HuiControlGrid.h"
#include "Controls/HuiControlGroup.h"
#include "Controls/HuiControlLabel.h"
#include "Controls/HuiControlListView.h"
#include "Controls/HuiControlMultilineEdit.h"
#include "Controls/HuiControlPaned.h"
#include "Controls/HuiControlProgressBar.h"
#include "Controls/HuiControlRadioButton.h"
#include "Controls/HuiControlRevealer.h"
#include "Controls/HuiControlScroller.h"
#include "Controls/HuiControlSeparator.h"
#include "Controls/HuiControlSlider.h"
#include "Controls/HuiControlSpinButton.h"
#include "Controls/HuiControlTabControl.h"
#include "Controls/HuiControlToggleButton.h"
#include "Controls/HuiControlTreeView.h"
#ifdef HUI_API_GTK
#include "../math/math.h"

#ifndef OS_WINDOWS
#include <pango/pangocairo.h>
#endif

namespace hui
{

GtkTreeIter dummy_iter;

void GetPartStrings(const string &id, const string &title);
//string ScanOptions(int id, const string &title);
extern Array<string> PartString;
extern string OptionString, HuiFormatString;


//----------------------------------------------------------------------------------
// creating control items



enum{
	HuiGtkInsertWindow,
	HuiGtkInsertTable,
	HuiGtkInsertTabControl,
};

#if !GTK_CHECK_VERSION(2,22,0)
	void gtk_table_get_size(GtkTable *table, guint *rows, guint *columns)
	{
		g_object_get(G_OBJECT(table), "n-rows", rows, NULL);
		g_object_get(G_OBJECT(table), "n-columns", columns, NULL);
	}
#endif

#if !GTK_CHECK_VERSION(3,0,0)
	GtkWidget *gtk_scale_new_with_range(GtkOrientation orientation, double min, double max, double step)
	{
		if (orientation == GTK_ORIENTATION_VERTICAL)
			return gtk_vscale_new_with_range(min, max, step);
		else
			return gtk_hscale_new_with_range(min, max, step);
	}

	void gtk_combo_box_text_remove_all(GtkComboBoxText *c)
	{
		GtkTreeModel *m = gtk_combo_box_get_model(GTK_COMBO_BOX(c));
		int n = gtk_tree_model_iter_n_children(m, NULL);
		for (int i=0;i<n;i++)
			gtk_combo_box_text_remove(c, 0);
	}

	void gtk_combo_box_text_append(GtkComboBoxText *c, const char *id, const char *text)
	{
		gtk_combo_box_text_append_text(c, text);
	}

	GtkWidget *gtk_box_new(GtkOrientation orientation, int spacing)
	{
		if (orientation == GTK_ORIENTATION_VERTICAL)
			return gtk_vbox_new(false, spacing);
		else
			return gtk_hbox_new(false, spacing);
	}
#endif

void HuiPanel::_insert_control_(HuiControl *c, int x, int y, int width, int height)
{
	GtkWidget *frame = c->get_frame();
	c->panel = this;
	c->is_button_bar = false;
	if (is_resizable){
		if (cur_control){
			if (cur_control->type == HUI_KIND_GRID){
				cur_control->add(c, x, y);
			}else if (cur_control->type == HUI_KIND_TABCONTROL){
				cur_control->add(c, tab_creation_page, 0);
			}else if (cur_control->type == HUI_KIND_GROUP){
				cur_control->add(c, 0, 0);
			}else if (cur_control->type == HUI_KIND_EXPANDER){
				cur_control->add(c, 0, 0);
			}else if (cur_control->type == HUI_KIND_SCROLLER){
				cur_control->add(c, 0, 0);
			}else if (cur_control->type == HUI_KIND_REVEALER){
				cur_control->add(c, 0, 0);
			}
		}else{
			root_control = c;
			// directly into the window...
			//gtk_container_add(GTK_CONTAINER(plugable), frame);
			if (plugable){
				// this = HuiWindow...
				gtk_box_pack_start(GTK_BOX(plugable), frame, true, true, 0);
				gtk_container_set_border_width(GTK_CONTAINER(plugable), border_width);
			}
		}
	}else{
		if ((c->type == HUI_KIND_BUTTON) or (c->type == HUI_KIND_COLORBUTTON) or (c->type == HUI_KIND_COMBOBOX)){
			x -= 1;
			y -= 1;
			width += 2;
			height += 2;
		}else if (c->type == HUI_KIND_LABEL){
			y -= 4;
			height += 8;
		}else if (c->type == HUI_KIND_DRAWINGAREA){
			/*x -= 2;
			y -= 2;*/
		}
		// fixed
		GtkWidget *target_widget = plugable;
		if (cur_control)
			target_widget = gtk_notebook_get_nth_page(GTK_NOTEBOOK(cur_control->widget), tab_creation_page); // selected by SetTabCreationPage()...
			
		gtk_widget_set_size_request(frame, width, height);
		gtk_fixed_put(GTK_FIXED(target_widget), frame, x, y);
	}
	if (frame != c->widget)
		gtk_widget_show(frame);
	gtk_widget_show(c->widget);
	c->enabled = true;
	//c->TabID = TabCreationID;
	//c->TabPage = TabCreationPage;
	/*c->x = 0;
	c->y = 0;*/
	control.add(c);
}

HuiControl *HuiPanel ::_get_control_(const string &id)
{
	if ((id.num == 0) and (cur_id.num > 0))
		return _get_control_(cur_id);

	// search backwards -> multiple AddText()s with identical ids
	//   will always set their own text
	foreachb(HuiControl *c, control)
		if (c->id == id)
			return c;

	if (id.num != 0){
		// ...test if exists in menu/toolbar before reporting an error!
		//msg_error("hui: unknown id: '" + id + "'");
	}
	return NULL;
}

HuiControl *HuiPanel::_get_control_by_widget_(GtkWidget *widget)
{
	for (int j=0;j<control.num;j++)
		if (control[j]->widget == widget)
			return control[j];
	return NULL;
}

string HuiPanel::_get_id_by_widget_(GtkWidget *widget)
{
	for (int j=0;j<control.num;j++)
		if (control[j]->widget == widget)
			return control[j]->id;
	return "";
}



void NotifyWindowByWidget(HuiPanel *panel, GtkWidget *widget, const string &message = "", bool is_default = true)
{
	if (allow_signal_level > 0)
		return;
	msg_db_m("NotifyWindowByWidget", 2);
	string id = panel->_get_id_by_widget_(widget);
	panel->_set_cur_id_(id);
	if (id.num > 0){
		HuiEvent e = HuiEvent(id, message);
		_HuiSendGlobalCommand_(&e);
		e.is_default = is_default;
		panel->_send_event_(&e);
	}
}

void SetImageById(HuiPanel *panel, const string &id)
{
	if ((id == "ok") or (id == "cancel") or (id == "apply"))
		panel->setImage(id, "hui:" + id);
	else if (id != ""){
		for (HuiCommand &c : _HuiCommand_)
			if ((c.id == id) and (c.image != ""))
				panel->setImage(id, c.image);
	}
}



void HuiPanel::addButton(const string &title,int x,int y,int width,int height,const string &id)
{
	_insert_control_(new HuiControlButton(title, id), x, y, width, height);

	SetImageById(this, id);
}

void HuiPanel::addColorButton(const string &title,int x,int y,int width,int height,const string &id)
{
	_insert_control_(new HuiControlColorButton(title, id), x, y, width, height);
}

void HuiPanel::addDefButton(const string &title,int x,int y,int width,int height,const string &id)
{
	addButton(title, x, y, width, height, id);
	GtkWidget *b = control.back()->widget;
	gtk_widget_set_can_default(b, true);
	gtk_widget_grab_default(b);
}




void HuiPanel::addCheckBox(const string &title,int x,int y,int width,int height,const string &id)
{
	_insert_control_(new HuiControlCheckBox(title, id), x, y, width, height);
}

void HuiPanel::addLabel(const string &title,int x,int y,int width,int height,const string &id)
{
	_insert_control_(new HuiControlLabel(title, id), x, y, width, height);
}



void HuiPanel::addEdit(const string &title,int x,int y,int width,int height,const string &id)
{
	_insert_control_(new HuiControlEdit(title, id), x, y, width, height);
}

void HuiPanel::addMultilineEdit(const string &title,int x,int y,int width,int height,const string &id)
{
	_insert_control_(new HuiControlMultilineEdit(title, id), x, y, width, height);
	if (win)
		if ((!win->main_input_control) and ((HuiControlMultilineEdit*)control.back())->handle_keys)
			win->main_input_control = control.back();
}

void HuiPanel::addSpinButton(const string &title,int x,int y,int width,int height,const string &id)
{
	_insert_control_(new HuiControlSpinButton(title, id), x, y, width, height);
}

void HuiPanel::addGroup(const string &title,int x,int y,int width,int height,const string &id)
{
	_insert_control_(new HuiControlGroup(title, id), x, y, width, height);
}

void HuiPanel::addComboBox(const string &title,int x,int y,int width,int height,const string &id)
{
	_insert_control_(new HuiControlComboBox(title, id), x, y, width, height);
}

void HuiPanel::addToggleButton(const string &title,int x,int y,int width,int height,const string &id)
{
	_insert_control_(new HuiControlToggleButton(title, id), x, y, width, height);
}

void HuiPanel::addRadioButton(const string &title,int x,int y,int width,int height,const string &id)
{
	_insert_control_(new HuiControlRadioButton(title, id, this), x, y, width, height);
}

void HuiPanel::addTabControl(const string &title,int x,int y,int width,int height,const string &id)
{
	_insert_control_(new HuiControlTabControl(title, id, this), x, y, width, height);
}

void HuiPanel::setTarget(const string &id,int page)
{
	tab_creation_page = page;
	cur_control = NULL;
	if (id.num > 0)
		for (int i=0;i<control.num;i++)
			if (id == control[i]->id)
				cur_control = control[i];
}

void HuiPanel::addListView(const string &title,int x,int y,int width,int height,const string &id)
{
	_insert_control_(new HuiControlListView(title, id, this), x, y, width, height);
}

void HuiPanel::addTreeView(const string &title,int x,int y,int width,int height,const string &id)
{
	_insert_control_(new HuiControlTreeView(title, id, this), x, y, width, height);
}

void HuiPanel::addIconView(const string &title,int x,int y,int width,int height,const string &id)
{
	msg_todo("AddIconView: deprecated");
	/*
	GetPartStrings(id,title);

	GtkWidget *sw = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	GtkWidget *view;

	GtkListStore *model;
	model = gtk_list_store_new (2, G_TYPE_STRING,GDK_TYPE_PIXBUF);
	view=gtk_icon_view_new_with_model(GTK_TREE_MODEL(model));
	gtk_icon_view_set_text_column(GTK_ICON_VIEW(view),0);
	gtk_icon_view_set_pixbuf_column(GTK_ICON_VIEW(view),1);
	gtk_icon_view_set_item_width(GTK_ICON_VIEW(view),130);
	gtk_icon_view_set_selection_mode(GTK_ICON_VIEW(view), GTK_SELECTION_SINGLE);
	// react on double click
	g_signal_connect(G_OBJECT(view), "item-activated", G_CALLBACK(&OnGtkIconListActivate), this);
	if (OptionString.find("nobar") >= 0)
		gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(view), false);

	GtkWidget *frame = gtk_frame_new(NULL);
	//gtk_widget_set_size_request(frame,width-4,height-4);
	//gtk_fixed_put(GTK_FIXED(cur_cnt),frame,x,y);
	//gtk_container_add(GTK_CONTAINER(frame),c.win);
	gtk_container_add(GTK_CONTAINER(frame),sw);
	gtk_container_add(GTK_CONTAINER(sw), view);
	gtk_widget_show(sw);
	_InsertControl_(view, x, y, width, height, id, HuiKindIconView, frame);*/
}

void HuiPanel::addProgressBar(const string &title,int x,int y,int width,int height,const string &id)
{
	_insert_control_(new HuiControlProgressBar(title, id), x, y, width, height);
}

void HuiPanel::addSlider(const string &title,int x,int y,int width,int height,const string &id)
{
	_insert_control_(new HuiControlSlider(title, id, height > width), x, y, width, height);
}

void HuiPanel::addImage(const string &title,int x,int y,int width,int height,const string &id)
{
	msg_todo("AddImage: deprecated");
	/*GetPartStrings(id, title);
	GtkWidget *im;
	if (PartString[0].num > 0){
		if (PartString[0][0] == '/')
			im = gtk_image_new_from_file(sys_str_f(PartString[0]));
		else
			im = gtk_image_new_from_file(sys_str_f(HuiAppDirectoryStatic + PartString[0]));
	}else
		im = gtk_image_new();
	_InsertControl_(im, x, y, width, height, id, HuiKindImage);*/
}


void HuiPanel::addDrawingArea(const string &title,int x,int y,int width,int height,const string &id)
{
	_insert_control_(new HuiControlDrawingArea(title, id), x, y, width, height);
	if (win and (!win->main_input_control))
		win->main_input_control = control.back();
}


void HuiPanel::addGrid(const string &title, int x, int y, int width, int height, const string &id)
{
	_insert_control_(new HuiControlGrid(title, id, width, height, this), x, y, width, height);
}

void HuiPanel::addExpander(const string &title,int x,int y,int width,int height,const string &id)
{
	_insert_control_(new HuiControlExpander(title, id), x, y, width, height);
}

void HuiPanel::addPaned(const string &title,int x,int y,int width,int height,const string &id)
{
	_insert_control_(new HuiControlPaned(title, id), x, y, width, height);
}

void HuiPanel::addScroller(const string &title,int x,int y,int width,int height,const string &id)
{
	_insert_control_(new HuiControlScroller(title, id), x, y, width, height);
}

void HuiPanel::addSeparator(const string &title,int x,int y,int width,int height,const string &id)
{
	_insert_control_(new HuiControlSeparator(title, id), x, y, width, height);
}

void HuiPanel::addRevealer(const string &title,int x,int y,int width,int height,const string &id)
{
	_insert_control_(new HuiControlRevealer(title, id), x, y, width, height);
}

void HuiPanel::embedDialog(const string &id, int x, int y)
{
	border_width = 8;

	HuiResource *res = HuiGetResource(id);
	if (!res)
		return;
	if (res->type != "SizableDialog")
		return;
	if (res->children.num == 0)
		return;
	HuiResource rr = res->children[0];
	rr.x = x;
	rr.y = y;

	string parent_id;
	if (cur_control)
		parent_id = cur_control->id;
	_addControl(id, rr, parent_id);
}

void hui_rm_event(Array<HuiEventListener> &event, HuiControl *c)
{
	for (int i=0; i<event.num; i++)
		if (event[i].id == c->id){
			//msg_write("erase event");
			event.erase(i);
			i --;
		}
	for (HuiControl *cc : c->children)
		hui_rm_event(event, cc);
}

void HuiPanel::removeControl(const string &id)
{
	HuiControl *c = _get_control_(id);
	if (c){
		hui_rm_event(events, c);
		delete(c);
	}
}



//----------------------------------------------------------------------------------
// drawing

void HuiPanel::redraw(const string &_id)
{
	HuiControl *c = _get_control_(_id);
	if (c){
		GdkWindow *w = gtk_widget_get_window(c->widget);
		if (w)
			gdk_window_invalidate_rect(w, NULL, false);
	}
}

void HuiPanel::redrawRect(const string &_id, int x, int y, int w, int h)
{
	HuiControl *c = _get_control_(_id);
	if (c){
		if (w < 0){
			x += w;
			w = - w;
		}
		if (h < 0){
			y += h;
			h = - h;
		}
		GdkRectangle r;
		r.x = x;
		r.y = y;
		r.width = w;
		r.height = h;
		gdk_window_invalidate_rect(gtk_widget_get_window(c->widget), &r, false);
	}
}

};


#endif
