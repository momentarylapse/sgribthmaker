/*
 * hui_error.h
 *
 *  Created on: 26.06.2013
 *      Author: michi
 */

#ifndef HUI_ERROR_H_
#define HUI_ERROR_H_

namespace hui
{

// error handling
void HuiSetErrorFunction(const Callback &function);
void HuiSetDefaultErrorHandler(const Callback &error_cleanup_function);
void HuiRaiseError(const string &message);
void HuiSendBugReport(Window *parent);

};

#endif /* HUI_ERROR_H_ */
