/*----------------------------------------------------------------------------*\
| Hui input                                                                    |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last update: 2009.12.05 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#ifndef _HUI_INPUT_EXISTS_
#define _HUI_INPUT_EXISTS_

namespace hui
{

class EventHandler;

struct Command
{
	string id, image;
	int type, key_code;
	bool enabled;
	Callback func;
};

extern Array<Command> _HuiCommand_;


class Event
{
	public:
	Window *win;
	string message, id;
	bool is_default;
	float mx, my;
	float dx, dy;
	float scroll_x, scroll_y;
	int key, key_code;
	string text;
	int width, height;
	bool lbut, mbut, rbut;
	int row, column, row_target;
	Event(){}
	Event(const string &id, const string &message);
};

extern Event _HuiEvent_;
Event *GetEvent();


class EventListener
{
public:
	string id, message;
	Callback function;
	CallbackP function_p;
	EventListener(){}
	EventListener(const string &id, const string &message, const Callback &function);
	EventListener(const string &id, const string &message, int __, const CallbackP &function);
};


// internal
void _HuiInitInput_();
bool _HuiEventMatch_(Event *e, const string &id, const string &message);
void _HuiSendGlobalCommand_(Event *e);

// key codes and id table ("shortcuts")
void HuiAddKeyCode(const string &id, int key_code);
void HuiAddCommand(const string &id, const string &image, int default_key_code, const Callback &func);
void HuiAddCommandToggle(const string &id, const string &image, int default_key_code, const Callback &func);
/*void _HuiAddCommandM(const string &id, const string &image, int default_key_code, HuiEventHandler *object, void (HuiEventHandler::*function)());
void _HuiAddCommandMToggle(const string &id, const string &image, int default_key_code, HuiEventHandler *object, void (HuiEventHandler::*function)());
template<typename T>
void HuiAddCommandM(const string &id, const string &image, int default_key_code, void *object, T fun)
{	_HuiAddCommandM(id, image, default_key_code, (HuiEventHandler*)object, (void(HuiEventHandler::*)())fun);	}
template<typename T>
void HuiAddCommandMToggle(const string &id, const string &image, int default_key_code, void *object, T fun)
{	_HuiAddCommandMToggle(id, image, default_key_code, (HuiEventHandler*)object, (void(HuiEventHandler::*)())fun);	}*/

void HuiLoadKeyCodes(const string &filename);
void HuiSaveKeyCodes(const string &filename);

// input
string _cdecl HuiGetKeyName(int key_code);
string _cdecl HuiGetKeyCodeName(int key_code);
string _cdecl HuiGetKeyChar(int key_code);


#ifdef HUI_API_GTK
extern GdkEvent *HuiGdkEvent;
#endif

};

#endif
