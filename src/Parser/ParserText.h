/*
 * ParserText.h
 *
 *  Created on: 06.06.2013
 *      Author: michi
 */

#ifndef PARSERTEXT_H_
#define PARSERTEXT_H_

#include "Parser.h"

class ParserText: public Parser
{
public:
	ParserText();
	virtual ~ParserText();
	virtual string GetName(){	return "Text";	}
};

#endif /* PARSERTEXT_H_ */
