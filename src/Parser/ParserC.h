/*
 * ParserC.h
 *
 *  Created on: 06.06.2013
 *      Author: michi
 */

#ifndef PARSERC_H_
#define PARSERC_H_

#include "BaseParser.h"

class ParserC : public Parser
{
public:
	ParserC();
	virtual ~ParserC();
	virtual string GetName(){	return "C/C++";	}

	virtual int WordType(const string &name);
	virtual void CreateTextColors(SourceView *sv, int first_line = -1, int last_line = -1);
};

#endif /* PARSERC_H_ */
