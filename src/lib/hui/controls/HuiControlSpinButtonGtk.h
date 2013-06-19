/*
 * HuiControlSpinButtonGtk.h
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#ifndef HUICONTROLSPINBUTTONGTK_H_
#define HUICONTROLSPINBUTTONGTK_H_

#include "HuiControlGtk.h"


class HuiControlSpinButtonGtk : public HuiControlGtk
{
public:
	HuiControlSpinButtonGtk(const string &text, const string &id);
	virtual ~HuiControlSpinButtonGtk();
	virtual string GetString();
	virtual void SetString(const string &str);
	virtual void SetInt(int i);
	virtual int GetInt();
	virtual float GetFloat();
	virtual void SetFloat(float f);
};

#endif /* HUICONTROLSPINBUTTONGTK_H_ */
