
#include "dict.h"
#include "lib.h"
#include "list.h"
#include "../kaba.h"
#include "../dynamic/exception.h"
#include "../dynamic/dynamic.h"
#include "../../base/map.h"

namespace kaba {

extern const Class *TypeDictBase;
extern const Class *TypeIntDict;
extern const Class *TypeFloatDict;
extern const Class *TypeStringDict;

#pragma GCC push_options
#pragma GCC optimize("no-omit-frame-pointer")
#pragma GCC optimize("no-inline")
#pragma GCC optimize("0")

class IntDict : public base::map<string,int> {
public:
	void set_int(const string &k, int v)
	{ set(k, v); }
	int get_int(const string &k)
	{ KABA_EXCEPTION_WRAPPER(return (*this)[k]); return 0; }
	void assign(const IntDict &o)
	{ *this = o; }
//	string str()
//	{ return var2str(this, TypeIntDict); }
};

class FloatDict : public base::map<string,float> {
public:
	void set_float(const string &k, float v)
	{ set(k, v); }
	float get_float(const string &k)
	{ KABA_EXCEPTION_WRAPPER(return (*this)[k]); return 0.0f; }
//	string str()
//	{ return var2str(this, TypeFloatDict); }
};

class StringDict : public base::map<string,string> {
public:
	string get_string(const string &k)
	{ KABA_EXCEPTION_WRAPPER(return (*this)[k]); return ""; }
	void assign(const StringDict &o)
	{ *this = o; }
//	string str()
//	{ return var2str(this, TypeStringDict); }
};
#pragma GCC pop_options

void kaba_make_dict(Class *t, SyntaxTree *ps) {
	const Class *p = t->param[0];
	t->derive_from(TypeDictBase, DeriveFlags::SET_SIZE);
	t->param = {p};
	add_class(t);

	if (p->can_memcpy()) {
		// elements don't need a destructor
		class_add_func(Identifier::Func::DELETE, TypeVoid, &base::map<string,int>::clear);
		class_add_func("clear", TypeVoid, &base::map<string,int>::clear);
		class_add_func(Identifier::Func::ASSIGN, TypeVoid, &IntDict::assign);
			func_add_param("other", t);
	}

	if (p == TypeInt) {
		class_add_func(Identifier::Func::INIT, TypeVoid, &XDict<int>::__init__);
		class_add_func(Identifier::Func::SET, TypeVoid, &IntDict::set_int);
			func_add_param("key", TypeString);
			func_add_param("x", p);
		class_add_func(Identifier::Func::GET, p, &IntDict::get_int, Flags::RAISES_EXCEPTIONS);
			func_add_param("key", TypeString);
//		class_add_func(Identifier::Func::STR, TypeString, &IntDict::str, Flags::PURE);
	} else if (p == TypeFloat32) {
		class_add_func(Identifier::Func::INIT, TypeVoid, &XDict<float>::__init__);
		class_add_func(Identifier::Func::SET, TypeVoid, &FloatDict::set_float);
			func_add_param("key", TypeString);
			func_add_param("x", p);
		class_add_func(Identifier::Func::GET, p, &FloatDict::get_float, Flags::RAISES_EXCEPTIONS);
			func_add_param("key", TypeString);
//		class_add_func(Identifier::Func::STR, TypeString, &FloatDict::str, Flags::PURE);
	} else if (p == TypeString) {
		class_add_func(Identifier::Func::INIT, TypeVoid, &XDict<string>::__init__);
		class_add_func(Identifier::Func::SET, TypeVoid, &base::map<string,string>::set);
			func_add_param("key", TypeString);
			func_add_param("x", p);
		class_add_func(Identifier::Func::GET, p, &StringDict::get_string, Flags::RAISES_EXCEPTIONS);
			func_add_param("key", TypeString);
		class_add_func(Identifier::Func::DELETE, TypeVoid, &base::map<string,string>::clear);
		class_add_func("clear", TypeVoid, &base::map<string,string>::clear);
		class_add_func(Identifier::Func::ASSIGN, TypeVoid, &StringDict::assign);
			func_add_param("other", t);
//		class_add_func(Identifier::Func::STR, TypeString, &StringDict::str, Flags::PURE);
	}
}



}
