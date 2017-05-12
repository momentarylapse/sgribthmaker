/*
 * clipboard.h
 *
 *  Created on: 26.06.2013
 *      Author: michi
 */

#ifndef CLIPBOARD_H_
#define CLIPBOARD_H_

namespace hui
{

// clipboard
void _cdecl CopyToClipBoard(const string &buffer);
string _cdecl PasteFromClipBoard();

}


#endif /* CLIPBOARD_H_ */
