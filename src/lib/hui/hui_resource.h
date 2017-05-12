/*----------------------------------------------------------------------------*\
| Hui resource                                                                 |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last update: 2011.01.18 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#ifndef _HUI_RESOURCE_EXISTS_
#define _HUI_RESOURCE_EXISTS_

namespace hui
{

//----------------------------------------------------------------------------------
// resource handling

Window *_cdecl HuiCreateResourceDialog(const string &id, Window *root);
Menu *_cdecl HuiCreateResourceMenu(const string &id);



class HuiResource
{
public:
	string type;
	string id;
	string title;
	string tooltip;
	Array<string> options;
	bool enabled;
	int x, y, w, h;
	int page;
	string image;
	Array<HuiResource> children;
	void reset();
	HuiResource* get_node(const string &id) const;
	void load(const string &buffer);
	void show(int indent = 0);
};



// resources
void _cdecl HuiLoadResource(const string &filename);
HuiResource *HuiGetResource(const string &id);

};

#endif
