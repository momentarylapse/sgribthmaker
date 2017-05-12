/*
 * HuiMenuItemSubmenu.h
 *
 *  Created on: 26.06.2013
 *      Author: michi
 */

#ifndef HUIMENUITEMSUBMENU_H_
#define HUIMENUITEMSUBMENU_H_

#include "Control.h"

namespace hui
{

class Menu;

class HuiMenuItemSubmenu : public Control
{
public:
	HuiMenuItemSubmenu(const string &title, Menu *menu, const string &id);
	virtual ~HuiMenuItemSubmenu();

	Menu *sub_menu;
};

};

#endif /* HUIMENUITEMSUBMENU_H_ */
