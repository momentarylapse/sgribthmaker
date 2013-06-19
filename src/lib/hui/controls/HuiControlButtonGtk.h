/*
 * HuiControlButtonGtk.h
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#ifndef HUICONTROLBUTTONGTK_H_
#define HUICONTROLBUTTONGTK_H_

#include "HuiControlGtk.h"


class HuiControlButtonGtk : public HuiControlGtk
{
public:
	HuiControlButtonGtk(const string &text, const string &id);
	virtual ~HuiControlButtonGtk();
	virtual string GetString();
	virtual void SetString(const string &str);
	virtual void SetImage(const string &str);
};

#endif /* HUICONTROLBUTTONGTK_H_ */
