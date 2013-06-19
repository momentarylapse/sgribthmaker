/*
 * HuiControlToggleButtonGtk.h
 *
 *  Created on: 18.06.2013
 *      Author: michi
 */

#ifndef HUICONTROLTOGGLEBUTTONGTK_H_
#define HUICONTROLTOGGLEBUTTONGTK_H_

#include "HuiControlGtk.h"


class HuiControlToggleButtonGtk : public HuiControlGtk
{
public:
	HuiControlToggleButtonGtk(const string &text, const string &id);
	virtual ~HuiControlToggleButtonGtk();
	virtual string GetString();
	virtual void SetString(const string &str);
	virtual void SetImage(const string &str);
	virtual void Check(bool checked);
	virtual bool IsChecked();
};

#endif /* HUICONTROLTOGGLEBUTTONGTK_H_ */
