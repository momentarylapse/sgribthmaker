/*
 * HuiControlListViewGtk.h
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#ifndef HUICONTROLLISTVIEWGTK_H_
#define HUICONTROLLISTVIEWGTK_H_

#include "HuiControlGtk.h"


class HuiControlListViewGtk : public HuiControlGtk
{
public:
	HuiControlListViewGtk(const string &text, const string &id, HuiWindow *win);
	virtual ~HuiControlListViewGtk();
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

	Array<GtkTreeIter> _item_;
};

#endif /* HUICONTROLLISTVIEWGTK_H_ */
