/*
 * HuiControlRadioButtonGtk.h
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#ifndef HUICONTROLRADIOBUTTONGTK_H_
#define HUICONTROLRADIOBUTTONGTK_H_

#include "HuiControlGtk.h"


class HuiControlRadioButtonGtk : public HuiControlGtk
{
public:
	HuiControlRadioButtonGtk(const string &text, const string &id);
	virtual ~HuiControlRadioButtonGtk();
	virtual string GetString();
	virtual void SetString(const string &str);
	virtual void Check(bool checked);
	virtual bool IsChecked();
};

#endif /* HUICONTROLRADIOBUTTONGTK_H_ */
