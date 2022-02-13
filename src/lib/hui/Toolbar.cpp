/*
 * HuiToolbar.cpp
 *
 *  Created on: 26.06.2013
 *      Author: michi
 */

#include "Toolbar.h"

#include "Controls/ToolItemButton.h"
#include "Controls/ToolItemMenuButton.h"
#include "Controls/ToolItemSeparator.h"
#include "Controls/ToolItemToggleButton.h"

namespace hui
{

Menu *_create_res_menu_(const string &ns, Resource *res, Panel *p);


// add a default button
void Toolbar::add(const string &title, const string &image, const string &id) {
	_add(new ToolItemButton(title, image, id));
}

// add a checkable button
void Toolbar::add_checkable(const string &title, const string &image, const string &id) {
	_add(new ToolItemToggleButton(title, image, id));
}

void Toolbar::add_menu(const string &title, const string &image, Menu *menu, const string &id) {
	if (!menu)
		return;
	_add(new ToolItemMenuButton(title, menu, image, id, win));
	menu->set_panel(win);
}

void Toolbar::add_menu_by_id(const string &title, const string &image, const string &menu_id, const string &id) {
	Menu *menu = create_resource_menu(menu_id, win);
	add_menu(title, image, menu, id);
}

void Toolbar::add_separator() {
	_add(new ToolItemSeparator());
}

// remove all items from the toolbar
void Toolbar::reset() {
	for (int i=0; i<item.num; i++)
		delete item[i];
	item.clear();
}

// create and apply a toolbar bar resource id
void Toolbar::set_by_id(const string &id) {
	Resource *res = get_resource(id);
	if (!res) {
		msg_error("Toolbar.SetByID  :~~(");
		return;
	}
	from_resource(res);
}

void Toolbar::from_resource(Resource *res) {
	reset();
	id = res->id;
	//Configure(res->b_param[0], res->b_param[1]);
	for (Resource &cmd: res->children) {
		string title = get_lang(id, cmd.id, cmd.title, false);
		string tooltip = get_language_t(id, cmd.id, cmd.tooltip);
		if (tooltip.num == 0)
			tooltip = title;

		if (cmd.type == "Item") {
			if (sa_contains(cmd.options, "checkable"))
				add_checkable(title, cmd.image(), cmd.id);
			else
				add(title, cmd.image(), cmd.id);
			item.back()->set_tooltip(tooltip);
		} else if (cmd.type == "Separator") {
			add_separator();
		} else if (cmd.type == "Menu") {
			bool ok = false;
			for (string &o: cmd.options)
				if (o.find("menu=") == 0) {
					add_menu_by_id(title, cmd.image(), o.sub(5), cmd.id);
					item.back()->set_tooltip(get_language_t(id, cmd.id, cmd.tooltip));
					ok = true;
				}
			if ((!ok) and (cmd.children.num > 0)) {
				add_menu(title, cmd.image(), _create_res_menu_(id, &cmd, win), cmd.id);
				item.back()->set_tooltip(get_language_t(id, cmd.id, cmd.tooltip));
			}
		}
		for (auto &o: cmd.options)
			item.back()->set_options(o);
	}
	enable(true);
}

void Toolbar::from_source(const string &source) {
	Resource res = parse_resource(source);
	from_resource(&res);
}


void Toolbar::apply_foreach(const string &id, std::function<void(Control*)> f) {
	for (Control *c: item)
		c->apply_foreach(id, f);

}

};

