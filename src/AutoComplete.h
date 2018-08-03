/*
 * AutoComplete.h
 *
 *  Created on: 02.08.2018
 *      Author: michi
 */

#ifndef SRC_AUTOCOMPLETE_H_
#define SRC_AUTOCOMPLETE_H_

#include "lib/base/base.h"

class AutoComplete
{
public:
	//AutoComplete();

	struct Item
	{
		string name, context;
	};

	struct Data
	{
		Array<Item> suggestions;
		int offset;
		void add(const string &name, const string &context);
		void append(const Data &d);
	};

	static Data run(const string &code, int line, int pos);
};

#endif /* SRC_AUTOCOMPLETE_H_ */
