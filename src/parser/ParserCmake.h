/*
 * ParserCmake.h
 *
 *  Created on: 30.09.2024
 *      Author: michi
 */

#ifndef PARSERCMAKE_H_
#define PARSERCMAKE_H_

#include "BaseParser.h"

class ParserCmake : public Parser {
public:
	ParserCmake();

	void CreateTextColors(SourceView *sv, int first_line = -1, int last_line = -1) override;
};

#endif /* PARSERCMAKE_H_ */
