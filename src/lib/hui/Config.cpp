/*
 * HuiConfig.cpp
 *
 *  Created on: 01.03.2014
 *      Author: michi
 */

#include "Config.h"
#include "../file/file.h"


string f2s_clean(float f, int dez);

namespace hui
{


#ifdef OS_WINDOWS
	//int _tchar_str_size_(TCHAR *str);
#endif

Configuration config;

Configuration::Configuration() {
	loaded = false;
	changed = false;
}

Configuration::~Configuration() {
}

void Configuration::__init__() {
	new(this) Configuration;
}

void Configuration::__del__() {
	this->~Configuration();
}

void Configuration::set_int(const string& name, int val) {
	set_str(name, i2s(val));
}

void Configuration::set_float(const string& name, float val) {
	set_str(name, f2s_clean(val, 6));
}

void Configuration::set_bool(const string& name, bool val) {
	set_str(name, b2s(val));
}

void Configuration::set_str(const string& name, const string& str) {
	map.set(name, str);
	changed = true;
}

int Configuration::get_int(const string& name, int default_val) const {
	return get_str(name, i2s(default_val))._int();
}

float Configuration::get_float(const string& name, float default_val) const {
	return get_str(name, f2s(default_val, 6))._float();
}

bool Configuration::get_bool(const string& name, bool default_val) const {
	return get_str(name, b2s(default_val))._bool();
}

string Configuration::get_str(const string& name, const string& default_str) const {
	try {
		return map[name];
	} catch(...) {
		return default_str;
	}
}

bool Configuration::has(const string& name) const {
	return map.find(name) >= 0;
}

static string _parse_value(const string &s) {
	string r = s.trim();
	if (r.head(1) == "\"" and r.tail(1) == "\"")
		return r.sub(1, -1).unescape();
	if (r.head(1) == "\'" and r.tail(1) == "\'")
		return r.sub(1, -1).replace("\\\'", "\'");
	return r;
}

bool Configuration::load(const Path &filename) {
	try {
		auto f = new TextLinesFormatter(file_open(filename, "rt"));
		map.clear();

		string t = f->read_str();
		if (t == "// NumConfigs") {
			// old format
			int num = f->read_int();
			for (int i=0;i<num;i++) {
				string temp = f->read_str();
				string key = temp.sub(3);
				string value = f->read_str();
				map.set(key, value);
			}
		} else if (t.head(3) == "// ") {
			// semi old format
			f->stream->set_pos(0);
			while (!f->stream->is_end()) {
				string temp = f->read_str();
				if (temp == "#")
					break;
				string key = temp.sub(3).lower().replace(" ", "-");
				string value = f->read_str();
				map.set(key, value);
			}
		} else {
			// new format
			f->stream->set_pos(0);
			string _namespace;
			while (!f->stream->is_end()) {
				string s = f->read_str();
				if (s.num == 0)
					continue;

				if (s[0] == '#') {
					comments.add(s);
				} else if (s[0] == '[') {
					_namespace = s.replace("[", "").replace("]", "").trim();
				} else {
					int p = s.find("=");
					if (p >= 0) {
						string key = s.head(p).trim();
						if (_namespace.num > 0)
							key = _namespace + "." + key;
						string value = _parse_value(s.sub(p+1));
						map.set(key, value);
					}
				}
			}
		}
//		for (auto &k: map.keys())
//			msg_write("config:  " + k + " := " + map[k]);
		delete f;
		loaded = true;
		changed = false;
		return true;
	} catch(Exception &e) {
		return false;
	}
}

bool string_needs_quotes(const string &v) {
	if (v.num == 0)
		return false;

	if (v.head(1) == " " or v.tail(1) == " ")
		return true;
	if (v.has_char('\"') or v.has_char('\t') or v.has_char('\n'))
		return true;
	return false;
}

string str_simple_repr(const string &v) {
	if (!string_needs_quotes(v))
		return v;
	int mode = 2;
	if (v.has_char('\t') or v.has_char('\n'))
		mode = 1;
	if (v.has_char('\''))
		mode = 1;

	if (mode == 1)
		return v.repr();
	return "'" + v.replace("'", "\\'") + "'";
}

string config_get_namespace(const string &key) {
	int p = key.rfind(".");
	if (p >= 0)
		return key.sub_ref(0, p);
	return "";
}

string config_get_base(const string &key) {
	int p = key.rfind(".");
	if (p >= 0)
		return key.sub_ref(p + 1);
	return key;
}

void Configuration::save(const Path &filename) {
	dir_create(filename.parent());
	try {
		auto f = new TextLinesFormatter(file_open(filename, "wt"));
		Set<string> namespaces;
		for (auto &e: map)
			namespaces.add(config_get_namespace(e.key));
		for (auto &e: map)
			if (config_get_namespace(e.key) == "")
				f->write_str(format("%s = %s", e.key, str_simple_repr(e.value)));
		for (auto &n: namespaces)
			if (n.num > 0) {
				f->write_str(format("\n[%s]", n));
				for (auto &e: map)
					if (config_get_namespace(e.key) == n)
						f->write_str(format("\t%s = %s", config_get_base(e.key), str_simple_repr(e.value)));
			}

		if (comments.num > 0)
			f->write_str("");
		for (auto &s: comments)
			f->write_str(s);
		delete f;
		loaded = true;
		changed = false;
	} catch(Exception &e) {
		msg_error(e.message());
	}
}


Array<string> Configuration::keys() const {
	return map.keys();
}



};

