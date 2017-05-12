#include "../../file/file.h"
#include "../kaba.h"
#include "../../config.h"
#include "common.h"

#ifdef _X_USE_HUI_
	#include "../../hui/hui.h"
#else
	we are re screwed.... TODO: test for _X_USE_HUI_
#endif

namespace Kaba{

#ifdef _X_USE_HUI_
	static hui::HuiEvent *_event;
	static hui::HuiPainter *_painter;
	#define GetDAWindow(x)			long(&_win->x)-long(_win)
	#define GetDAEvent(x)	long(&_event->x)-long(_event)
	#define GetDAPainter(x)	long(&_painter->x)-long(_painter)
#else
	#define GetDAWindow(x)		0
	#define GetDAEvent(x)	0
	#define GetDAPainter(x)	0
#endif

#ifdef _X_USE_HUI_
	#define hui_p(p)		(void*)p
#else
	#define hui_p(p)		NULL
#endif


extern Class *TypeIntList;
extern Class *TypeIntPs;
extern Class *TypeComplexList;
extern Class *TypeImage;
Class *TypeHuiWindowP;

void SIAddPackageHui()
{
	add_package("hui", false);
	
	Class*
	TypeHuiMenu		= add_type  ("Menu",  sizeof(hui::HuiMenu));
	Class*
	TypeHuiMenuP	= add_type_p("Menu*", TypeHuiMenu);
	Class*
	TypeHuiPanel	= add_type  ("Panel", sizeof(hui::HuiPanel));
	Class*
	TypeHuiWindow	= add_type  ("Window", sizeof(hui::HuiWindow));
	TypeHuiWindowP	= add_type_p("Window*",	TypeHuiWindow);
	Class*
	TypeHuiNixWindow= add_type  ("NixWindow", sizeof(hui::HuiWindow));
	Class*
	TypeHuiDialog	= add_type  ("Dialog", sizeof(hui::HuiWindow));
	Class*
	TypeHuiFixedDialog= add_type  ("FixedDialog", sizeof(hui::HuiWindow));
	Class*
	TypeHuiEvent	= add_type  ("Event", sizeof(hui::HuiEvent));
	Class*
	TypeHuiEventP	= add_type_p("Event*", TypeHuiEvent);
	Class*
	TypeHuiPainter	= add_type  ("Painter", sizeof(hui::HuiPainter));
	Class*
	TypeHuiPainterP	= add_type_p("Painter*", TypeHuiPainter);
	Class*
	TypeHuiTimer	= add_type  ("Timer", sizeof(hui::HuiTimer));
	Class*
	TypeHuiConfiguration = add_type  ("Configuration", sizeof(hui::HuiConfiguration));

	
	add_func("GetKeyName",									TypeString,	hui_p(&hui::HuiGetKeyName));
		func_add_param("id",		TypeInt);

	add_class(TypeHuiMenu);
		class_add_func(IDENTIFIER_FUNC_INIT,		TypeVoid,		mf(&hui::HuiMenu::__init__));
		class_add_func("popup",	TypeVoid,		mf(&hui::HuiMenu::openPopup));
			func_add_param("w",			TypeHuiWindowP);
			func_add_param("x",			TypeInt);
			func_add_param("y",			TypeInt);
		class_add_func("add",		TypeVoid,		mf(&hui::HuiMenu::addItem));
			func_add_param("name",		TypeString);
			func_add_param("id",		TypeInt);
		class_add_func("addWithImage",	TypeVoid,		mf(&hui::HuiMenu::addItemImage));
			func_add_param("name",		TypeString);
			func_add_param("image",		TypeInt);
			func_add_param("id",		TypeInt);
		class_add_func("addCheckable",	TypeVoid,		mf(&hui::HuiMenu::addItemCheckable));
			func_add_param("name",		TypeString);
			func_add_param("id",		TypeInt);
		class_add_func("addSeparator",	TypeVoid,		mf(&hui::HuiMenu::addSeparator));
		class_add_func("addSubMenu",	TypeVoid,		mf(&hui::HuiMenu::addSubMenu));
			func_add_param("name",		TypeString);
			func_add_param("id",		TypeInt);
			func_add_param("sub_menu",	TypeHuiMenuP);

		add_class(TypeHuiPanel);
			class_add_func(IDENTIFIER_FUNC_INIT,		TypeVoid,		mf(&hui::HuiPanel::__init__));
			class_add_func_virtual(IDENTIFIER_FUNC_DELETE,		TypeVoid,		mf(&hui::HuiPanel::__delete__));
			class_add_func("setBorderWidth",			TypeVoid,		mf(&hui::HuiPanel::setBorderWidth));
				func_add_param("width",		TypeInt);
			class_add_func("setDecimals",			TypeVoid,		mf(&hui::HuiPanel::setDecimals));
				func_add_param("decimals",		TypeInt);
			class_add_func("activate",										TypeVoid,		mf(&hui::HuiPanel::activate));
				func_add_param("id",		TypeInt);
			class_add_func("isActive",										TypeVoid,		mf(&hui::HuiPanel::isActive));
				func_add_param("id",	TypeString);
			class_add_func("fromSource",	TypeVoid,		mf(&hui::HuiPanel::fromSource));
				func_add_param("source",		TypeString);
			class_add_func("addButton",										TypeVoid,		mf(&hui::HuiPanel::addButton));
				func_add_param("title",		TypeString);
				func_add_param("x",			TypeInt);
				func_add_param("y",			TypeInt);
				func_add_param("width",		TypeInt);
				func_add_param("height",	TypeInt);
				func_add_param("id",		TypeString);
			class_add_func("addDefButton",										TypeVoid,		mf(&hui::HuiPanel::addDefButton));
				func_add_param("title",		TypeString);
				func_add_param("x",			TypeInt);
				func_add_param("y",			TypeInt);
				func_add_param("width",		TypeInt);
				func_add_param("height",	TypeInt);
				func_add_param("id",		TypeString);
			class_add_func("addCheckBox",								TypeVoid,		mf(&hui::HuiPanel::addCheckBox));
				func_add_param("title",		TypeString);
				func_add_param("x",			TypeInt);
				func_add_param("y",			TypeInt);
				func_add_param("width",		TypeInt);
				func_add_param("height",	TypeInt);
				func_add_param("id",		TypeString);
			class_add_func("addLabel",										TypeVoid,		mf(&hui::HuiPanel::addLabel));
				func_add_param("title",		TypeString);
				func_add_param("x",			TypeInt);
				func_add_param("y",			TypeInt);
				func_add_param("width",		TypeInt);
				func_add_param("height",	TypeInt);
				func_add_param("id",		TypeString);
			class_add_func("addEdit",										TypeVoid,		mf(&hui::HuiPanel::addEdit));
				func_add_param("title",		TypeString);
				func_add_param("x",			TypeInt);
				func_add_param("y",			TypeInt);
				func_add_param("width",		TypeInt);
				func_add_param("height",	TypeInt);
				func_add_param("id",		TypeString);
			class_add_func("addMultilineEdit",										TypeVoid,		mf(&hui::HuiPanel::addMultilineEdit));
				func_add_param("title",		TypeString);
				func_add_param("x",			TypeInt);
				func_add_param("y",			TypeInt);
				func_add_param("width",		TypeInt);
				func_add_param("height",	TypeInt);
				func_add_param("id",		TypeString);
			class_add_func("addGroup",										TypeVoid,		mf(&hui::HuiPanel::addGroup));
				func_add_param("title",		TypeString);
				func_add_param("x",			TypeInt);
				func_add_param("y",			TypeInt);
				func_add_param("width",		TypeInt);
				func_add_param("height",	TypeInt);
				func_add_param("id",		TypeString);
			class_add_func("addComboBox",								TypeVoid,		mf(&hui::HuiPanel::addComboBox));
				func_add_param("title",		TypeString);
				func_add_param("x",			TypeInt);
				func_add_param("y",			TypeInt);
				func_add_param("width",		TypeInt);
				func_add_param("height",	TypeInt);
				func_add_param("id",		TypeString);
			class_add_func("addTabControl",								TypeVoid,		mf(&hui::HuiPanel::addTabControl));
				func_add_param("title",		TypeString);
				func_add_param("x",			TypeInt);
				func_add_param("y",			TypeInt);
				func_add_param("width",		TypeInt);
				func_add_param("height",	TypeInt);
				func_add_param("id",		TypeString);
			class_add_func("setTarget",				TypeVoid,		mf(&hui::HuiPanel::setTarget));
				func_add_param("id",		TypeString);
				func_add_param("page",		TypeInt);
			class_add_func("addListView",								TypeVoid,		mf(&hui::HuiPanel::addListView));
				func_add_param("title",		TypeString);
				func_add_param("x",			TypeInt);
				func_add_param("y",			TypeInt);
				func_add_param("width",		TypeInt);
				func_add_param("height",	TypeInt);
				func_add_param("id",		TypeString);
			class_add_func("addTreeView",								TypeVoid,		mf(&hui::HuiPanel::addTreeView));
				func_add_param("title",		TypeString);
				func_add_param("x",			TypeInt);
				func_add_param("y",			TypeInt);
				func_add_param("width",		TypeInt);
				func_add_param("height",	TypeInt);
				func_add_param("id",		TypeString);
			class_add_func("addIconView",								TypeVoid,		mf(&hui::HuiPanel::addIconView));
				func_add_param("title",		TypeString);
				func_add_param("x",			TypeInt);
				func_add_param("y",			TypeInt);
				func_add_param("width",		TypeInt);
				func_add_param("height",	TypeInt);
				func_add_param("id",		TypeString);
			class_add_func("addProgressBar",						TypeVoid,		mf(&hui::HuiPanel::addProgressBar));
				func_add_param("title",		TypeString);
				func_add_param("x",			TypeInt);
				func_add_param("y",			TypeInt);
				func_add_param("width",		TypeInt);
				func_add_param("height",	TypeInt);
				func_add_param("id",		TypeString);
			class_add_func("addSlider",										TypeVoid,		mf(&hui::HuiPanel::addSlider));
				func_add_param("title",		TypeString);
				func_add_param("x",			TypeInt);
				func_add_param("y",			TypeInt);
				func_add_param("width",		TypeInt);
				func_add_param("height",	TypeInt);
				func_add_param("id",		TypeString);
			class_add_func("addDrawingArea",										TypeVoid,		mf(&hui::HuiPanel::addDrawingArea));
				func_add_param("title",		TypeString);
				func_add_param("x",			TypeInt);
				func_add_param("y",			TypeInt);
				func_add_param("width",		TypeInt);
				func_add_param("height",	TypeInt);
				func_add_param("id",		TypeString);
			class_add_func("addGrid",										TypeVoid,		mf(&hui::HuiPanel::addGrid));
				func_add_param("title",		TypeString);
				func_add_param("x",			TypeInt);
				func_add_param("y",			TypeInt);
				func_add_param("width",		TypeInt);
				func_add_param("height",	TypeInt);
				func_add_param("id",		TypeString);
			class_add_func("addSpinButton",										TypeVoid,		mf(&hui::HuiPanel::addSpinButton));
				func_add_param("title",		TypeString);
				func_add_param("x",			TypeInt);
				func_add_param("y",			TypeInt);
				func_add_param("width",		TypeInt);
				func_add_param("height",	TypeInt);
				func_add_param("id",		TypeString);
			class_add_func("addRadioButton",										TypeVoid,		mf(&hui::HuiPanel::addRadioButton));
				func_add_param("title",		TypeString);
				func_add_param("x",			TypeInt);
				func_add_param("y",			TypeInt);
				func_add_param("width",		TypeInt);
				func_add_param("height",	TypeInt);
				func_add_param("id",		TypeString);
			class_add_func("addScroller",								TypeVoid,		mf(&hui::HuiPanel::addScroller));
				func_add_param("title",		TypeString);
				func_add_param("x",			TypeInt);
				func_add_param("y",			TypeInt);
				func_add_param("width",		TypeInt);
				func_add_param("height",	TypeInt);
				func_add_param("id",		TypeString);
			class_add_func("addExpander",								TypeVoid,		mf(&hui::HuiPanel::addExpander));
				func_add_param("title",		TypeString);
				func_add_param("x",			TypeInt);
				func_add_param("y",			TypeInt);
				func_add_param("width",		TypeInt);
				func_add_param("height",	TypeInt);
				func_add_param("id",		TypeString);
			class_add_func("addSeparator",								TypeVoid,		mf(&hui::HuiPanel::addSeparator));
				func_add_param("title",		TypeString);
				func_add_param("x",			TypeInt);
				func_add_param("y",			TypeInt);
				func_add_param("width",		TypeInt);
				func_add_param("height",	TypeInt);
				func_add_param("id",		TypeString);
			class_add_func("addPaned",								TypeVoid,		mf(&hui::HuiPanel::addPaned));
				func_add_param("title",		TypeString);
				func_add_param("x",			TypeInt);
				func_add_param("y",			TypeInt);
				func_add_param("width",		TypeInt);
				func_add_param("height",	TypeInt);
				func_add_param("id",		TypeString);
			class_add_func("addRevealer",								TypeVoid,		mf(&hui::HuiPanel::addRevealer));
				func_add_param("title",		TypeString);
				func_add_param("x",			TypeInt);
				func_add_param("y",			TypeInt);
				func_add_param("width",		TypeInt);
				func_add_param("height",	TypeInt);
				func_add_param("id",		TypeString);
			class_add_func("setString",						TypeVoid,		mf(&hui::HuiPanel::setString));
				func_add_param("id",		TypeString);
				func_add_param("s",			TypeString);
			class_add_func("getString",						TypeString,		mf(&hui::HuiPanel::getString));
				func_add_param("id",		TypeString);
			class_add_func("setFloat",						TypeVoid,		mf(&hui::HuiPanel::setFloat));
				func_add_param("id",		TypeString);
				func_add_param("f",			TypeFloat32);
			class_add_func("getFloat",						TypeFloat32,		mf(&hui::HuiPanel::getFloat));
				func_add_param("id",		TypeString);
			class_add_func("enable",								TypeVoid,		mf(&hui::HuiPanel::enable));
				func_add_param("id",		TypeString);
				func_add_param("enabled",	TypeBool);
			class_add_func("isEnabled",					TypeBool,		mf(&hui::HuiPanel::isEnabled));
				func_add_param("id",		TypeString);
			class_add_func("check",								TypeVoid,		mf(&hui::HuiPanel::check));
				func_add_param("id",		TypeString);
				func_add_param("checked",	TypeBool);
			class_add_func("isChecked",					TypeBool,		mf(&hui::HuiPanel::isChecked));
				func_add_param("id",		TypeString);
			class_add_func("getInt",			TypeInt,		mf(&hui::HuiPanel::getInt));
				func_add_param("id",		TypeString);
			class_add_func("getSelection",			TypeIntList,		mf(&hui::HuiPanel::getSelection));
				func_add_param("id",		TypeString);
			class_add_func("setInt",			TypeVoid,		mf(&hui::HuiPanel::setInt));
				func_add_param("id",		TypeString);
				func_add_param("i",			TypeInt);
			class_add_func("setImage",			TypeVoid,		mf(&hui::HuiPanel::setImage));
				func_add_param("id",		TypeString);
				func_add_param("image",		TypeString);
			class_add_func("getCell",						TypeString,		mf(&hui::HuiPanel::getCell));
				func_add_param("id",		TypeString);
				func_add_param("row",		TypeInt);
				func_add_param("column",	TypeInt);
			class_add_func("setCell",						TypeVoid,		mf(&hui::HuiPanel::setCell));
				func_add_param("id",		TypeString);
				func_add_param("row",		TypeInt);
				func_add_param("column",	TypeInt);
				func_add_param("s",			TypeString);
			class_add_func("setOptions",			TypeVoid,		mf(&hui::HuiPanel::setOptions));
				func_add_param("id",		TypeString);
				func_add_param("options",	TypeString);
			class_add_func("reset",								TypeVoid,		mf(&hui::HuiPanel::reset));
				func_add_param("id",		TypeString);
			class_add_func("redraw",								TypeVoid,		mf(&hui::HuiPanel::redraw));
				func_add_param("id",		TypeString);
			class_add_func("event",						TypeVoid,		mf(&hui::HuiPanel::_kaba_event));
				func_add_param("id",			TypeString);
				func_add_param("func",			TypePointer);
			class_add_func("eventO",						TypeVoid,		mf(&hui::HuiPanel::_kaba_eventO));
				func_add_param("id",			TypeString);
				func_add_param("handler",		TypePointer);
				func_add_param("func",			TypePointer);
			class_add_func("eventX",						TypeVoid,		mf(&hui::HuiPanel::_kaba_eventX));
				func_add_param("id",			TypeString);
				func_add_param("msg",			TypeString);
				func_add_param("func",			TypePointer);
			class_add_func("eventOX",						TypeVoid,		mf(&hui::HuiPanel::_kaba_eventOX));
				func_add_param("id",			TypeString);
				func_add_param("msg",			TypeString);
				func_add_param("handler",		TypePointer);
				func_add_param("func",			TypePointer);
			//class_add_func("beginDraw",								TypeHuiPainterP,		mf(&hui::HuiPanel::beginDraw));
			//	func_add_param("id",		TypeString);
			class_set_vtable(hui::HuiPanel);


	add_class(TypeHuiWindow);
		TypeHuiWindow->DeriveFrom(TypeHuiPanel, false);
		TypeHuiWindow->vtable = TypeHuiPanel->vtable;
		class_add_func(IDENTIFIER_FUNC_INIT,		TypeVoid,		mf(&hui::HuiWindow::__init_ext__), FLAG_OVERRIDE);
			func_add_param("title",		TypeString);
			func_add_param("x",		TypeInt);
			func_add_param("y",		TypeInt);
			func_add_param("width",		TypeInt);
			func_add_param("height",		TypeInt);
		class_add_func_virtual(IDENTIFIER_FUNC_DELETE,		TypeVoid,		mf(&hui::HuiWindow::__delete__), FLAG_OVERRIDE);
		class_add_func("run",			TypeVoid,		mf(&hui::HuiWindow::run));
		class_add_func("destroy",		TypeVoid,		mf(&hui::HuiWindow::destroy));
		class_add_func("show",			TypeVoid,		mf(&hui::HuiWindow::show));
		class_add_func("hide",			TypeVoid,		mf(&hui::HuiWindow::hide));

		class_add_func("setMenu",			TypeVoid,		mf(&hui::HuiWindow::setMenu));
			func_add_param("menu",		TypeHuiMenuP);
		class_add_func("setMaximized",		TypeVoid,		mf(&hui::HuiWindow::setMaximized));
			func_add_param("max",		TypeBool);
		class_add_func("isMaximized",		TypeBool,		mf(&hui::HuiWindow::isMaximized));
		class_add_func("isMinimized",		TypeBool,		mf(&hui::HuiWindow::isMinimized));
		class_add_func("setID",			TypeVoid,		mf(&hui::HuiWindow::setID));
			func_add_param("id",		TypeInt);
		class_add_func("setFullscreen",				TypeVoid,		mf(&hui::HuiWindow::setFullscreen));
			func_add_param("fullscreen",TypeBool);
		class_add_func("setTitle",										TypeVoid,		mf(&hui::HuiWindow::setTitle));
			func_add_param("title",		TypeString);
		class_add_func("setPosition",								TypeVoid,		mf(&hui::HuiWindow::setPosition));
			func_add_param("x",			TypeInt);
			func_add_param("y",			TypeInt);
	//add_func("setOuterior",								TypeVoid,		2,	TypePointer,"win",
	//																										TypeIRect,"r");
	//add_func("getOuterior",								TypeIRect,		1,	TypePointer,"win");
	//add_func("setInerior",								TypeVoid,		2,	TypePointer,"win",
	//																										TypeIRect,"r");
	//add_func("getInterior",									TypeIRect,		1,	TypePointer,"win");
		class_add_func("setCursorPos",								TypeVoid,		mf(&hui::HuiWindow::setCursorPos));
			func_add_param("x",			TypeInt);
			func_add_param("y",			TypeInt);
		class_add_func("getMouse",								TypeBool,		mf(&hui::HuiWindow::getMouse));
			func_add_param("x",			TypeIntPs);
			func_add_param("y",			TypeIntPs);
			func_add_param("button",	TypeInt);
			func_add_param("change",	TypeInt);
		class_add_func("getKey",							TypeBool,		mf(&hui::HuiWindow::getKey));
			func_add_param("key",			TypeInt);
		class_add_func_virtual("onMouseMove", TypeVoid, mf(&hui::HuiWindow::onMouseMove));
		class_add_func_virtual("onMouseWheel", TypeVoid, mf(&hui::HuiWindow::onMouseWheel));
		class_add_func_virtual("onLeftButtonDown", TypeVoid, mf(&hui::HuiWindow::onLeftButtonDown));
		class_add_func_virtual("onMiddleButtonDown", TypeVoid, mf(&hui::HuiWindow::onMiddleButtonDown));
		class_add_func_virtual("onRightButtonDown", TypeVoid, mf(&hui::HuiWindow::onRightButtonDown));
		class_add_func_virtual("onLeftButtonUp", TypeVoid, mf(&hui::HuiWindow::onLeftButtonUp));
		class_add_func_virtual("onMiddleButtonUp", TypeVoid, mf(&hui::HuiWindow::onMiddleButtonUp));
		class_add_func_virtual("onRightButtonUp", TypeVoid, mf(&hui::HuiWindow::onRightButtonUp));
		class_add_func_virtual("onDoubleClick", TypeVoid, mf(&hui::HuiWindow::onDoubleClick));
		class_add_func_virtual("onCloseRequest", TypeVoid, mf(&hui::HuiWindow::onCloseRequest));
		class_add_func_virtual("onKeyDown", TypeVoid, mf(&hui::HuiWindow::onKeyDown));
		class_add_func_virtual("onKeyUp", TypeVoid, mf(&hui::HuiWindow::onKeyUp));
		class_add_func_virtual("onDraw", TypeVoid, mf(&hui::HuiWindow::onDraw));
			func_add_param("p", TypeHuiPainterP);
		class_set_vtable(hui::HuiWindow);

	add_class(TypeHuiNixWindow);
		TypeHuiNixWindow->DeriveFrom(TypeHuiWindow, false);
		TypeHuiNixWindow->vtable = TypeHuiWindow->vtable;
		class_add_func(IDENTIFIER_FUNC_INIT,		TypeVoid,		mf(&hui::HuiNixWindow::__init_ext__), FLAG_OVERRIDE);
			func_add_param("title",		TypeString);
			func_add_param("x",		TypeInt);
			func_add_param("y",		TypeInt);
			func_add_param("width",		TypeInt);
			func_add_param("height",		TypeInt);
		class_add_func_virtual(IDENTIFIER_FUNC_DELETE,		TypeVoid,		mf(&hui::HuiWindow::__delete__), FLAG_OVERRIDE);
		class_set_vtable(hui::HuiWindow);

	add_class(TypeHuiDialog);
		TypeHuiDialog->DeriveFrom(TypeHuiWindow, false);
		TypeHuiDialog->vtable = TypeHuiWindow->vtable;
		class_add_func(IDENTIFIER_FUNC_INIT,		TypeVoid,		mf(&hui::HuiDialog::__init_ext__), FLAG_OVERRIDE);
			func_add_param("title",		TypeString);
			func_add_param("width",		TypeInt);
			func_add_param("height",		TypeInt);
			func_add_param("root",		TypeHuiWindowP);
			func_add_param("allow_root",TypeBool);
		class_add_func_virtual(IDENTIFIER_FUNC_DELETE,		TypeVoid,		mf(&hui::HuiWindow::__delete__), FLAG_OVERRIDE);
		class_set_vtable(hui::HuiWindow);

	add_class(TypeHuiFixedDialog);
		TypeHuiFixedDialog->DeriveFrom(TypeHuiWindow, false);
		TypeHuiFixedDialog->vtable = TypeHuiWindow->vtable;
		class_add_func(IDENTIFIER_FUNC_INIT,		TypeVoid,		mf(&hui::HuiFixedDialog::__init_ext__), FLAG_OVERRIDE);
			func_add_param("title",		TypeString);
			func_add_param("width",		TypeInt);
			func_add_param("height",		TypeInt);
			func_add_param("root",		TypeHuiWindowP);
			func_add_param("allow_root",TypeBool);
		class_add_func_virtual(IDENTIFIER_FUNC_DELETE,		TypeVoid,		mf(&hui::HuiWindow::__delete__), FLAG_OVERRIDE);
		class_set_vtable(hui::HuiWindow);
	
	add_class(TypeHuiPainter);
		class_add_element("width",		TypeInt,	GetDAPainter(width));
		class_add_element("height",		TypeInt,	GetDAPainter(height));
		//class_add_func_virtual("end",								TypeVoid,		mf(&hui::HuiPainter::end));
		class_add_func_virtual("setColor",								TypeVoid,		mf(&hui::HuiPainter::setColor));
			func_add_param("c",			TypeColor);
		class_add_func_virtual("setLineWidth",								TypeVoid,		mf(&hui::HuiPainter::setLineWidth));
			func_add_param("w",			TypeFloat32);
		class_add_func_virtual("setAntialiasing",								TypeVoid,		mf(&hui::HuiPainter::setAntialiasing));
			func_add_param("enabled",			TypeBool);
		class_add_func_virtual("setFontSize",								TypeVoid,		mf(&hui::HuiPainter::setFontSize));
			func_add_param("size",			TypeFloat32);
		class_add_func_virtual("setFill",								TypeVoid,		mf(&hui::HuiPainter::setFill));
			func_add_param("fill",			TypeBool);
		class_add_func_virtual("clip",								TypeVoid,		mf(&hui::HuiPainter::clip));
			func_add_param("r",			TypeRect);
		class_add_func_virtual("drawPoint",								TypeVoid,		mf(&hui::HuiPainter::drawPoint));
			func_add_param("x",			TypeFloat32);
			func_add_param("y",			TypeFloat32);
		class_add_func_virtual("drawLine",								TypeVoid,		mf(&hui::HuiPainter::drawLine));
			func_add_param("x1",		TypeFloat32);
			func_add_param("y1",		TypeFloat32);
			func_add_param("x2",		TypeFloat32);
			func_add_param("y2",		TypeFloat32);
		class_add_func_virtual("drawLines",								TypeVoid,		mf(&hui::HuiPainter::drawLines));
			func_add_param("p",			TypeComplexList);
		class_add_func_virtual("drawPolygon",								TypeVoid,		mf(&hui::HuiPainter::drawPolygon));
			func_add_param("p",			TypeComplexList);
		class_add_func_virtual("drawRect",								TypeVoid,		mf((void (hui::HuiPainter::*) (float,float,float,float))&hui::HuiPainter::drawRect));
			func_add_param("x",			TypeFloat32);
			func_add_param("y",			TypeFloat32);
			func_add_param("w",			TypeFloat32);
			func_add_param("h",			TypeFloat32);
		class_add_func_virtual("drawCircle",								TypeVoid,		mf(&hui::HuiPainter::drawCircle));
			func_add_param("x",			TypeFloat32);
			func_add_param("y",			TypeFloat32);
			func_add_param("r",			TypeFloat32);
		class_add_func_virtual("drawStr",								TypeVoid,		mf(&hui::HuiPainter::drawStr));
			func_add_param("x",			TypeFloat32);
			func_add_param("y",			TypeFloat32);
			func_add_param("str",		TypeString);
		class_add_func_virtual("drawImage",								TypeVoid,		mf(&hui::HuiPainter::drawImage));
			func_add_param("x",			TypeFloat32);
			func_add_param("y",			TypeFloat32);
			func_add_param("image",		TypeImage);
		class_set_vtable(hui::HuiPainter);


	add_class(TypeHuiTimer);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, mf(&hui::HuiTimer::reset));
		class_add_func("get", TypeFloat32, mf(&hui::HuiTimer::get));
		class_add_func("reset", TypeVoid, mf(&hui::HuiTimer::reset));
		class_add_func("peek", TypeFloat32, mf(&hui::HuiTimer::peek));


