/*
 * ParserKaba.h
 *
 *  Created on: 06.06.2013
 *      Author: michi
 */

#ifndef PARSERKABA_H_
#define PARSERKABA_H_

#include "BaseParser.h"

class ParserKaba : public Parser {
public:
	ParserKaba();

	Array<Label> FindLabels(SourceView *sv) override;
	void CreateTextColors(SourceView *sv, int first_line = -1, int last_line = -1) override;

	void clear_symbols() override;
	void update_symbols(SourceView *sv) override;
};

#endif /* PARSERKABA_H_ */
