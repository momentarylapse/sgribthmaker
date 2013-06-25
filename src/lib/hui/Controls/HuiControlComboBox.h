/*
 * HuiControlComboBox.h
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#ifndef HUICONTROLCOMBOBOX_H_
#define HUICONTROLCOMBOBOX_H_

#include "HuiControl.h"


class HuiControlComboBox : public HuiControl
{
public:
	HuiControlComboBox(const string &text, const string &id);
	virtual ~HuiControlComboBox();
	virtual string GetString();
	virtual void SetString(const string &str);
	virtual void AddString(const string &str);
	virtual void SetInt(int i);
	virtual int GetInt();
	virtual void Reset();
};

#endif /* HUICONTROLCOMBOBOX_H_ */
