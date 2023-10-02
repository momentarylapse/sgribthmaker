/*
 * ParserIni.h
 *
 *  Created on: 2 Oct 2023
 *      Author: michi
 */

#ifndef SRC_PARSER_PARSERINI_H_
#define SRC_PARSER_PARSERINI_H_

#include "BaseParser.h"

class ParserIni : public Parser {
public:
	ParserIni();

	void CreateTextColors(SourceView *sv, int first_line = -1, int last_line = -1) override;


	void CreateTextColorsHeader(SourceView *sv, int line_no, const string& line);
	void CreateTextColorsKeyValue(SourceView *sv, int line_no, const string& line);
};

#endif /* SRC_PARSER_PARSERINI_H_ */
