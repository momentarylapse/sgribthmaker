/*
 * HuiMenuItem.h
 *
 *  Created on: 26.06.2013
 *      Author: michi
 */

#ifndef HUIMENUITEM_H_
#define HUIMENUITEM_H_

#include "Control.h"

namespace hui
{

class HuiMenuItem : public Control
{
public:
	HuiMenuItem(const string &title, const string &id);

	virtual void setImage(const string &image);
};

};

#endif /* HUIMENUITEM_H_ */