	add_class(TypeHuiConfiguration);
		class_add_func("setInt",								TypeVoid,	mf(&hui::HuiConfiguration::getInt));
			func_add_param("name",		TypeString);
			func_add_param("value",		TypeInt);
		class_add_func("setFloat",								TypeVoid,	mf(&hui::HuiConfiguration::getFloat));
			func_add_param("name",		TypeString);
			func_add_param("value",		TypeFloat32);
		class_add_func("setBool",								TypeVoid,	mf(&hui::HuiConfiguration::getBool));
			func_add_param("name",		TypeString);
			func_add_param("value",		TypeBool);
		class_add_func("setStr",								TypeVoid,	mf(&hui::HuiConfiguration::getStr));
			func_add_param("name",		TypeString);
			func_add_param("value",		TypeString);
		class_add_func("getInt",								TypeInt,	mf(&hui::HuiConfiguration::setInt));
			func_add_param("name",		TypeString);
			func_add_param("default",	TypeInt);
		class_add_func("getFloat",								TypeFloat32,	mf(&hui::HuiConfiguration::setFloat));
			func_add_param("name",		TypeString);
			func_add_param("default",	TypeFloat32);
		class_add_func("getBool",								TypeBool,	mf(&hui::HuiConfiguration::setBool));
			func_add_param("name",		TypeString);
			func_add_param("default",	TypeBool);
		class_add_func("getStr",								TypeString,	mf(&hui::HuiConfiguration::setStr));
			func_add_param("name",		TypeString);
			func_add_param("default",	TypeString);
	
