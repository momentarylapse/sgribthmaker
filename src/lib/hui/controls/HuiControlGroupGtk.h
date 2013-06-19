/*
 * HuiControlGroupGtk.h
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#ifndef HUICONTROLGROUPGTK_H_
#define HUICONTROLGROUPGTK_H_

#include "HuiControlGtk.h"


class HuiControlGroupGtk : public HuiControlGtk
{
public:
	HuiControlGroupGtk(const string &text, const string &id);
	virtual ~HuiControlGroupGtk();
	//virtual string GetString();
	//virtual void SetString(const string &str);
};

#endif /* HUICONTROLGROUPGTK_H_ */
