/*
 * ParserKaba.h
 *
 *  Created on: 06.06.2013
 *      Author: michi
 */

#ifndef PARSERKABA_H_
#define PARSERKABA_H_

#include "BaseParser.h"

class ParserKaba : public Parser
{
public:
	ParserKaba();
	virtual ~ParserKaba();
	virtual string GetName(){	return "Kaba";	}

	virtual Array<Label> FindLabels(SourceView *sv);
	virtual int WordType(const string &name);
	virtual void CreateTextColors(SourceView *sv, int first_line = -1, int last_line = -1);
};

#endif /* PARSERKABA_H_ */
