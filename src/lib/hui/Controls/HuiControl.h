/*
 * hui_control.h
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#ifndef HUI_CONTROL_H_
#define HUI_CONTROL_H_

#include "../hui_common.h"
#include "../hui_language.h"
#include "../../image/image.h"

class HuiWindow;

extern int allow_signal_level; // -> hui_window_control.cpp


void GetPartStrings(const string &id, const string &title);
//string ScanOptions(int id, const string &title);
extern Array<string> PartString;
extern string OptionString, HuiFormatString;

class HuiControl
{
public:
	HuiControl(int _type, const string &_id);
	virtual ~HuiControl();
	int type;
	string id;
	int x, y;
#ifdef HUI_API_WIN
	HWND hWnd, hWnd2;//,hWnd3;
	//Array<HWND> _item_;
	//int color[4]; // ColorButton...
#endif
#ifdef HUI_API_GTK
    GtkWidget *widget;
    GtkWidget *frame;
#endif
	bool enabled;
	bool is_button_bar;
	HuiWindow *win;

	virtual void Reset(){}
	virtual string GetString(){	return "";	}
	virtual int GetInt(){	return GetString()._int();	}
	virtual float GetFloat(){	return GetString()._float();	}
	virtual color GetColor(){	return Black;	}
	virtual void SetString(const string &str){}
	virtual void AddString(const string &str){}
	virtual void SetInt(int i){	SetString(i2s(i));	}
	virtual void SetFloat(float f){	SetString(f2s(f, 3));	}
	virtual void SetColor(const color &c){}
	virtual void SetImage(const string &str){}

	virtual void AddChildString(int parent_row, const string &str){}
	virtual void ChangeString(int row, const string &str){}
	virtual string GetCell(int row, int column){	return "";	}
	virtual void SetCell(int row, int column, const string &str){}
	virtual Array<int> GetMultiSelection(){	Array<int> r;	return r;	}
	virtual void SetMultiSelection(Array<int> &sel){}
	virtual void Expand(int row, bool expand){}
	virtual void ExpandAll(bool expand){}
	virtual bool IsExpanded(int row){	return false;	}

	virtual void Enable(bool enabled);
	virtual bool IsEnabled();
	virtual void Hide(bool hidden);
	virtual void Check(bool checked){}
	virtual bool IsChecked(){	return false;	}
	virtual void SetTooltip(const string &str);
	virtual void Focus();
	virtual void CompletionAdd(const string &text){}
	virtual void CompletionClear(){}

	void Notify(const string &message = "", bool is_default = true);
};



#endif /* HUI_CONTROL_H_ */