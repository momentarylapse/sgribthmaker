/*
 * HuiControlMultilineEdit.h
 *
 *  Created on: 18.06.2013
 *      Author: michi
 */

#ifndef HUICONTROLMULTILINEEDIT_H_
#define HUICONTROLMULTILINEEDIT_H_

#include "HuiControl.h"


class HuiControlMultilineEdit : public HuiControl
{
public:
	HuiControlMultilineEdit(const string &text, const string &id);
	virtual ~HuiControlMultilineEdit();
	virtual string GetString();
	virtual void SetString(const string &str);
	virtual void AddString(const string &str);
};

#endif /* HUICONTROLMULTILINEEDIT_H_ */
