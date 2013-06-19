#include "hui.h"
#include "hui_internal.h"
#include "controls/HuiControlButtonGtk.h"
#include "controls/HuiControlCheckBoxGtk.h"
#include "controls/HuiControlColorButtonGtk.h"
#include "controls/HuiControlComboBoxGtk.h"
#include "controls/HuiControlDrawingAreaGtk.h"
#include "controls/HuiControlEditGtk.h"
#include "controls/HuiControlGridGtk.h"
#include "controls/HuiControlGroupGtk.h"
#include "controls/HuiControlLabelGtk.h"
#include "controls/HuiControlListViewGtk.h"
#include "controls/HuiControlMultilineEditGtk.h"
#include "controls/HuiControlProgressBarGtk.h"
#include "controls/HuiControlRadioButtonGtk.h"
#include "controls/HuiControlSliderGtk.h"
#include "controls/HuiControlSpinButtonGtk.h"
#include "controls/HuiControlTabControlGtk.h"
#include "controls/HuiControlToggleButtonGtk.h"
#include "controls/HuiControlTreeViewGtk.h"
#ifdef HUI_API_GTK
#include "../math/math.h"

#ifndef OS_WINDOWS
#include <pango/pangocairo.h>
#endif

GtkTreeIter dummy_iter;

void GetPartStrings(const string &id, const string &title);
//string ScanOptions(int id, const string &title);
extern Array<string> PartString;
extern string OptionString, HuiFormatString;


//----------------------------------------------------------------------------------
// creating control items



enum{
	HuiGtkInsertContainer,
	HuiGtkInsertTable,
	HuiGtkInsertTabControl,
};

#if GTK_MAJOR_VERSION == 2
#if GTK_MINOR_VERSION < 22
	void gtk_table_get_size(GtkTable *table, guint *rows, guint *columns)
	{
		g_object_get(G_OBJECT(table), "n-rows", rows, NULL);
		g_object_get(G_OBJECT(table), "n-columns", columns, NULL);
	}
#endif

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

