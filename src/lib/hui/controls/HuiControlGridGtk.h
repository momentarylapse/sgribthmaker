/*
 * HuiControlGridGtk.h
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#ifndef HUICONTROLGRIDGTK_H_
#define HUICONTROLGRIDGTK_H_

#include "HuiControlGtk.h"


class HuiControlGridGtk : public HuiControlGtk
{
public:
	HuiControlGridGtk(const string &text, const string &id, int num_x, int num_y, HuiWindow *win);
	virtual ~HuiControlGridGtk();
};

#endif /* HUICONTROLGRIDGTK_H_ */
