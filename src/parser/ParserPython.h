/*
 * ParserPython.h
 *
 *  Created on: 25.06.2013
 *      Author: michi
 */

#ifndef PARSERPYTHON_H_
#define PARSERPYTHON_H_

#include "BaseParser.h"

class ParserPython : public Parser {
public:
	ParserPython();

	void CreateTextColors(SourceView *sv, int first_line = -1, int last_line = -1) override;
};

#endif /* PARSERPYTHON_H_ */
