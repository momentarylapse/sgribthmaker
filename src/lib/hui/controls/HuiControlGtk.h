/*
 * HuiControlGtk.h
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#ifndef HUI_CONTROL_GTK_H_
#define HUI_CONTROL_GTK_H_

#include "HuiControl.h"
#include "../hui_language.h"

#ifdef HUI_API_GTK

void GetPartStrings(const string &id, const string &title);
//string ScanOptions(int id, const string &title);
extern Array<string> PartString;
extern string OptionString, HuiFormatString;

class HuiControlGtk : public HuiControl
{
public:
	HuiControlGtk(int _type, const string &_id);
	virtual ~HuiControlGtk();
    GtkWidget *widget;
    GtkWidget *frame;
	/*int selected;
	Array<GtkTreeIter> _item_;*/

	virtual void Enable(bool enabled);
	virtual bool IsEnabled();
	virtual void Hide(bool hidden);
	virtual void SetTooltip(const string &str);
	virtual void Focus();
};

#endif


#endif /* HUI_CONTROL_GTK_H_ */
