/*
 * HuiControlSliderGtk.h
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#ifndef HUICONTROLSLIDERGTK_H_
#define HUICONTROLSLIDERGTK_H_

#include "HuiControlGtk.h"


class HuiControlSliderGtk : public HuiControlGtk
{
public:
	HuiControlSliderGtk(const string &text, const string &id, bool horizontal);
	virtual ~HuiControlSliderGtk();
	virtual float GetFloat();
	virtual void SetFloat(float f);
};

#endif /* HUICONTROLSLIDERGTK_H_ */
