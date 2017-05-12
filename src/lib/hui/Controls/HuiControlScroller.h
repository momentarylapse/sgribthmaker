/*
 * HuiControlScroller.h
 *
 *  Created on: 18.09.2013
 *      Author: michi
 */

#ifndef HUICONTROLSCROLLER_H_
#define HUICONTROLSCROLLER_H_

#include "HuiControl.h"

namespace hui
{

class HuiControlScroller : public HuiControl
{
public:
	HuiControlScroller(const string &text, const string &id);

	virtual void add(HuiControl *child, int x, int y);

	GtkWidget *viewport;
};

};

#endif /* HUICONTROLSCROLLER_H_ */
