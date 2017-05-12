/*----------------------------------------------------------------------------*\
| Hui window                                                                   |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last update: 2010.07.14 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#include "Controls/Control.h"
#include "hui.h"
#include "hui_internal.h"
#include "Toolbar.h"


namespace hui
{

extern int HuiMainLevel;

Window *HuiCurWindow = NULL;

// recursively find a menu item and execute message_function
/*bool TestMenuID(CHuiMenu *menu, const string &id, message_function *mf)
{
	if (!menu)
		return false;
	for (int i=0;i<menu->Item.num;i++){
		if (menu->Item[i].ID==id){
			mf(id);
			return true;
		}
		if (menu->Item[i].SubMenu)
			if (TestMenuID(menu->Item[i].SubMenu,id,mf))
				return true;
	}
	return false;
}*/


void HuiInputData::reset()
{
	x = y = dx = dy = scroll_x = scroll_y = 0;
	lb = mb = rb = false;
	memset(key, 0, sizeof(key));
	key_buffer.clear();
}


Window::Window(const string &title, int x, int y, int width, int height, Window *root, bool allow_root, int mode)
{
	_init_(title, x, y, width, height, root, allow_root, mode);
}

Window::Window()
{
	_init_("", -1, -1, 0, 0, NULL, true, HUI_WIN_MODE_DUMMY);
}

Window::Window(const string &title, int x, int y, int width, int height)
{
	_init_(title, x, y, width, height, NULL, true, HUI_WIN_MODE_RESIZABLE | HUI_WIN_MODE_CONTROLS);
}

void Window::__init_ext__(const string& title, int x, int y, int width, int height)
{
	new(this) Window(title, x, y, width, height);
}


Window::Window(const string &id, Window *parent)
{
	HuiResource *res = HuiGetResource(id);
	if (!res){
		msg_error("HuiWindow: undefined resource id: " + id);
	}

	int mode = HUI_WIN_MODE_CONTROLS;
	if (res->type == "SizableDialog")
		mode = HUI_WIN_MODE_CONTROLS | HUI_WIN_MODE_RESIZABLE;
	bool allow_parent = false;
	for (string &o: res->options)
		if ((o == "allow-root") or (o == "allow-parent"))
			allow_parent = true;
	_init_(HuiGetLanguage(id, id), -1, -1, res->w, res->h, parent, allow_parent, mode);

	// menu/toolbar?
	for (string &o: res->options){
		if (o.head(5) == "menu=")
			setMenu(HuiCreateResourceMenu(o.substr(5, -1)));
		if (o.head(8) == "toolbar=")
			toolbar[HUI_TOOLBAR_TOP]->setByID(o.substr(8, -1));
	}

	// controls
	for (HuiResource &cmd: res->children)
		_addControl(id, cmd, "");

	msg_db_m("  \\(^_^)/",1);
}

void Window::_init_generic_(Window *_root, bool _allow_root, int _mode)
{
	_HuiMakeUsable_();
	HuiWindows.add(this);

	is_resizable = ((_mode & HUI_WIN_MODE_RESIZABLE) > 0);
	allowed = true;
	allow_keys = true;
	parent = _root;
	main_input_control = NULL;
	if (parent){
		parent->allowed = _allow_root;
	}
	menu = popup = NULL;
	statusbar_enabled = false;
	toolbar[HUI_TOOLBAR_TOP] = new Toolbar(this);
	toolbar[HUI_TOOLBAR_LEFT] = new Toolbar(this, true);
	toolbar[HUI_TOOLBAR_RIGHT] = new Toolbar(this, true);
	toolbar[HUI_TOOLBAR_BOTTOM] = new Toolbar(this);
	input.reset();

	allow_input = false; // allow only if ->Show() was called
	main_level = HuiMainLevel;
}

void Window::_clean_up_()
{
	for (int i=0; i<4; i++)
		delete(toolbar[i]);

	_ClearPanel_();
	input.reset();
	
	// unregister window
	for (int i=0;i<HuiWindows.num;i++)
		if (HuiWindows[i] == this){
			HuiWindows.erase(i);
			break;
		}
}