	// user interface
	add_func("HuiSetIdleFunction",	TypeVoid,		(void*)&hui::HuiSetIdleFunction);
		func_add_param("idle_func",	TypePointer);
	add_func("HuiAddKeyCode",	TypeVoid,		(void*)&hui::HuiAddKeyCode);
		func_add_param("id",	TypeString);
		func_add_param("key_code",	TypeInt);
	add_func("HuiAddCommand",	TypeVoid,		(void*)&hui::HuiAddCommand);
		func_add_param("id",	TypeString);
		func_add_param("image",	TypeString);
		func_add_param("key_code",	TypeInt);
		func_add_param("func",	TypePointer);
	add_func("HuiGetEvent",	TypeHuiEventP,		(void*)&hui::HuiGetEvent);
	add_func("HuiRun",				TypeVoid,		(void*)&hui::HuiRun);
	add_func("HuiEnd",				TypeVoid,		(void*)&hui::HuiEnd);
	add_func("HuiDoSingleMainLoop",	TypeVoid,	(void*)&hui::HuiDoSingleMainLoop);
	add_func("HuiSleep",			TypeVoid,	(void*)&hui::HuiSleep);
		func_add_param("duration",		TypeFloat32);
	add_func("HuiFileDialogOpen",	TypeBool,	(void*)&hui::HuiFileDialogOpen);
		func_add_param("root",		TypeHuiWindowP);
		func_add_param("title",		TypeString);
		func_add_param("dir",		TypeString);
		func_add_param("show_filter",	TypeString);
		func_add_param("filter",	TypeString);
	add_func("HuiFileDialogSave",	TypeBool,	(void*)&hui::HuiFileDialogSave);
		func_add_param("root",		TypeHuiWindowP);
		func_add_param("title",		TypeString);
		func_add_param("dir",		TypeString);
		func_add_param("show_filter",	TypeString);
		func_add_param("filter",	TypeString);
	add_func("HuiFileDialogDir",	TypeBool,	(void*)&hui::HuiFileDialogDir);
		func_add_param("root",		TypeHuiWindowP);
		func_add_param("title",		TypeString);
		func_add_param("dir",		TypeString);
	add_func("HuiQuestionBox",		TypeString,	(void*)&hui::HuiQuestionBox);
		func_add_param("root",		TypeHuiWindowP);
		func_add_param("title",		TypeString);
		func_add_param("text",		TypeString);
		func_add_param("allow_cancel",	TypeBool);
	add_func("HuiInfoBox",			TypeVoid,			(void*)&hui::HuiInfoBox);
		func_add_param("root",		TypeHuiWindowP);
		func_add_param("title",		TypeString);
		func_add_param("text",		TypeString);
	add_func("HuiErrorBox",			TypeVoid,		(void*)&hui::HuiErrorBox);
		func_add_param("root",		TypeHuiWindowP);
		func_add_param("title",		TypeString);
		func_add_param("text",		TypeString);

