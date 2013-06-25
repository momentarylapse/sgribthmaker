/*
 * HuiControlListView.h
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#ifndef HUICONTROLLISTVIEW_H_
#define HUICONTROLLISTVIEW_H_

#include "HuiControl.h"


class HuiControlListView : public HuiControl
{
public:
	HuiControlListView(const string &text, const string &id, HuiWindow *win);
	virtual ~HuiControlListView();
	virtual string GetString();
	virtual void SetString(const string &str);
	virtual void AddString(const string &str);
	virtual void SetInt(int i);
	virtual int GetInt();
	virtual void ChangeString(int row, const string &str);
	virtual string GetCell(int row, int column);
	virtual void SetCell(int row, int column, const string &str);
	virtual Array<int> GetMultiSelection();
	virtual void SetMultiSelection(Array<int> &sel);
	virtual void Reset();

#ifdef HUI_API_GTK
	Array<GtkTreeIter> _item_;
#endif
};

#endif /* HUICONTROLLISTVIEW_H_ */
