/*
 * HuiControlEditGtk.h
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#ifndef HUICONTROLEDITGTK_H_
#define HUICONTROLEDITGTK_H_

#include "HuiControlGtk.h"


class HuiControlEditGtk : public HuiControlGtk
{
public:
	HuiControlEditGtk(const string &text, const string &id);
	virtual ~HuiControlEditGtk();
	virtual string GetString();
	virtual void SetString(const string &str);
	virtual void CompletionAdd(const string &text);
	virtual void CompletionClear();
};

#endif /* HUICONTROLEDITGTK_H_ */