	// clipboard
	add_func("HuiCopyToClipboard",	TypeVoid,			(void*)&hui::HuiCopyToClipBoard);
		func_add_param("buffer",	TypeString);
	add_func("HuiPasteFromClipboard",	TypeString,		(void*)&hui::HuiPasteFromClipBoard);
	add_func("HuiOpenDocument",		TypeVoid,			(void*)&hui::HuiOpenDocument);
		func_add_param("filename",	TypeString);
	add_func("HuiSetImage",			TypeString,			(void*)&hui::HuiSetImage);
		func_add_param("image",		TypeImage);

	add_class(TypeHuiEvent);
		class_add_element("id",			TypeString,	GetDAEvent(id));
		class_add_element("message",	TypeString,	GetDAEvent(message));
		class_add_element("mouse_x",	TypeFloat32,	GetDAEvent(mx));
		class_add_element("mouse_y",	TypeFloat32,	GetDAEvent(my));
		class_add_element("scroll_x",		TypeFloat32,	GetDAEvent(scroll_x));
		class_add_element("scroll_y",		TypeFloat32,	GetDAEvent(scroll_y));
		class_add_element("key",		TypeInt,	GetDAEvent(key));
		class_add_element("key_code",	TypeInt,	GetDAEvent(key_code));
		class_add_element("width",		TypeInt,	GetDAEvent(width));
		class_add_element("height",		TypeInt,	GetDAEvent(height));
		class_add_element("button_l",	TypeBool,	GetDAEvent(lbut));
		class_add_element("button_m",	TypeBool,	GetDAEvent(mbut));
		class_add_element("button_r",	TypeBool,	GetDAEvent(rbut));
		class_add_element("text",		TypeString,	GetDAEvent(text));
		class_add_element("row",		TypeInt,	GetDAEvent(row));
		class_add_element("column",		TypeInt,	GetDAEvent(column));

