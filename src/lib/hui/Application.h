/*
 * HuiApplication.h
 *
 *  Created on: 13.07.2014
 *      Author: michi
 */

#ifndef HUIAPPLICATION_H_
#define HUIAPPLICATION_H_

#include "../base/base.h"
#include "hui_common.h"

namespace hui
{

enum
{
	HUI_FLAG_LOAD_RESOURCE = 1,
	HUI_FLAG_SILENT = 2,
	HUI_FLAG_UNIQUE = 16,
};

class HuiApplication : public HuiEventHandler
{
public:
	HuiApplication(const string &app_name, const string &def_lang, int flags);
	virtual ~HuiApplication();

	virtual bool onStartup(const Array<string> &arg) = 0;

	int run();
};

}

#define HUI_EXECUTE(APP_CLASS) \
int hui_main(const Array<string> &arg) \
{ \
	APP_CLASS *app = new APP_CLASS; \
	int r = 0; \
	if (app->onStartup(arg)) \
		r = app->run(); \
	delete(app); \
	return r; \
}

#endif /* HUIAPPLICATION_H_ */
