/*
 * HuiControlLabelGtk.h
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#ifndef HUI_CONTROL_LABEL_GTK_H_
#define HUI_CONTROL_LABEL_GTK_H_

#include "HuiControlGtk.h"


class HuiControlLabelGtk : public HuiControlGtk
{
public:
	HuiControlLabelGtk(const string &text, const string &id);
	virtual ~HuiControlLabelGtk();
	virtual string GetString();
	virtual void SetString(const string &str);
};


#endif /* HUI_CONTROL_LABEL_GTK_H_ */
