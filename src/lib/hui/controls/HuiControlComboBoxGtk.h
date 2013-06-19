/*
 * HuiControlComboBoxGtk.h
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#ifndef HUICONTROLCOMBOBOXGTK_H_
#define HUICONTROLCOMBOBOXGTK_H_

#include "HuiControlGtk.h"


class HuiControlComboBoxGtk : public HuiControlGtk
{
public:
	HuiControlComboBoxGtk(const string &text, const string &id);
	virtual ~HuiControlComboBoxGtk();
	virtual string GetString();
	virtual void SetString(const string &str);
	virtual void AddString(const string &str);
	virtual void SetInt(int i);
	virtual int GetInt();
	virtual void Reset();
};

#endif /* HUICONTROLCOMBOBOXGTK_H_ */
