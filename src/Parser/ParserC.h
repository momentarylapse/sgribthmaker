/*
 * ParserC.h
 *
 *  Created on: 06.06.2013
 *      Author: michi
 */

#ifndef PARSERC_H_
#define PARSERC_H_

#include "BaseParser.h"

class ParserC : public Parser {
public:
	ParserC();

	void CreateTextColors(SourceView *sv, int first_line = -1, int last_line = -1) override;
};

#endif /* PARSERC_H_ */
