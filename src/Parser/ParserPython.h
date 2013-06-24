/*
 * ParserPython.h
 *
 *  Created on: 25.06.2013
 *      Author: michi
 */

#ifndef PARSERPYTHON_H_
#define PARSERPYTHON_H_

#include "Parser.h"

class ParserPython : public Parser
{
public:
	ParserPython();
	virtual ~ParserPython();
	virtual string GetName(){	return "Python";	}

	virtual int WordType(const string &name);
	virtual void CreateTextColors(SourceView *sv, int first_line = -1, int last_line = -1);
};

#endif /* PARSERPYTHON_H_ */
