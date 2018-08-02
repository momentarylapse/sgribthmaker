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

	struct Data
	{
		Array<string> suggestions;
		int offset;
	};

	static Data run(const string &code, int line, int pos);
};

#endif /* SRC_AUTOCOMPLETE_H_ */
