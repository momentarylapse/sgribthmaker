/*
 * HuiControlProgressBarGtk.h
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#ifndef HUICONTROLPROGRESSBARGTK_H_
#define HUICONTROLPROGRESSBARGTK_H_

#include "HuiControlGtk.h"


class HuiControlProgressBarGtk : public HuiControlGtk
{
public:
	HuiControlProgressBarGtk(const string &text, const string &id);
	virtual ~HuiControlProgressBarGtk();
	virtual string GetString();
	virtual void SetString(const string &str);
	virtual float GetFloat();
	virtual void SetFloat(float f);
};

#endif /* HUICONTROLPROGRESSBARGTK_H_ */
