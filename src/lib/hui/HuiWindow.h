/*----------------------------------------------------------------------------*\
| Hui window                                                                   |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last update: 2009.12.05 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#ifndef _HUI_WINDOW_EXISTS_
#define _HUI_WINDOW_EXISTS_

#include "hui_input.h"
#include "HuiPanel.h"

class HuiMenu;
class HuiEvent;
class HuiControl;
class HuiWindow;
class HuiPainter;
class HuiToolbar;
class rect;
class HuiResourceNew;


struct HuiCompleteWindowMessage
{
	#ifdef HUI_API_WIN
		unsigned int msg,wparam,lparam;
	#endif
};


// user input
struct HuiInputData
{
	// mouse
	float x, y, dx, dy, dz;	// position, change
	bool lb,mb,rb;				// buttons
	int row, column;
	// keyboard
	bool key[256];
	int key_code;
	int KeyBufferDepth;
	int KeyBuffer[HUI_MAX_KEYBUFFER_DEPTH];
	void reset()
	{	memset(this, 0, sizeof(HuiInputData));	}
};

class HuiToolbar;
class HuiControl;
class HuiControlTabControl;
class HuiControlListView;
class HuiControlTreeView;
class HuiControlGrid;
class HuiControlRadioButton;
class HuiControlGroup;
class HuiControlExpander;

class HuiWindow : public HuiPanel
{
	friend class HuiToolbar;
	friend class HuiControl;
	friend class HuiControlTabControl;
	friend class HuiControlListView;
	friend class HuiControlTreeView;
	friend class HuiControlGrid;
	friend class HuiControlRadioButton;
	friend class HuiControlGroup;
	friend class HuiControlExpander;
	friend class HuiMenu;
public:
	HuiWindow();
	HuiWindow(const string &title, int x, int y, int width, int height, HuiWindow *parent, bool allow_parent, int mode);
	HuiWindow(const string &title, int x, int y, int width, int height);
	HuiWindow(const string &id, HuiWindow *parent, bool allow_parent);
	void _cdecl __init_ext__(const string &title, int x, int y, int width, int height);
	virtual ~HuiWindow();
	virtual void _cdecl __delete__();

	void _Init_(const string &title, int x, int y, int width, int height, HuiWindow *parent, bool allow_parent, int mode);
	void _InitGeneric_(HuiWindow *parent, bool allow_parent, int mode);
	void _CleanUp_();

	// the window
	string _cdecl Run();
	void _cdecl Show();
	void _cdecl Hide();
	void _cdecl SetMaximized(bool maximized);
	bool _cdecl IsMaximized();
	bool _cdecl IsMinimized();
	void _cdecl SetID(const string &id);
	void _cdecl SetFullscreen(bool fullscreen);
	void _cdecl SetTitle(const string &title);
	void _cdecl SetPosition(int x, int y);
	void _cdecl SetPositionSpecial(HuiWindow *win, int mode);
	void _cdecl GetPosition(int &x, int &y);
	void _cdecl SetSize(int width, int height);
	void _cdecl GetSize(int &width, int &height);
	void _cdecl SetSizeDesired(int width, int height);
	void _cdecl GetSizeDesired(int &width, int &height);
	void _cdecl SetMenu(HuiMenu *menu);
	HuiMenu* _cdecl GetMenu();
	void _cdecl SetBorderWidth(int width);
	void _cdecl SetIndent(int indent);
	HuiWindow* _cdecl GetParent();


	void _cdecl SetCursorPos(int x,int y);
	void _cdecl ShowCursor(bool show);

	// status bar
	void _cdecl EnableStatusbar(bool enabled);
	//bool _cdecl IsStatusbarEnabled();
	void _cdecl SetStatusText(const string &str);

	// events by overwriting
	virtual void _cdecl OnMouseMove(){}
	virtual void _cdecl OnLeftButtonDown(){}
	virtual void _cdecl OnMiddleButtonDown(){}
	virtual void _cdecl OnRightButtonDown(){}
	virtual void _cdecl OnLeftButtonUp(){}
	virtual void _cdecl OnMiddleButtonUp(){}
	virtual void _cdecl OnRightButtonUp(){}
	virtual void _cdecl OnDoubleClick(){}
	virtual void _cdecl OnMouseWheel(){}
	virtual void _cdecl OnCloseRequest();
	virtual void _cdecl OnKeyDown(){}
	virtual void _cdecl OnKeyUp(){}
	virtual void _cdecl OnDraw(){}

	// input
	bool _cdecl GetKey(int key);
	bool _cdecl GetMouse(int &x, int &y, int button);


	// hui internal
	int _GetMainLevel_();
	int _GetUniqueID_();
	bool allow_input;
	HuiInputData input;
	int mouse_offset_x, mouse_offset_y;
	HuiControl *main_input_control;

	HuiToolbar *toolbar[4];

private:


#ifdef OS_WINDOWS
public:
	HWND hWnd;
private:
#endif
#ifdef HUI_API_WIN
	bool ready;
	//hui_callback *NixGetInputFromWindow;
	HWND statusbar, gl_hwnd;
	RECT WindowBounds,WindowClient;
	DWORD WindowStyle;
	int cdx,cdy;
#endif
#ifdef HUI_API_GTK
public:
	GtkWidget *window;
private:
	GtkWidget *vbox, *hbox, *menubar, *statusbar, *__ttt__;
	Array<GtkWidget*> gtk_menu;
	int gtk_num_menus;
	int desired_width, desired_height;
#endif
	
	HuiMenu *menu, *popup;
	bool statusbar_enabled;
	bool allowed, allow_keys;
	HuiWindow *parent;

	int unique_id;
	int main_level;
};


class HuiNixWindow : public HuiWindow
{
public:
	HuiNixWindow(const string &title, int x, int y, int width, int height);
	void _cdecl __init_ext__(const string &title, int x, int y, int width, int height);
};

class HuiDialog : public HuiWindow
{
public:
	HuiDialog(const string &title, int width, int height, HuiWindow *root, bool allow_root);
	void _cdecl __init_ext__(const string &title, int width, int height, HuiWindow *root, bool allow_root);
};

class HuiFixedDialog : public HuiWindow
{
public:
	HuiFixedDialog(const string &title, int width, int height, HuiWindow *root, bool allow_root);
	void _cdecl __init_ext__(const string &title, int width, int height, HuiWindow *root, bool allow_root);
};

extern HuiWindow *HuiCurWindow;


class HuiSourceDialog : public HuiWindow
{
public:
	HuiSourceDialog(const string &source, HuiWindow *root);
	void _cdecl __init_ext__(const string &source, HuiWindow *root);
};


void HuiFuncIgnore();
void HuiFuncClose();

enum{
	HuiWinModeResizable = 1,
	HuiWinModeNoFrame = 2,
	HuiWinModeNoTitle = 4,
	HuiWinModeControls = 8,
	HuiWinModeDummy = 16,
};

#define HuiLeft		1
#define HuiRight	2
#define HuiTop		4
#define HuiBottom	8



// which one of the toolbars?
enum{
	HuiToolbarTop,
	HuiToolbarBottom,
	HuiToolbarLeft,
	HuiToolbarRight
};


#endif
