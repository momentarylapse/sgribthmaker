/*
 * SgribthMaker.h
 *
 *  Created on: 14.10.2017
 *      Author: michi
 */

#ifndef SRC_SGRIBTHMAKER_H_
#define SRC_SGRIBTHMAKER_H_


#include "lib/hui/hui.h"

class SgribthMakerWindow;


class SgribthMaker : public hui::Application {
public:
	SgribthMaker();
	hui::AppStatus on_startup(const Array<string> &arg) override;

	SgribthMakerWindow* win;
};

extern SgribthMaker* sgribthmaker;

#endif /* SRC_SGRIBTHMAKER_H_ */
