/*
 * CodeCompiler.h
 *
 *  Created on: 1 Jun 2023
 *      Author: michi
 */

#ifndef SRC_CODECOMPILER_H_
#define SRC_CODECOMPILER_H_

class Document;

class CodeCompiler {
public:
	explicit CodeCompiler(Document *doc);
	~CodeCompiler();

	void compile();
	void compile_and_run(bool verbose);

	void compile_kaba();
	void compile_shader();

	Document *doc;
};

#endif /* SRC_CODECOMPILER_H_ */
