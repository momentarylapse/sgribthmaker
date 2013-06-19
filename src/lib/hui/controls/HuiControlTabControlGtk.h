/*
 * HuiControlTabControlGtk.h
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#ifndef HUICONTROLTABCONTROLGTK_H_
#define HUICONTROLTABCONTROLGTK_H_

#include "HuiControlGtk.h"


class HuiControlTabControlGtk : public HuiControlGtk
{
public:
	HuiControlTabControlGtk(const string &text, const string &id, HuiWindow *win);
	virtual ~HuiControlTabControlGtk();
	virtual string GetString();
	virtual void SetString(const string &str);
	virtual void SetInt(int i);
	virtual int GetInt();

	int cur_page;
};

#endif /* HUICONTROLTABCONTROLGTK_H_ */
