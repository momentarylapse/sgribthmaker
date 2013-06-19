/*
 * HuiControlMultilineEditGtk.h
 *
 *  Created on: 18.06.2013
 *      Author: michi
 */

#ifndef HUICONTROLMULTILINEEDITGTK_H_
#define HUICONTROLMULTILINEEDITGTK_H_

#include "HuiControlGtk.h"


class HuiControlMultilineEditGtk : public HuiControlGtk
{
public:
	HuiControlMultilineEditGtk(const string &text, const string &id);
	virtual ~HuiControlMultilineEditGtk();
	virtual string GetString();
	virtual void SetString(const string &str);
	virtual void AddString(const string &str);
};

#endif /* HUICONTROLMULTILINEEDITGTK_H_ */
