//
// Created by michi on 3/26/24.
//

#ifndef SGRIBTHMAKER_OPENFILELIST_H
#define SGRIBTHMAKER_OPENFILELIST_H

#include "lib/hui/hui.h"
#include "lib/pattern/Observable.h"

class FileBrowser : public obs::Node<VirtualBase> {
public:
	FileBrowser(hui::Window *w);

	obs::xsource<Path> out_file_clicked{this, "file-clicked"};

	void on_draw(Painter *p);
	void on_mouse_wheel();
	void on_left_button_down();
	void on_mouse_move();
	void on_mouse_leave();
	void set_directory(const Path& dir);
	void update();

	hui::Window *window;
	string id;
	Path directory;
	float y_max;
	float viewport_y;
	float width, height;

	struct FileItem {
		Path path;
		bool is_directory;
		bool expanded = false;
		Array<FileItem> children;
		vec2 pos = {-999, -999};
	} content;

	FileItem* hover = nullptr;

	void draw_children(Painter *p, FileItem& f, float x, float& y);
};

#endif //SGRIBTHMAKER_OPENFILELIST_H
