/*
 * Document.h
 *
 *  Created on: 18.02.2014
 *      Author: michi
 */

#ifndef DOCUMENT_H_
#define DOCUMENT_H_

#include "lib/base/base.h"

class History;
class Parser;
class HighlightScheme;
class SourceView;
class SgribthMaker;

class Document
{
public:
	Document(SgribthMaker *sgribthmaker);
	virtual ~Document();

	string name(bool long_name) const;
	bool load(const string &filename);
	bool save(const string &filename);

	string filename;
	string buffer;
	bool buffer_dirty;

	History *history;
	Parser *parser;
	HighlightScheme *scheme;
	SourceView *source_view;
	SgribthMaker *sgribthmaker;
};

#endif /* DOCUMENT_H_ */
