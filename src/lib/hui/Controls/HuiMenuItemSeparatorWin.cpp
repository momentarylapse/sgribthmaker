/*
 * HuiMenuItemSeparatorWin.cpp
 *
 *  Created on: 26.06.2013
 *      Author: michi
 */

#include "HuiMenuItemSeparator.h"

namespace hui
{

#ifdef HUI_API_WIN

HuiMenuItemSeparator::HuiMenuItemSeparator() :
	HuiControl(HuiKindMenuItemSeparator, "")
{
}

HuiMenuItemSeparator::~HuiMenuItemSeparator()
{
}

#endif

};
