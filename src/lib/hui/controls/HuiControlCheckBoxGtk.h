/*
 * HuiControlCheckBoxGtk.h
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#ifndef HUICONTROLCHECKBOXGTK_H_
#define HUICONTROLCHECKBOXGTK_H_

#include "HuiControlGtk.h"


class HuiControlCheckBoxGtk : public HuiControlGtk
{
public:
	HuiControlCheckBoxGtk(const string &text, const string &id);
	virtual ~HuiControlCheckBoxGtk();
	virtual string GetString();
	virtual void SetString(const string &str);
	virtual void Check(bool checked);
	virtual bool IsChecked();
};

#endif /* HUICONTROLCHECKBOXGTK_H_ */
