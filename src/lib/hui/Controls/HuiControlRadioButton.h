/*
 * HuiControlRadioButton.h
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#ifndef HUICONTROLRADIOBUTTON_H_
#define HUICONTROLRADIOBUTTON_H_

#include "HuiControl.h"


class HuiControlRadioButton : public HuiControl
{
public:
	HuiControlRadioButton(const string &text, const string &id);
	virtual ~HuiControlRadioButton();
	virtual string GetString();
	virtual void SetString(const string &str);
	virtual void Check(bool checked);
	virtual bool IsChecked();
};

#endif /* HUICONTROLRADIOBUTTON_H_ */
