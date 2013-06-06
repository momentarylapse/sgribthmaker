/*
 * Parser.cpp
 *
 *  Created on: 06.06.2013
 *      Author: michi
 */

#include "Parser.h"
#include "ParserKaba.h"
#include "ParserText.h"
#include "../HighlightSchema.h"

struct ParserAssociation
{
	Parser *parser;
	string extension;
	ParserAssociation(){}
	ParserAssociation(Parser *p, const string &ext)
	{
		parser = p;
		extension = ext;
	}
};
static Array<ParserAssociation> ParserAssociations;

Parser::Label::Label(const string &_name, int _line)
{
	name = _name;
	line = _line;
}

Parser::Parser()
{
}

Parser::~Parser()
{
}


Array<Parser::Label> Parser::FindLabels(SourceView *sv)
{
	Array<Parser::Label> labels;
	return labels;
}

int Parser::WordType(const string &name)
{
	return InWord;
}

void Parser::CreateTextColors(SourceView *sv, int first_line, int last_line)
{
}




void InitParser()
{
	ParserAssociations.add(ParserAssociation(new ParserText, "*"));
	ParserAssociations.add(ParserAssociation(new ParserKaba, "kaba"));
}

Parser *GetParser(const string &filename)
{
	string ext = filename.extension();
	foreach(ParserAssociation &a, ParserAssociations)
		if (ext == a.extension)
			return a.parser;
	return ParserAssociations[0].parser;
}
