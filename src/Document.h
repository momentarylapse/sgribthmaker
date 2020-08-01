/*
 * Document.h
 *
 *  Created on: 18.02.2014
 *      Author: michi
 */

#ifndef DOCUMENT_H_
#define DOCUMENT_H_

#include "lib/base/base.h"
#include "lib/file/path.h"

class History;
class Parser;
class HighlightScheme;
class SourceView;
class SgribthMaker;

class Document {
public:
	Document(SgribthMaker *sgribthmaker);
	virtual ~Document();

	string name(bool long_name) const;
	bool load(const Path &filename);
	bool save(const Path &filename);

	Path filename;
	string buffer;
	bool buffer_dirty;

	History *history;
	Parser *parser;
	HighlightScheme *scheme;
	SourceView *source_view;
	SgribthMaker *sgribthmaker;
};

#endif /* DOCUMENT_H_ */