void HuiWindow::_InsertControl_(HuiControl *_c, int x, int y, int width, int height)
{
	HuiControlGtk *c = dynamic_cast<HuiControlGtk*>(_c);
	GtkWidget *frame = c->frame;
	if (!frame)
		frame = c->widget;
	c->win = this;
	c->is_button_bar = false;
	if (is_resizable){
		HuiControl *root_ctrl = cur_control;
		GtkWidget *target_widget = plugable;
		int root_type = HuiGtkInsertContainer;
		if (root_ctrl){
			if (root_ctrl->type == HuiKindControlTable){
				root_type = HuiGtkInsertTable;
				target_widget = dynamic_cast<HuiControlGtk*>(root_ctrl)->widget;
				unsigned int n_cols, n_rows;
				gtk_table_get_size(GTK_TABLE(target_widget), &n_rows, &n_cols);

				// top table?
				if (cur_control == control[0])
					if ((n_cols == 1) && (y == (n_rows - 1)))
						c->is_button_bar = true;
			}else if (root_ctrl->type == HuiKindTabControl){
				root_type = HuiGtkInsertTabControl;
				target_widget = dynamic_cast<HuiControlGtk*>(root_ctrl)->widget;
				target_widget = gtk_notebook_get_nth_page(GTK_NOTEBOOK(target_widget), tab_creation_page); // selected by SetTabCreationPage()...
			}else
				root_type = -1;
		}
		bool root_is_button_bar = false;
		if (root_ctrl)
			if (root_ctrl->is_button_bar)
				root_is_button_bar = true;
		// insert
		if (root_type == HuiGtkInsertContainer){
			// directly into the window...
			//gtk_container_add(GTK_CONTAINER(target_widget), frame);
			gtk_box_pack_start(GTK_BOX(target_widget), frame, true, true, 0);
	//		_cur_widget_ = frame;
			/*if ((kind == HuiKindListView) || (kind == HuiKindDrawingArea) || (kind == HuiKindMultilineEdit))
				gtk_container_set_border_width(GTK_CONTAINER(target_widget), 0);
			else*/
				gtk_container_set_border_width(GTK_CONTAINER(target_widget), border_width);
		}else if (root_type == HuiGtkInsertTable){
			GtkAttachOptions op_x = GtkAttachOptions(GTK_FILL | GTK_EXPAND);
			GtkAttachOptions op_y = GtkAttachOptions(GTK_FILL | GTK_EXPAND);
			if (c->type == HuiKindButton){
				//gtk_widget_set_size_request(frame, 100, 30);
				const char *_label = gtk_button_get_label(GTK_BUTTON(c->widget));
				if ((_label) && (strlen(_label) > 0)){ // != NULL ... cause set even if ""
					if (root_is_button_bar){
						op_x = GtkAttachOptions(GTK_FILL);
						//gtk_widget_set_size_request(frame, 120, 30);
						gtk_widget_set_size_request(frame, -1, 30);
					}else{
						op_x = GtkAttachOptions(GTK_FILL | GTK_EXPAND);
						gtk_widget_set_size_request(frame, -1, 30);
					}
				}else{
					op_x = GtkAttachOptions(GTK_FILL);
					gtk_widget_set_size_request(frame, 30, 30);
				}
				op_y = GtkAttachOptions(GTK_FILL);// | GTK_SHRINK);
			/*}else if ((type == HuiKindEdit) || (kind == HuiKindComboBox)  || (type == HuiKindCheckBox) || (type == HuiKindControlTable)){
				op_y = GtkAttachOptions(GTK_FILL);// | GTK_SHRINK);*/
			}else if (c->type == HuiKindColorButton){
				op_y = GtkAttachOptions(GTK_FILL);// | GTK_SHRINK);
				gtk_widget_set_size_request(frame, 100, 28);
			}else if (c->type == HuiKindComboBox){
				op_y = GtkAttachOptions(GTK_FILL);// | GTK_SHRINK);
				gtk_widget_set_size_request(frame, -1, 28);
			}else if ((c->type == HuiKindEdit) || (c->type == HuiKindSpinButton) || (c->type == HuiKindCheckBox) || (c->type == HuiKindRadioButton) || (c->type == HuiKindSlider) || (c->type == HuiKindProgressBar)){
				op_y = GtkAttachOptions(GTK_FILL);// | GTK_SHRINK);
				gtk_widget_set_size_request(frame, -1, 25);
			}else if (c->type == HuiKindControlTable){
				//op_y = GtkAttachOptions(GTK_FILL);// | GTK_SHRINK);
			}else if (c->type == HuiKindText){
				//op_x = GtkAttachOptions(GTK_FILL | GTK_EXPAND);
				if (!root_is_button_bar)
					op_x = GtkAttachOptions(GTK_FILL);
				op_y = GtkAttachOptions(GTK_FILL);
			}
			if (OptionString.find("noexpandy") >= 0)
				op_y = GtkAttachOptions(GTK_FILL);
			else if (OptionString.find("expandy") >= 0)
				op_y = GtkAttachOptions(GTK_FILL | GTK_EXPAND);
			if (OptionString.find("noexpandx") >= 0)
				op_x = GtkAttachOptions(GTK_FILL);
			else if (OptionString.find("expandx") >= 0)
				op_x = GtkAttachOptions(GTK_FILL | GTK_EXPAND);
			if (OptionString.find("width") >= 0){
				string ww = OptionString.substr(OptionString.find("width") + 6, -1);
				if (ww.find(","))
					ww = ww.substr(0, ww.find(","));
				int width = s2i(ww);
				gtk_widget_set_size_request(frame, width, 28);
				op_x = GtkAttachOptions(0);
			}

			// TODO
			unsigned int nx, ny;
			gtk_table_get_size(GTK_TABLE(target_widget), &ny, &nx);
			if (x >= nx){
				y += (x / nx);
				x = (x % nx);
			}

			
			gtk_table_attach(GTK_TABLE(target_widget), frame, x, x+1, y, y+1, op_x, op_y, 0, 0);
			if (root_is_button_bar)
				gtk_container_child_set(GTK_CONTAINER(target_widget), frame, "y-padding", 7, NULL);
		}else if (root_type == HuiGtkInsertTabControl){
			gtk_container_add(GTK_CONTAINER(target_widget), frame);
			gtk_container_set_border_width(GTK_CONTAINER(target_widget), border_width);
		}
	}else{
		if ((c->type == HuiKindButton) || (c->type == HuiKindColorButton) || (c->type == HuiKindComboBox)){
			x -= 1;
			y -= 1;
			width += 2;
			height += 2;
		}else if (c->type == HuiKindText){
			y -= 4;
			height += 8;
		}else if (c->type == HuiKindDrawingArea){
			/*x -= 2;
			y -= 2;*/
		}
		// fixed
		GtkWidget *target_widget = plugable;
		if (cur_control)
			target_widget = gtk_notebook_get_nth_page(GTK_NOTEBOOK(dynamic_cast<HuiControlGtk*>(cur_control)->widget), tab_creation_page); // selected by SetTabCreationPage()...
			
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

HuiControl *HuiWindow ::_GetControl_(const string &id)
{
	if ((id.num == 0) && (cur_id.num > 0))
		return _GetControl_(cur_id);

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

HuiControl *HuiWindow::_GetControlByWidget_(GtkWidget *widget)
{
	for (int j=0;j<control.num;j++)
		if (((HuiControlGtk*)control[j])->widget == widget)
			return control[j];
	return NULL;
}

string HuiWindow::_GetIDByWidget_(GtkWidget *widget)
{
	// controls
	for (int j=0;j<control.num;j++)
		if (((HuiControlGtk*)control[j])->widget == widget)
			return control[j]->id;

	// toolbars
	for (int t=0;t<4;t++)
		for (int j=0;j<toolbar[t].item.num;j++)
			if ((GtkWidget*)toolbar[t].item[j].widget == widget)
				return toolbar[t].item[j].id;
	return "";
}



void NotifyWindowByWidget(HuiWindow *win, GtkWidget *widget, const string &message = "", bool is_default = true)
{
	if (allow_signal_level > 0)
		return;
	msg_db_m("NotifyWindowByWidget", 2);
	string id = win->_GetIDByWidget_(widget);
	win->_SetCurID_(id);
	if (id.num > 0){
		HuiEvent e = HuiEvent(id, message);
		_HuiSendGlobalCommand_(&e);
		e.is_default = is_default;
		win->_SendEvent_(&e);
	}
}

void SetImageById(HuiWindow *win, const string &id)
{
	if ((id == "ok") || (id == "cancel") || (id == "apply"))
		win->SetImage(id, "hui:" + id);
	else if (id != ""){
		foreach(HuiCommand &c, _HuiCommand_)
			if ((c.id == id) && (c.image != ""))
				win->SetImage(id, c.image);
	}
}



void HuiWindow::AddButton(const string &title,int x,int y,int width,int height,const string &id)
{
	_InsertControl_(new HuiControlButtonGtk(title, id), x, y, width, height);

	SetImageById(this, id);
}

void HuiWindow::AddColorButton(const string &title,int x,int y,int width,int height,const string &id)
{
	_InsertControl_(new HuiControlColorButtonGtk(title, id), x, y, width, height);
}

void HuiWindow::AddDefButton(const string &title,int x,int y,int width,int height,const string &id)
{
	AddButton(title, x, y, width, height, id);
	GtkWidget *b = ((HuiControlGtk*)control.back())->widget;
#ifdef OS_WINDOWS
	GTK_WIDGET_SET_FLAGS(b, GTK_CAN_DEFAULT);
#else
	gtk_widget_set_can_default(b, true);
#endif
	gtk_widget_grab_default(b);
}




void HuiWindow::AddCheckBox(const string &title,int x,int y,int width,int height,const string &id)
{
	_InsertControl_(new HuiControlCheckBoxGtk(title, id), x, y, width, height);
}

void HuiWindow::AddText(const string &title,int x,int y,int width,int height,const string &id)
{
	_InsertControl_(new HuiControlLabelGtk(title, id), x, y, width, height);
}



void HuiWindow::AddEdit(const string &title,int x,int y,int width,int height,const string &id)
{
	_InsertControl_(new HuiControlEditGtk(title, id), x, y, width, height);

	// dumb but usefull test
/*	if (height > 30){
		GtkStyle* style = gtk_widget_get_style(text);
		PangoFontDescription *font_desc = pango_font_description_copy(style->font_desc);
		pango_font_description_set_absolute_size(font_desc, height * PANGO_SCALE * 0.95);
		gtk_widget_modify_font(text, font_desc);
	}*/
}

void HuiWindow::AddMultilineEdit(const string &title,int x,int y,int width,int height,const string &id)
{
	_InsertControl_(new HuiControlMultilineEditGtk(title, id), x, y, width, height);
}

void HuiWindow::AddSpinButton(const string &title,int x,int y,int width,int height,const string &id)
{
	_InsertControl_(new HuiControlSpinButtonGtk(title, id), x, y, width, height);
}

void HuiWindow::AddGroup(const string &title,int x,int y,int width,int height,const string &id)
{
	_InsertControl_(new HuiControlGroupGtk(title, id), x, y, width, height);
}

void HuiWindow::AddComboBox(const string &title,int x,int y,int width,int height,const string &id)
{
	_InsertControl_(new HuiControlComboBoxGtk(title, id), x, y, width, height);
	if ((PartString.num > 1) || (PartString[0] != ""))
		for (int i=0;i<PartString.num;i++)
			AddString(id, PartString[i]);
	SetInt(id, 0);
}

void HuiWindow::AddToggleButton(const string &title,int x,int y,int width,int height,const string &id)
{
	_InsertControl_(new HuiControlToggleButtonGtk(title, id), x, y, width, height);
}

void HuiWindow::AddRadioButton(const string &title,int x,int y,int width,int height,const string &id)
{
	_InsertControl_(new HuiControlRadioButtonGtk(title, id), x, y, width, height);
}

void HuiWindow::AddTabControl(const string &title,int x,int y,int width,int height,const string &id)
{
	_InsertControl_(new HuiControlTabControlGtk(title, id, this), x, y, width, height);
}

void HuiWindow::SetTarget(const string &id,int page)
{
	tab_creation_page = page;
	cur_control = NULL;
	if (id.num > 0)
		for (int i=0;i<control.num;i++)
			if (id == control[i]->id)
				cur_control = control[i];
}

void HuiWindow::AddListView(const string &title,int x,int y,int width,int height,const string &id)
{
	_InsertControl_(new HuiControlListViewGtk(title, id, this), x, y, width, height);
}

void HuiWindow::AddTreeView(const string &title,int x,int y,int width,int height,const string &id)
{
	_InsertControl_(new HuiControlTreeViewGtk(title, id, this), x, y, width, height);
}

void HuiWindow::AddIconView(const string &title,int x,int y,int width,int height,const string &id)
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

void HuiWindow::AddProgressBar(const string &title,int x,int y,int width,int height,const string &id)
{
	_InsertControl_(new HuiControlProgressBarGtk(title, id), x, y, width, height);
}

void HuiWindow::AddSlider(const string &title,int x,int y,int width,int height,const string &id)
{
	_InsertControl_(new HuiControlSliderGtk(title, id, height > width), x, y, width, height);
}

void HuiWindow::AddImage(const string &title,int x,int y,int width,int height,const string &id)
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


void HuiWindow::AddDrawingArea(const string &title,int x,int y,int width,int height,const string &id)
{
	_InsertControl_(new HuiControlDrawingAreaGtk(title, id), x, y, width, height);
	input_widget = ((HuiControlGtk*)control.back())->widget;
}


void HuiWindow::AddControlTable(const string &title, int x, int y, int width, int height, const string &id)
{
	_InsertControl_(new HuiControlGridGtk(title, id, width, height, this), x, y, width, height);
}

void HuiWindow::EmbedDialog(const string &id, int x, int y)
{
#if 0
	HuiWindow *dlg = HuiCreateResourceDialog(id, NULL, NULL);
	dlg->Update();

	for (int i=0;i<dlg->control.num;i++)
		control.add(dlg->control[i]);

	gtk_widget_unparent(dlg->control[0]->widget);

	//GtkWidget *b = gtk_button_new_with_label("test");

	
	_InsertControl_(this, dlg->control[0]->widget, root, page, 10, 10, 86237, dlg->control[0]->type);
	//_InsertControl_(this, b, root, page, 10, 10, 86237, dlg->control[0]->type);

	/*GtkWidget *p = gtk_widget_get_parent(b);
	gtk_widget_unparent(b);
	gtk_widget_reparent(dlg->control[0]->widget, p);*/
#endif

	border_width = 8;

	HuiResource *res = HuiGetResource(id);
	if (res){
		if (res->type != "SizableDialog")
			return;
		foreachi(HuiResourceCommand &cmd, res->cmd, i){
			//msg_db_m(format("%d:  %d / %d",j,(cmd->type & 1023),(cmd->type >> 10)).c_str(),4);
			//if ((cmd->type & 1023)==HuiCmdDialogAddControl){

			string target_id = cmd.s_param[0];
			int target_page = cmd.i_param[4];
			if (i > 0)
				SetTarget(target_id, target_page);
			int _x = (i == 0) ? x : cmd.i_param[0];
			int _y = (i == 0) ? y : cmd.i_param[1];
			HuiWindowAddControl( this, cmd.type, HuiGetLanguage(cmd.id),
								_x, _y,
								cmd.i_param[2], cmd.i_param[3],
								cmd.id);
			Enable(cmd.id, cmd.enabled);
			if (cmd.image.num > 0)
				SetImage(cmd.id, cmd.image);
		}
	}
}

void HuiWindow::RemoveControl(const string &id)
{
	HuiControl *c = _GetControl_(id);
	/*for (int i=0;i<control.num;i++){
		GtkWidget *p = gtk_widget_get_parent(control[i]->widget);
		if (p == c->widget)
			RemoveControl(control[i]->ID);
	}*/
	for (int i=0;i<control.num;i++)
		if (control[i]->id == id){
			for (int j=i;j<control.num;j++)
				delete(control[j]);
			control.resize(i);
	}

	gtk_widget_destroy(((HuiControlGtk*)c)->widget);
}



//----------------------------------------------------------------------------------
// drawing

static int cur_font_size;
static string cur_font = "Sans";
static bool cur_font_bold, cur_font_italic;

void HuiWindow::Redraw(const string &_id)
{
	HuiControlGtk *c = (HuiControlGtk*)_GetControl_(_id);
	if (c)
		gdk_window_invalidate_rect(gtk_widget_get_window(c->widget), NULL, false);
}

void HuiWindow::RedrawRect(const string &_id, int x, int y, int w, int h)
{
	HuiControlGtk *c = (HuiControlGtk*)_GetControl_(_id);
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

static HuiDrawingContext hui_drawing_context; 

HuiDrawingContext *HuiWindow::BeginDraw(const string &_id)
{
	HuiControlGtk *c = (HuiControlGtk*)_GetControl_(_id);
	hui_drawing_context.win = this;
	hui_drawing_context.id = _id;
	hui_drawing_context.cr = NULL;
	if (c){
		hui_drawing_context.cr = gdk_cairo_create(gtk_widget_get_window(c->widget));
		//gdk_drawable_get_size(gtk_widget_get_window(c->widget), &hui_drawing_context.width, &hui_drawing_context.height);
		hui_drawing_context.width = gdk_window_get_width(gtk_widget_get_window(c->widget));
		hui_drawing_context.height = gdk_window_get_height(gtk_widget_get_window(c->widget));
		//hui_drawing_context.SetFontSize(16);
		hui_drawing_context.SetFont("Sans", 16, false, false);
	}
	return &hui_drawing_context;
}

void HuiDrawingContext::End()
{
	if (!cr)
		return;
	
	cairo_destroy(cr);
}

void HuiDrawingContext::SetColor(const color &c)
{
	if (!cr)
		return;
	cairo_set_source_rgba(cr, c.r, c.g, c.b, c.a);
}

void HuiDrawingContext::SetLineWidth(float w)
{
	if (!cr)
		return;
	cairo_set_line_width(cr, w);
}

color HuiDrawingContext::GetThemeColor(int i)
{
	GtkStyle *style = gtk_widget_get_style(win->window);
	int x = (i / 5);
	int y = (i % 5);
	GdkColor c;
	if (x == 0)
		c = style->fg[y];
	else if (x == 1)
		c = style->bg[y];
	else if (x == 2)
		c = style->light[y];
	else if (x == 3)
		c = style->mid[y];
	else if (x == 4)
		c = style->dark[y];
	else if (x == 5)
		c = style->base[y];
	else if (x == 6)
		c = style->text[y];
	return color(1, (float)c.red / 65535.0f, (float)c.green / 65535.0f, (float)c.blue / 65535.0f);
}


void HuiDrawingContext::DrawPoint(float x, float y)
{
	if (!cr)
		return;
}

void HuiDrawingContext::DrawLine(float x1, float y1, float x2, float y2)
{
	if (!cr)
		return;
	cairo_move_to(cr, x1 + 0.5f, y1 + 0.5f);
	cairo_line_to(cr, x2 + 0.5f, y2 + 0.5f);
	cairo_stroke(cr);
}

void HuiDrawingContext::DrawLines(float *x, float *y, int num_lines)
{
	if (!cr)
		return;
	//cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
	//cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
	cairo_move_to(cr, x[0], y[0]);
	for (int i=1;i<=num_lines;i++)  // <=  !!!
		cairo_line_to(cr, x[i], y[i]);
	cairo_stroke(cr);
}

void HuiDrawingContext::DrawLinesMA(Array<float> &x, Array<float> &y)
{
	DrawLines(&x[0], &y[0], x.num - 1);
}

void HuiDrawingContext::DrawPolygon(float *x, float *y, int num_points)
{
	if (!cr)
		return;
	cairo_move_to(cr, x[0], y[0]);
	for (int i=1;i<num_points;i++)
		cairo_line_to(cr, x[i], y[i]);
	cairo_close_path(cr);
	cairo_fill(cr);
}

void HuiDrawingContext::DrawPolygonMA(Array<float> &x, Array<float> &y)
{
	DrawPolygon(&x[0], &y[0], x.num);
}

void HuiDrawingContext::DrawStr(float x, float y, const string &str)
{
	if (!cr)
		return;
	cairo_move_to(cr, x, y);// + cur_font_size);
	PangoLayout *layout = pango_cairo_create_layout(cr);
	pango_layout_set_text(layout, (char*)str.data, str.num);//.c_str(), -1);
	string f = cur_font;
	if (cur_font_bold)
		f += " Bold";
	if (cur_font_italic)
		f += " Italic";
	f += " " + i2s(cur_font_size);
	PangoFontDescription *desc = pango_font_description_from_string(f.c_str());
	//PangoFontDescription *desc = pango_font_description_new();
	//pango_font_description_set_family(desc, "Sans");//cur_font);
	//pango_font_description_set_size(desc, 10);//cur_font_size);
	pango_layout_set_font_description(layout, desc);
	pango_font_description_free(desc);
	pango_cairo_show_layout(cr, layout);
	g_object_unref(layout);
	
	//cairo_show_text(cr, str);
}

float HuiDrawingContext::GetStrWidth(const string &str)
{
	if (!cr)
		return 0;
	PangoLayout *layout = pango_cairo_create_layout(cr);
	pango_layout_set_text(layout, (char*)str.data, str.num);//.c_str(), -1);
	string f = cur_font;
	if (cur_font_bold)
		f += " Bold";
	if (cur_font_italic)
		f += " Italic";
	f += " " + i2s(cur_font_size);
	PangoFontDescription *desc = pango_font_description_from_string(f.c_str());
	//PangoFontDescription *desc = pango_font_description_new();
	//pango_font_description_set_family(desc, "Sans");//cur_font);
	//pango_font_description_set_size(desc, 10);//cur_font_size);
	pango_layout_set_font_description(layout, desc);
	pango_font_description_free(desc);
	int w, h;
	pango_layout_get_size(layout, &w, &h);
	g_object_unref(layout);

	return (float)w / 1000.0f;
}

void HuiDrawingContext::DrawRect(float x, float y, float w, float h)
{
	if (!cr)
		return;
	cairo_rectangle(cr, x, y, w, h);
	cairo_fill(cr);
}

void HuiDrawingContext::DrawRect(const rect &r)
{
	if (!cr)
		return;
	cairo_rectangle(cr, r.x1, r.y1, r.width(), r.height());
	cairo_fill(cr);
}

void HuiDrawingContext::DrawCircle(float x, float y, float radius)
{
	if (!cr)
		return;
	cairo_arc(cr, x, y, radius, 0, 2 * pi);
	cairo_fill(cr);
}

void HuiDrawingContext::DrawImage(float x, float y, const Image &image)
{
#ifdef _X_USE_IMAGE_
	if (!cr)
		return;
	image.SetMode(Image::ModeBGRA);
	cairo_pattern_t *p = cairo_get_source(cr);
	cairo_pattern_reference(p);
	cairo_surface_t *img = cairo_image_surface_create_for_data((unsigned char*)image.data.data,
                                                         CAIRO_FORMAT_ARGB32,
                                                         image.width,
                                                         image.height,
	    image.width * 4);

	cairo_set_source_surface (cr, img, x, y);
	cairo_paint(cr);
	cairo_surface_destroy(img);
	cairo_set_source(cr, p);
	cairo_pattern_destroy(p);
#endif
}

void HuiDrawingContext::SetFont(const string &font, float size, bool bold, bool italic)
{
	if (!cr)
		return;
	//cairo_select_font_face(cr, "serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
	//cairo_set_font_size(cr, size);
	if (font.num > 0)
		cur_font = font;
	if (size > 0)
		cur_font_size = size;
	cur_font_bold = bold;
	cur_font_italic = italic;
}

void HuiDrawingContext::SetFontSize(float size)
{
	if (!cr)
		return;
	//cairo_set_font_size(cr, size);
	cur_font_size = size;
}

void HuiDrawingContext::SetAntialiasing(bool enabled)
{
	if (!cr)
		return;
	if (enabled)
		cairo_set_antialias(cr, CAIRO_ANTIALIAS_DEFAULT);
	else
		cairo_set_antialias(cr, CAIRO_ANTIALIAS_NONE);
}




//----------------------------------------------------------------------------------
// data exchanging functions for control items



// replace all the text
//    for all
void HuiWindow::SetString(const string &_id, const string &str)
{
	if (id == _id)
		SetTitle(str);
	HuiControl *c = _GetControl_(_id);
	if (!c)
		return;
	allow_signal_level++;
	c->SetString(str);
	allow_signal_level--;
}

// replace all the text with a numerical value (int)
//    for all
// select an item
//    for ComboBox, TabControl, ListView?
void HuiWindow::SetInt(const string &_id, int n)
{
	HuiControl *c = _GetControl_(_id);
	if (!c)
		return;
	allow_signal_level++;
	c->SetInt(n);
	allow_signal_level--;
}

// replace all the text with a float
//    for all
void HuiWindow::SetFloat(const string &_id, float f)
{
	HuiControl *c = _GetControl_(_id);
	if (!c)
		return;
	allow_signal_level ++;
	c->SetFloat(f);
	allow_signal_level --;
}

void HuiWindow::SetImage(const string &_id, const string &image)
{
	HuiControl *c = _GetControl_(_id);
	if (!c)
		return;
	c->SetImage(image);
}

void HuiWindow::SetTooltip(const string &_id, const string &tip)
{
	HuiControl *c = _GetControl_(_id);
	if (c)
		c->SetTooltip(tip);
}

/*void set_list_row(HuiControl *c)
{
	GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(c->widget)));
	gtk_list_store_append(store, &iter);
	for (int j=0;j<PartString.num;j++)
		set_list_cell(store, iter, j, PartString[j]);
}*/

// add a single line/string
//    for ComboBox, ListView, ListViewTree, ListViewIcons
void HuiWindow::AddString(const string &_id, const string &str)
{
	HuiControl *c = _GetControl_(_id);
	if (!c)
		return;
	allow_signal_level++;
	c->AddString(str);
	allow_signal_level--;
}

// add a single line as a child in the tree of a ListViewTree
//    for ListViewTree
void HuiWindow::AddChildString(const string &_id, int parent_row, const string &str)
{
	HuiControl *c = _GetControl_(_id);
	if (!c)
		return;
	allow_signal_level++;
	c->AddChildString(parent_row, str);
	allow_signal_level--;
}

// change a single line in the tree of a ListViewTree
//    for ListViewTree
void HuiWindow::ChangeString(const string &_id,int row,const string &str)
{
	HuiControl *c = _GetControl_(_id);
	if (!c)
		return;
	allow_signal_level++;
	c->ChangeString(row, str);
	allow_signal_level--;
}

// listview / treeview
string HuiWindow::GetCell(const string &_id, int row, int column)
{
	HuiControl *c = _GetControl_(_id);
	if (!c)
		return "";
	return c->GetCell(row, column);
}

// listview / treeview
void HuiWindow::SetCell(const string &_id, int row, int column, const string &str)
{
	HuiControl *c = _GetControl_(_id);
	if (!c)
		return;
	allow_signal_level++;
	c->SetCell(row, column, str);
	allow_signal_level--;				
}

void HuiWindow::SetColor(const string &_id, const color &col)
{
	HuiControl *c = _GetControl_(_id);
	if (!c)
		return;
	allow_signal_level++;
	c->SetColor(col);
	allow_signal_level--;
}

// retrieve the text
//    for edit
string HuiWindow::GetString(const string &_id)
{
	HuiControl *c = _GetControl_(_id);
	if (!c)
		return "";
	return c->GetString();
}

// retrieve the text as a numerical value (int)
//    for edit
// which item/line is selected?
//    for ComboBox, TabControl, ListView
int HuiWindow::GetInt(const string &_id)
{
	HuiControl *c = _GetControl_(_id);
	if (!c)
		return -1;
	return c->GetInt();
}

// retrieve the text as a numerical value (float)
//    for edit
float HuiWindow::GetFloat(const string &_id)
{
	HuiControl *c = _GetControl_(_id);
	if (!c)
		return 0;
	return c->GetFloat();
}

color HuiWindow::GetColor(const string &_id)
{
	HuiControl *c = _GetControl_(_id);
	if (!c)
		return Black;
	return c->GetColor();
}

// switch control to usable/unusable
//    for all
void HuiWindow::Enable(const string &_id,bool enabled)
{
	_ToolbarEnable_(_id, enabled);
	if (menu)
		menu->EnableItem(_id, enabled);
	
	HuiControl *c = _GetControl_(_id);
	if (!c)
		return;
	c->Enable(enabled);
}

// show/hide control
//    for all
void HuiWindow::HideControl(const string &_id,bool hide)
{
	HuiControl *c = _GetControl_(_id);
	if (!c)
		return;
	c->Hide(hide);
}

// mark as "checked"
//    for CheckBox, ToolBarItemCheckable
void HuiWindow::Check(const string &_id,bool checked)
{
	_ToolbarCheck_(_id, checked);
	HuiControl *c = _GetControl_(_id);
	if (menu)
		menu->CheckItem(_id, checked);
	if (!c)
		return;
	allow_signal_level++;
	c->Check(checked);
	allow_signal_level--;
}

// is marked as "checked"?
//    for CheckBox
bool HuiWindow::IsChecked(const string &_id)
{
	HuiControl *c = _GetControl_(_id);
	if (!c)
		return _ToolbarIsChecked_(_id);
	return c->IsChecked();
}

// which lines are selected?
//    for ListView
Array<int> HuiWindow::GetMultiSelection(const string &_id)
{
	Array<int> sel;
	HuiControl *c = _GetControl_(_id);
	if (!c)
		return sel;
	return c->GetMultiSelection();
}

void HuiWindow::SetMultiSelection(const string &_id, Array<int> &sel)
{
	HuiControl *c = _GetControl_(_id);
	if (!c)
		return;
	allow_signal_level++;
	c->SetMultiSelection(sel);
	allow_signal_level--;
}

// delete all the content
//    for ComboBox, ListView
void HuiWindow::Reset(const string &_id)
{
	HuiControl *c = _GetControl_(_id);
	if (!c)
		return;
	allow_signal_level++;
	c->Reset();
	allow_signal_level--;
}

void HuiWindow::CompletionAdd(const string &_id, const string &text)
{
	HuiControl *c = _GetControl_(_id);
	if (!c)
		return;
	c->CompletionAdd(text);
}

void HuiWindow::CompletionClear(const string &_id)
{
	HuiControl *c = _GetControl_(_id);
	if (!c)
		return;
	c->CompletionClear();
}

// expand a single row
//    for TreeView
void HuiWindow::Expand(const string &_id, int row, bool expand)
{
	HuiControl *c = _GetControl_(_id);
	if (!c)
		return;
	c->Expand(row, expand);
}

// expand all rows
//    for TreeView
void HuiWindow::ExpandAll(const string &_id, bool expand)
{
	HuiControl *c = _GetControl_(_id);
	if (c)
		c->ExpandAll(expand);
}

// is column in tree expanded?
//    for TreeView
bool HuiWindow::IsExpanded(const string &_id, int row)
{
	HuiControl *c = _GetControl_(_id);
	if (c)
		return c->IsExpanded(row);
	return false;
}

#endif
