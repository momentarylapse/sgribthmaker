/*
 * HuiControlSpinButton.h
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#ifndef HUICONTROLSPINBUTTON_H_
#define HUICONTROLSPINBUTTON_H_

#include "HuiControl.h"


class HuiControlSpinButton : public HuiControl
{
public:
	HuiControlSpinButton(const string &text, const string &id);
	virtual ~HuiControlSpinButton();
	virtual string GetString();
	virtual void SetString(const string &str);
	virtual void SetInt(int i);
	virtual int GetInt();
	virtual float GetFloat();
	virtual void SetFloat(float f);
};

#endif /* HUICONTROLSPINBUTTON_H_ */