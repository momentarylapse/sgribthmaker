/*----------------------------------------------------------------------------*\
| Kaba Lib                                                                     |
| -> "standart library" for the scripting system                               |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last updated: 2010.07.07 (c) by MichiSoft TM                                 |
\*----------------------------------------------------------------------------*/
#pragma once

#include "../CompilerConfiguration.h"
#include "../syntax/Flags.h"
#include "extern.h"
#include "../../base/pointer.h"
#include <cstddef>
#include <type_traits>

namespace kaba {


#define MAX_OPCODE				(2*65536)	// max. amount of opcode


class SyntaxTree;
class Module;
class Class;
class Value;
class Function;
class Variable;
class Constant;
class Context;
enum class DeriveFlags;


void kaba_make_super_array(Class *t, SyntaxTree *ps = nullptr);
void kaba_make_dict(Class *t, SyntaxTree *ps = nullptr);





//--------------------------------------------------------------------------------------------------
// type casting

class TypeCast {
public:
	int penalty;
	const Class *source, *dest;
	Function *f;
};


typedef void t_func();



void init(Abi abi = Abi::NATIVE, bool allow_std_lib = true);
void clean_up();





template<class C, class M>
int element_offset(M C::* p) {
	extern char _el_off_data[];
	auto *c = reinterpret_cast<C*>(&_el_off_data[0]);
	return (int_p)((char*)&(c->*p) - (char*)c);
	//return *(int*)(void*)&p;
}

void add_package(Context *c, const string &name, Flags = Flags::NONE);
const Class *add_type(const string &name, int size, Flags = Flags::NONE, const Class *_namespace = nullptr);
const Class *add_type_p(const Class *sub_type, Flags = Flags::NONE);
const Class *add_type_ref(const Class *sub_type);
const Class *add_type_xfer(const Class *sub_type);
const Class *add_type_a(const Class *sub_type, int array_length);
const Class *add_type_l(const Class *sub_type);
const Class *add_type_d(const Class *sub_type);
const Class *add_type_f(const Class *ret_type, const Array<const Class*> &params);
const Class *add_type_e(const string &name, const Class *_namespace = nullptr);
void capture_implicit_type(const Class *t, const string &name);


Function *add_func_x(const string &name, const Class *return_type, void *func, Flags flag = Flags::NONE);
// version: regular function
template<class T>
Function *add_func(const string &name, const Class *return_type, T func, Flags flag = Flags::NONE) {
	return add_func_x(name, return_type, (void*)func, flag);
}

void func_set_inline(InlineID index);
void func_add_param(const string &name, const Class *type, Flags flags = Flags::CONST);
void func_add_param_def_x(const string &name, const Class *type, const void *p, Flags flags = Flags::CONST);
template<class T>
void func_add_param_def(const string &name, const Class *type, T p, Flags flags = Flags::CONST) {
	func_add_param_def_x(name, type, &p, flags);
}
Class *add_class(const Class *root_type);
void class_add_element_x(const string &name, const Class *type, int offset, Flags flag = Flags::NONE);
template<class T>
void class_add_element(const string &name, const Class *type, T p, Flags flag = Flags::NONE) {
	// allows &Class::element
	if constexpr (std::is_integral<T>::value)
		class_add_element_x(name, type, p, flag);
	else if constexpr (std::is_same<T, std::nullptr_t>::value)
		class_add_element_x(name, type, 0, flag);
	else
		class_add_element_x(name, type, element_offset(p), flag);
}



Function* class_add_func_x(const string &name, const Class *return_type, void *func, Flags = Flags::NONE);
// version: null
Function* class_add_func(const string &name, const Class *return_type, std::nullptr_t func, Flags flag = Flags::NONE);
// version: regular function
template <typename R, typename ...Args>
Function* class_add_func(const string &name, const Class *return_type, R (*func)(Args...), Flags flag = Flags::NONE) {
	return class_add_func_x(name, return_type, (void*)func, flag);
}
// version: member function
template <typename T, typename R, typename ...Args>
Function* class_add_func(const string &name, const Class *return_type, R (T::*func)(Args...), Flags flag = Flags::NONE) {
	return class_add_func_x(name, return_type, mf(func), flag);
}
// version: const member function
template <typename T, typename R, typename ...Args>
Function* class_add_func(const string &name, const Class *return_type, R (T::*func)(Args...) const, Flags flag = Flags::NONE) {
	return class_add_func_x(name, return_type, mf(func), flag);
}





Function* class_add_func_virtual_x(const string &name, const Class *return_type, void *func, Flags = Flags::NONE);
template<class T>
Function* class_add_func_virtual(const string &name, const Class *return_type, T func, Flags flag = Flags::NONE) {
	return class_add_func_virtual_x(name, return_type, mf(func), flag);
}

void class_link_vtable(void *p);
void class_derive_from(const Class *parent, DeriveFlags flagse = (DeriveFlags)0);
void add_const(const string &name, const Class *type, const void *value);
void class_add_const(const string &name, const Class *type, const void *value);
template<class T>
void add_enum(const string &name, const Class *type, T e) {
	// for enums and nullptr!
	add_const(name, type, (const void*)(int_p)e);
}
template<class T>
void class_add_enum(const string &name, const Class *type, T e) {
	// for enums and nullptr!
	class_add_const(name, type, (const void*)(int_p)e);
}


void add_ext_var(const string &name, const Class *type, void *var);
void add_type_cast(int penalty, const Class *source, const Class *dest, const string &cmd);



void add_operator_x(OperatorID primitive_op, const Class *return_type, const Class *param_type1, const Class *param_type2, InlineID inline_index, void *func = nullptr);
// version: no function
void add_operator(OperatorID primitive_op, const Class *return_type, const Class *param_type1, const Class *param_type2, InlineID inline_index);
void add_operator(OperatorID primitive_op, const Class* return_type, const Class* param_type1, const Class* param_type2, InlineID inline_index, std::nullptr_t func);
// version: regular function
template <typename R, typename ...Args>
void add_operator(OperatorID primitive_op, const Class *return_type, const Class *param_type1, const Class *param_type2, InlineID inline_index, R (*func)(Args...)) {
	add_operator_x(primitive_op, return_type, param_type1, param_type2, inline_index, (void*)func);
}
// version: member function
template <typename T, typename R, typename ...Args>
void add_operator(OperatorID primitive_op, const Class *return_type, const Class *param_type1, const Class *param_type2, InlineID inline_index, R (T::*func)(Args...)) {
	add_operator_x(primitive_op, return_type, param_type1, param_type2, inline_index, mf(func));
}
// version: const member function
template <typename T, typename R, typename ...Args>
void add_operator(OperatorID primitive_op, const Class *return_type, const Class *param_type1, const Class *param_type2, InlineID inline_index, R (T::*func)(Args...) const) {
	add_operator_x(primitive_op, return_type, param_type1, param_type2, inline_index, mf(func));
}


#define class_set_vtable(TYPE) \
	{TYPE my_instance; \
	class_link_vtable(*(void***)&my_instance);}






};

