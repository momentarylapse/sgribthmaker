/*
 * HuiControlColorButtonGtk.h
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#ifndef HUICONTROLCOLORBUTTONGTK_H_
#define HUICONTROLCOLORBUTTONGTK_H_

#include "HuiControlGtk.h"


class HuiControlColorButtonGtk : public HuiControlGtk
{
public:
	HuiControlColorButtonGtk(const string &text, const string &id);
	virtual ~HuiControlColorButtonGtk();

	virtual void SetColor(const color &c);
	virtual color GetColor();
};

#endif /* HUICONTROLCOLORBUTTONGTK_H_ */
