#include "FileBrowser.h"
#include "lib/os/filesystem.h"
#include "lib/math/rect.h"
#include "lib/image/color.h"

static float ITEM_HEIGHT = 28;

FileBrowser::FileBrowser(hui::Window *w) {
	window = w;
	id = "file-browser";

	window->event_xp(id, hui::EventID::DRAW, [this] (Painter* p) { on_draw(p); });
	window->event_x(id, hui::EventID::MOUSE_WHEEL, [this] { on_mouse_wheel(); });
	window->event_x(id, hui::EventID::LEFT_BUTTON_DOWN, [this] { on_left_button_down(); });
	window->event_x(id, hui::EventID::MOUSE_LEAVE, [this] { on_mouse_leave(); });
	window->event_x(id, hui::EventID::MOUSE_MOVE, [this] { on_mouse_move(); });
}

void FileBrowser::set_directory(const Path &dir) {
	directory = dir;
	update();
}

static void find_children(FileBrowser::FileItem& item, const Path& directory, int level) {
	auto is_hidden = [] (const Path& p) {
		return str(p)[0] == '.';
	};
	for (auto& f: os::fs::search(directory, "*", "d")) {
		if (!is_hidden(f)) {
			item.children.add({directory| f, true});
			//if (level < 1)
				find_children(item.children.back(), directory | f, level + 1);
		}
	}
	for (auto& f: os::fs::search(directory, "*", "f")) {
		if (!is_hidden(f))
			item.children.add({directory| f, false});
	}
};

void FileBrowser::update() {
	content = {};
	find_children(content, directory, 0);
	viewport_y = 0;
}

void FileBrowser::draw_children(Painter *p, FileItem& f, float x, float& y) {
	static color col = color(1, 0.8f, 0.8f, 0.8f);

	for (auto& d: f.children) {
		float yy = y - viewport_y;
		if (&d == hover) {
			p->set_color(color(0.2, 0.3f, 0.3f, 1));
			p->draw_rect({0, width, yy, yy + ITEM_HEIGHT});

		}
		float R = 4;
		if (d.is_directory) {
			p->set_color(col);
			if (d.expanded)
				p->draw_str({x - 14, yy + 8}, u8"▾");
			else
				p->draw_str({x - 14, yy + 8}, u8"▸");
			p->set_color(color(1, 0.7f, 0.7f, 0.7f));
			p->draw_rect(rect(x + 12 - R, x + 12 + R, yy + ITEM_HEIGHT/2 - R, yy + ITEM_HEIGHT/2 + R));
		} else {
			p->set_color(color(1, 0.6f, 0.4f, 0.1f));
			p->draw_rect(rect(x + 12 - R, x + 12 + R, yy + ITEM_HEIGHT/2 - R, yy + ITEM_HEIGHT/2 + R));
		}
		p->set_color(col);
		p->draw_str({x + 25, yy + 8}, d.path.basename());
		d.pos = {x, y};
		y += ITEM_HEIGHT;

		if (d.is_directory and d.expanded)
			draw_children(p, d, x + 25, y);
	}
};

void FileBrowser::on_draw(Painter *p) {
	p->set_color(Black);
	width = p->width;
	height = p->height;

	p->set_font_size(13);
	float y = 0;
	draw_children(p, content, 30, y);
	y_max = y;
}

void FileBrowser::on_mouse_wheel() {
	viewport_y = clamp(viewport_y + hui::get_event()->scroll.y * 3, 0.0f, y_max - height);
	window->redraw(id);
}

static FileBrowser::FileItem* get_hover_child(const FileBrowser::FileItem& item, const vec2& m) {
	for (auto& c: item.children) {
		if (m.y >= c.pos.y and m.y < c.pos.y + ITEM_HEIGHT)
			return &c;
		if (c.is_directory and c.expanded)
			if (auto r = get_hover_child(c, m))
				return r;
	}
	return nullptr;
}

void FileBrowser::on_left_button_down() {
	vec2 m = hui::get_event()->m;
	hover = get_hover_child(content, m);
	if (hover) {
		if (hover->is_directory)
			hover->expanded = !hover->expanded;
		else
			out_file_clicked(hover->path);
	}
	window->redraw(id);
}

void FileBrowser::on_mouse_move() {
	vec2 m = hui::get_event()->m;
	hover = get_hover_child(content, m);
	window->redraw(id);
}

void FileBrowser::on_mouse_leave() {
	hover = nullptr;
	window->redraw(id);
}