// default handler when trying to close the windows
void Window::onCloseRequest()
{
	int level = _get_main_level_();
	destroy();
	
	// no message function (and last window in this main level): end program
	// ...or at least end nested main level
	for (Window *w: HuiWindows)
		if (w->_get_main_level_() >= level)
			return;
	HuiEnd();
}


// identify window (for automatic title assignment with language strings)
void Window::setID(const string &_id)
{
	id = _id;
	if (HuiLanguaged and (id.num > 0))
		setTitle(HuiGetLanguage(id, id));
}

// align window relative to another window (like..."top right corner")
void Window::setPositionSpecial(Window *win,int mode)
{
	int pw, ph, cw, ch, px, py, cx, cy;
	win->getSize(pw, ph);
	win->getPosition(px, py);
	getSize(cw, ch);
	getPosition(cx, cy);
	if ((mode & HUI_LEFT) > 0)
		cx = px + 2;
	if ((mode & HUI_RIGHT) > 0)
		cx = px + pw - cw - 2;
	if ((mode & HUI_TOP) > 0)
		cy = py + 20;
	if ((mode & HUI_BOTTOM) > 0)
		cy = py + ph - ch - 2;
	setPosition(cx, cy);
}

int Window::_get_main_level_()
{
	return main_level;
}

Menu *Window::getMenu()
{
	return menu;
}

Window *Window::getParent()
{
	return parent;
}

bool Window::getKey(int k)
{
	if (k == KEY_CONTROL)
		return (input.key[KEY_RCONTROL] or input.key[KEY_LCONTROL]);
	else if (k == KEY_SHIFT)
		return (input.key[KEY_RSHIFT] or input.key[KEY_LSHIFT]);
	else
		return input.key[k];
}

bool Window::getMouse(int &x, int &y, int button)
{
	x = (int)input.x;
	y = (int)input.y;
	if (button == 0){
		return input.lb;
	}else if (button == 1){
		return input.mb;
	}else{
		return input.rb;
	}
}





Window *HuiCreateDialog(const string &title,int width,int height,Window *root,bool allow_root)
{
	return new HuiFixedDialog(title, width, height, root, allow_root);
}

Window *HuiCreateSizableDialog(const string &title,int width,int height,Window *root,bool allow_root)
{
	return new HuiDialog(title, width, height, root, allow_root);
}

void HuiFuncIgnore()
{
}

void HuiFuncClose()
{
	GetEvent()->win->destroy();
}

HuiNixWindow::HuiNixWindow(const string& title, int x, int y, int width, int height) :
	Window(title, x, y, width, height, NULL, true, HUI_WIN_MODE_RESIZABLE)
{
	addDrawingArea("", 0, 0, 0, 0, "nix-area");
}

void HuiNixWindow::__init_ext__(const string& title, int x, int y, int width, int height)
{
	new(this) HuiNixWindow(title, x, y, width, height);
}

HuiDialog::HuiDialog(const string& title, int width, int height, Window* root, bool allow_root) :
	Window(title, -1, -1, width, height, root, allow_root, HUI_WIN_MODE_CONTROLS | HUI_WIN_MODE_RESIZABLE)
{
}

void HuiDialog::__init_ext__(const string& title, int width, int height, Window* root, bool allow_root)
{
	new(this) HuiDialog(title, width, height, root, allow_root);
}

HuiFixedDialog::HuiFixedDialog(const string& title, int width, int height, Window* root, bool allow_root) :
	Window(title, -1, -1, width, height, root, allow_root, HUI_WIN_MODE_CONTROLS)
{
}

void HuiFixedDialog::__init_ext__(const string& title, int width, int height, Window* root, bool allow_root)
{
	new(this) HuiFixedDialog(title, width, height, root, allow_root);
}



HuiSourceDialog::HuiSourceDialog(const string &buffer, Window *root) :
	Window("", -1, -1, 300, 200, root, buffer.find("allow-parent") > 0, HUI_WIN_MODE_CONTROLS | HUI_WIN_MODE_RESIZABLE)
{
	fromSource(buffer);
}

};
