/*
 * HuiControlDrawingAreaGtk.h
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#ifndef HUICONTROLDRAWINGAREAGTK_H_
#define HUICONTROLDRAWINGAREAGTK_H_

#include "HuiControlGtk.h"


class HuiControlDrawingAreaGtk : public HuiControlGtk
{
public:
	HuiControlDrawingAreaGtk(const string &text, const string &id);
	virtual ~HuiControlDrawingAreaGtk();
};

#endif /* HUICONTROLDRAWINGAREAGTK_H_ */
