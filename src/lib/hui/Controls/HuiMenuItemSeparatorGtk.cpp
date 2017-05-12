/*
 * HuiMenuItemSeparatorGtk.cpp
 *
 *  Created on: 26.06.2013
 *      Author: michi
 */

#include "HuiMenuItemSeparator.h"

#ifdef HUI_API_GTK

namespace hui
{

HuiMenuItemSeparator::HuiMenuItemSeparator() :
	Control(HUI_KIND_MENU_SEPARATOR, "")
{
	widget = gtk_separator_menu_item_new();
}

};

#endif

