/*----------------------------------------------------------------------------*\
| Hui menu                                                                   |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last update: 2010.01.31 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#include "hui.h"
#include "Controls/HuiMenuItemSubmenu.h"


// stupid function for HuiBui....
void HuiMenu::SetID(const string &id)
{
}

HuiMenu *HuiMenu::GetSubMenuByID(const string &id)
{
	foreach(HuiControl *c, item){
		HuiMenuItemSubmenu *s = dynamic_cast<HuiMenuItemSubmenu*>(c);
		if (s){
			if (s->id == id)
				return s->sub_menu;
			HuiMenu *m = s->sub_menu->GetSubMenuByID(id);
			if (m)
				return m;
		}
	}
	return NULL;
}


void HuiMenu::UpdateLanguage()
{
	msg_db_r("UpdateMenuLanguage", 1);
#if 0
	foreach(HuiMenuItem &it, item){
		if (it.sub_menu)
			it.sub_menu->UpdateLanguage();
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
	msg_db_l(1);
}

Array<HuiControl*> HuiMenu::get_all_controls()
{
	Array<HuiControl*> list = item;
	foreach(HuiControl *c, item){
		HuiMenuItemSubmenu *s = dynamic_cast<HuiMenuItemSubmenu*>(c);
		if (s)
			list.append(s->sub_menu->get_all_controls());
	}
	return list;
}

HuiMenu *HuiCreateMenu()
{
	return new HuiMenu();
}
