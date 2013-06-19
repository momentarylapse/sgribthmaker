/*
 * HuiControlGroupGtk.cpp
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#include "HuiControlGroupGtk.h"

HuiControlGroupGtk::HuiControlGroupGtk(const string &title, const string &id) :
	HuiControlGtk(HuiKindGroup, id)
{
	GetPartStrings(id, title);
	widget = gtk_frame_new(sys_str(PartString[0]));
}

HuiControlGroupGtk::~HuiControlGroupGtk() {
	// TODO Auto-generated destructor stub
}

