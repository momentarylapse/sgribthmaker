/*----------------------------------------------------------------------------*\
| Hui menu                                                                   |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last update: 2010.01.31 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#include "hui.h"
#include "Controls/HuiMenuItem.h"
#include "Controls/HuiMenuItemToggle.h"
#include "Controls/HuiMenuItemSubmenu.h"
#include "Controls/HuiMenuItemSeparator.h"

namespace hui
{

void Menu::__init__()
{
	new(this) Menu;
}

void Menu::__delete__()
{
	this->Menu::~Menu();
}

void Menu::clear()
{
	for (Control *c: item)
		delete(c);
	item.clear();
}

void Menu::addItem(const string &name, const string &id)
{
	add(new HuiMenuItem(name, id));
}

void Menu::addItemImage(const string &name, const string &image, const string &id)
{
	add(new HuiMenuItem(name, id));
	item.back()->setImage(image);
}

void Menu::addItemCheckable(const string &name, const string &id)
{
	add(new HuiMenuItemToggle(name, id));
}

void Menu::addSeparator()
{
	add(new HuiMenuItemSeparator());
}

void Menu::addSubMenu(const string &name, const string &id, Menu *menu)
{
	if (menu)
		add(new HuiMenuItemSubmenu(name, menu, id));
}

void Menu::set_panel(Panel *_panel)
{
	panel = _panel;
	for (Control *c: item){
		c->panel = panel;
		HuiMenuItemSubmenu *s = dynamic_cast<HuiMenuItemSubmenu*>(c);
		if (s)
			s->sub_menu->set_panel(panel);
	}
}

// only allow menu callback, if we are in layer 0 (if we don't edit it ourself)
int allow_signal_level = 0;

// stupid function for HuiBui....
void Menu::setID(const string &id)
{
}

Menu *Menu::getSubMenuByID(const string &id)
{
	for (Control *c: item){
		HuiMenuItemSubmenu *s = dynamic_cast<HuiMenuItemSubmenu*>(c);
		if (s){
			if (s->id == id)
				return s->sub_menu;
			Menu *m = s->sub_menu->getSubMenuByID(id);
			if (m)
				return m;
		}
	}
	return NULL;
}


void Menu::updateLanguage()
{
#if 0
	foreach(HuiMenuItem &it, item){
		if (it.sub_menu)
			it.sub_menu->updateLanguage();
		if ((it.id.num == 0) || (it.is_separator))
			continue;
		bool enabled = it.enabled;
		/*  TODO
		#ifdef HUI_API_WIN
			if (strlen(get_lang(it.ID, "", true)) > 0){
				strcpy(it.Name, HuiGetLanguage(it.ID));
				ModifyMenu(m->hMenu, i, MF_STRING | MF_BYPOSITION, it.ID, get_lang_sys(it.ID, "", true));
			}
		#endif
		*/
		string s = get_lang(it.id, "", true);
		if (s.num > 0)
			SetText(it.id, s);
		msg_todo("HuiUpdateMenuLanguage (GTK) (menu bar)");
			//gtk_menu_item_set_label(GTK_MENU_ITEM(it.g_item), get_lang_sys(it.ID, "", true));
		EnableItem(it.id, enabled);
	}
#endif
}

Array<Control*> Menu::get_all_controls()
{
	Array<Control*> list = item;
	for (Control *c: item){
		HuiMenuItemSubmenu *s = dynamic_cast<HuiMenuItemSubmenu*>(c);
		if (s)
			list.append(s->sub_menu->get_all_controls());
	}
	return list;
}


void Menu::enable(const string &id, bool enabled)
{
	for (Control *c: item){
		if (c->id == id)
			c->enable(enabled);
		if (c->type == HUI_KIND_MENU_SUBMENU)
			dynamic_cast<HuiMenuItemSubmenu*>(c)->sub_menu->enable(id, enabled);
	}
}

};