	// key ids (int)
	add_const("KeyControl",TypeInt,(void*)hui::KEY_CONTROL);
	add_const("KeyControlL",TypeInt,(void*)hui::KEY_LCONTROL);
	add_const("KeyControlR",TypeInt,(void*)hui::KEY_RCONTROL);
	add_const("KeyShift",TypeInt,(void*)hui::KEY_SHIFT);
	add_const("KeyShiftL",TypeInt,(void*)hui::KEY_LSHIFT);
	add_const("KeyShiftR",TypeInt,(void*)hui::KEY_RSHIFT);
	add_const("KeyAlt",TypeInt,(void*)hui::KEY_ALT);
	add_const("KeyAltL",TypeInt,(void*)hui::KEY_LALT);
	add_const("KeyAltR",TypeInt,(void*)hui::KEY_RALT);
	add_const("KeyPlus",TypeInt,(void*)hui::KEY_ADD);
	add_const("KeyMinus",TypeInt,(void*)hui::KEY_SUBTRACT);
	add_const("KeyFence",TypeInt,(void*)hui::KEY_FENCE);
	add_const("KeyEnd",TypeInt,(void*)hui::KEY_END);
	add_const("KeyNext",TypeInt,(void*)hui::KEY_NEXT);
	add_const("KeyPrior",TypeInt,(void*)hui::KEY_PRIOR);
	add_const("KeyUp",TypeInt,(void*)hui::KEY_UP);
	add_const("KeyDown",TypeInt,(void*)hui::KEY_DOWN);
	add_const("KeyLeft",TypeInt,(void*)hui::KEY_LEFT);
	add_const("KeyRight",TypeInt,(void*)hui::KEY_RIGHT);
	add_const("KeyReturn",TypeInt,(void*)hui::KEY_RETURN);
	add_const("KeyEscape",TypeInt,(void*)hui::KEY_ESCAPE);
	add_const("KeyInsert",TypeInt,(void*)hui::KEY_INSERT);
	add_const("KeyDelete",TypeInt,(void*)hui::KEY_DELETE);
	add_const("KeySpace",TypeInt,(void*)hui::KEY_SPACE);
	add_const("KeyF1",TypeInt,(void*)hui::KEY_F1);
	add_const("KeyF2",TypeInt,(void*)hui::KEY_F2);
	add_const("KeyF3",TypeInt,(void*)hui::KEY_F3);
	add_const("KeyF4",TypeInt,(void*)hui::KEY_F4);
	add_const("KeyF5",TypeInt,(void*)hui::KEY_F5);
	add_const("KeyF6",TypeInt,(void*)hui::KEY_F6);
	add_const("KeyF7",TypeInt,(void*)hui::KEY_F7);
	add_const("KeyF8",TypeInt,(void*)hui::KEY_F8);
	add_const("KeyF9",TypeInt,(void*)hui::KEY_F9);
	add_const("KeyF10",TypeInt,(void*)hui::KEY_F10);
	add_const("KeyF11",TypeInt,(void*)hui::KEY_F11);
	add_const("KeyF12",TypeInt,(void*)hui::KEY_F12);
	add_const("Key0",TypeInt,(void*)hui::KEY_0);
	add_const("Key1",TypeInt,(void*)hui::KEY_1);
	add_const("Key2",TypeInt,(void*)hui::KEY_2);
	add_const("Key3",TypeInt,(void*)hui::KEY_3);
	add_const("Key4",TypeInt,(void*)hui::KEY_4);
	add_const("Key5",TypeInt,(void*)hui::KEY_5);
	add_const("Key6",TypeInt,(void*)hui::KEY_6);
	add_const("Key7",TypeInt,(void*)hui::KEY_7);
	add_const("Key8",TypeInt,(void*)hui::KEY_8);
	add_const("Key9",TypeInt,(void*)hui::KEY_9);
	add_const("KeyA",TypeInt,(void*)hui::KEY_A);
	add_const("KeyB",TypeInt,(void*)hui::KEY_B);
	add_const("KeyC",TypeInt,(void*)hui::KEY_C);
	add_const("KeyD",TypeInt,(void*)hui::KEY_D);
	add_const("KeyE",TypeInt,(void*)hui::KEY_E);
	add_const("KeyF",TypeInt,(void*)hui::KEY_F);
	add_const("KeyG",TypeInt,(void*)hui::KEY_G);
	add_const("KeyH",TypeInt,(void*)hui::KEY_H);
	add_const("KeyI",TypeInt,(void*)hui::KEY_I);
	add_const("KeyJ",TypeInt,(void*)hui::KEY_J);
	add_const("KeyK",TypeInt,(void*)hui::KEY_K);
	add_const("KeyL",TypeInt,(void*)hui::KEY_L);
	add_const("KeyM",TypeInt,(void*)hui::KEY_M);
	add_const("KeyN",TypeInt,(void*)hui::KEY_N);
	add_const("KeyO",TypeInt,(void*)hui::KEY_O);
	add_const("KeyP",TypeInt,(void*)hui::KEY_P);
	add_const("KeyQ",TypeInt,(void*)hui::KEY_Q);
	add_const("KeyR",TypeInt,(void*)hui::KEY_R);
	add_const("KeyS",TypeInt,(void*)hui::KEY_S);
	add_const("KeyT",TypeInt,(void*)hui::KEY_T);
	add_const("KeyU",TypeInt,(void*)hui::KEY_U);
	add_const("KeyV",TypeInt,(void*)hui::KEY_V);
	add_const("KeyW",TypeInt,(void*)hui::KEY_W);
	add_const("KeyX",TypeInt,(void*)hui::KEY_X);
	add_const("KeyY",TypeInt,(void*)hui::KEY_Y);
	add_const("KeyZ",TypeInt,(void*)hui::KEY_Z);
	add_const("KeyBackspace",TypeInt,(void*)hui::KEY_BACKSPACE);
	add_const("KeyTab",TypeInt,(void*)hui::KEY_TAB);
	add_const("KeyHome",TypeInt,(void*)hui::KEY_HOME);
	add_const("KeyNum0",TypeInt,(void*)hui::KEY_NUM_0);
	add_const("KeyNum1",TypeInt,(void*)hui::KEY_NUM_1);
	add_const("KeyNum2",TypeInt,(void*)hui::KEY_NUM_2);
	add_const("KeyNum3",TypeInt,(void*)hui::KEY_NUM_3);
	add_const("KeyNum4",TypeInt,(void*)hui::KEY_NUM_4);
	add_const("KeyNum5",TypeInt,(void*)hui::KEY_NUM_5);
	add_const("KeyNum6",TypeInt,(void*)hui::KEY_NUM_6);
	add_const("KeyNum7",TypeInt,(void*)hui::KEY_NUM_7);
	add_const("KeyNum8",TypeInt,(void*)hui::KEY_NUM_8);
	add_const("KeyNum9",TypeInt,(void*)hui::KEY_NUM_9);
	add_const("KeyNumPlus",TypeInt,(void*)hui::KEY_NUM_ADD);
	add_const("KeyNumMinus",TypeInt,(void*)hui::KEY_NUM_SUBTRACT);
	add_const("KeyNumMultiply",TypeInt,(void*)hui::KEY_NUM_MULTIPLY);
	add_const("KeyNumDivide",TypeInt,(void*)hui::KEY_NUM_DIVIDE);
	add_const("KeyNumComma",TypeInt,(void*)hui::KEY_NUM_COMMA);
	add_const("KeyNumEnter",TypeInt,(void*)hui::KEY_NUM_ENTER);
	add_const("KeyComma",TypeInt,(void*)hui::KEY_COMMA);
	add_const("KeyDot",TypeInt,(void*)hui::KEY_DOT);
	add_const("KeySmaller",TypeInt,(void*)hui::KEY_SMALLER);
	add_const("KeySz",TypeInt,(void*)hui::KEY_SZ);
	add_const("KeyAe",TypeInt,(void*)hui::KEY_AE);
	add_const("KeyOe",TypeInt,(void*)hui::KEY_OE);
	add_const("KeyUe",TypeInt,(void*)hui::KEY_UE);
	add_const("NumKeys",TypeInt,(void*)hui::HUI_NUM_KEYS);
	add_const("KeyAny",TypeInt,(void*)hui::KEY_ANY);

	add_ext_var("AppFilename",		TypeString,		hui_p(&hui::HuiAppFilename));
	add_ext_var("AppDirectory",		TypeString,		hui_p(&hui::HuiAppDirectory));
	add_ext_var("AppDirectoryStatic",TypeString,		hui_p(&hui::HuiAppDirectoryStatic));
	add_ext_var("HuiFilename",		TypeString,		hui_p(&hui::HuiFilename));
	//add_ext_var("HuiRunning",		TypeBool,		hui_p(&hui::HuiRunning));
	add_ext_var("HuiConfig",		TypeHuiConfiguration,	hui_p(&hui::HuiConfig));
}

};
