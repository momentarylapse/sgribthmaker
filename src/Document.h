/*
 * Document.h
 *
 *  Created on: 18.02.2014
 *      Author: michi
 */

#ifndef DOCUMENT_H_
#define DOCUMENT_H_

#include "lib/base/base.h"
#include "lib/os/path.h"
#include "lib/pattern/Observable.h"

class History;
class Parser;
class HighlightScheme;
class SourceView;
class SgribthMakerWindow;
class CodeCompiler;

class Document : public obs::Node<VirtualBase> {
public:
	Document(SgribthMakerWindow *win);
	virtual ~Document();

	obs::Source out_not_utf8{this, "not-utf8"};

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
	SgribthMakerWindow *win;
	CodeCompiler *compiler;
};

#endif /* DOCUMENT_H_ */
