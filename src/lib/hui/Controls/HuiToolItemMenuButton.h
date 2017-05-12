/*
 * HuiToolItemMenuButton.h
 *
 *  Created on: 26.06.2013
 *      Author: michi
 */

#ifndef HUITOOLITEMMENUBUTTON_H_
#define HUITOOLITEMMENUBUTTON_H_

#include "Control.h"

namespace hui
{

class Menu;

class HuiToolItemMenuButton : public Control
{
public:
	HuiToolItemMenuButton(const string &title, Menu *menu, const string &image, const string &id);
};

}

#endif /* HUITOOLITEMMENUBUTTON_H_ */
