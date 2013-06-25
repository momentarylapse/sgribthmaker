/*----------------------------------------------------------------------------*\
| CHui                                                                         |
| -> Heroic User Interface                                                     |
| -> abstraction layer for GUI                                                 |
|   -> Windows (default api) or Linux (Gtk+)                                   |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last updated: 2009.12.05 (c) by MichiSoft TM                                 |
\*----------------------------------------------------------------------------*/

#ifndef _HUI_EXISTS_
#define _HUI_EXISTS_

#include "hui_common.h"
#ifdef _X_USE_IMAGE_
	#include "../image/image.h"
#else
	typedef int Image;
	typedef int color;
#endif

extern string HuiVersion;




class HuiWindow;
class HuiMenu;
class HuiEventHandler;







//----------------------------------------------------------------------------------
// hui itself


// execution
void HuiInit(const string &program, bool load_res, const string &def_lang);
void _HuiMakeUsable_();
int HuiRun();
void HuiPushMainLevel();
void HuiPopMainLevel();
void HuiSetIdleFunction(hui_callback *idle_function);
void _HuiSetIdleFunctionM(HuiEventHandler *object, void (HuiEventHandler::*function)());
template<typename T>
void HuiSetIdleFunctionM(HuiEventHandler *object, T fun)
{	_HuiSetIdleFunctionM(object, (void(HuiEventHandler::*)())fun);	}
void HuiRunLater(float time, hui_callback *function);
void _HuiRunLaterM(float time, HuiEventHandler *object, void (HuiEventHandler::*function)());
template<typename T>
void HuiRunLaterM(float time, HuiEventHandler *object, T fun)
{	_HuiRunLaterM(time, object, (void(HuiEventHandler::*)())fun);	}
void HuiDoSingleMainLoop();
void HuiEnd();
extern bool HuiEndKeepMsgAlive;

// images
//int HuiLoadImage(const string &filename);
string HuiSetImage(const Image &image);

// HUI configuration
extern string HuiComboBoxSeparator;
extern bool HuiCreateHiddenWindows;

// data from hui (...don't change...)
extern string HuiAppFilename, HuiAppDirectory, HuiAppDirectoryStatic;
extern string HuiInitialWorkingDirectory;
extern bool HuiRunning;



#include "hui_menu.h"
#include "hui_window.h"
#include "hui_common_dlg.h"
#include "hui_language.h"
#include "hui_config.h"
#include "hui_input.h"
#include "hui_resource.h"
#include "hui_utility.h"


#endif


