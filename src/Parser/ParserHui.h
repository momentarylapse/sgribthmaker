/*
 * ParserHui.h
 *
 *  Created on: 27.03.2021
 *      Author: michi
 */

#ifndef PARSERHUI_H_
#define PARSERHUI_H_

#include "BaseParser.h"

class ParserHui : public Parser {
public:
	ParserHui();

	void CreateTextColors(SourceView *sv, int first_line = -1, int last_line = -1) override;
};

#endif /* PARSERHUI_H_ */
