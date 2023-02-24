#include "../kaba.h"
#include "implicit.h"
#include "../asm/asm.h"
#include "../parser/Parser.h"
#include "../syntax/SyntaxTree.h"
#include <stdio.h>
#include "../../os/msg.h"
#include "../../base/iter.h"

namespace kaba {


const int FULL_CONSTRUCTOR_MAX_PARAMS = 4;


AutoImplementer::AutoImplementer(Parser *p, SyntaxTree *t) {
	parser = p;
	tree = t;
}

shared<Node> AutoImplementer::node_false() {
	auto c = tree->add_constant(TypeBool);
	c->as_int() = 0;
	return add_node_const(c);
}

shared<Node> AutoImplementer::node_true() {
	auto c = tree->add_constant(TypeBool);
	c->as_int() = 1;
	return add_node_const(c);
}

shared<Node> AutoImplementer::node_nil() {
	auto c = tree->add_constant(TypePointer);
	c->as_int64() = 0;
	return add_node_const(c);
}

shared<Node> AutoImplementer::const_int(int i) {
	return add_node_const(tree->add_constant_int(i));
}

void AutoImplementer::db_add_print_node(shared<Block> block, shared<Node> node) {
	auto ff = tree->required_func_global("print");
	auto cmd = add_node_call(ff);
	cmd->set_param(0, parser->con.add_converter_str(node, false));
	block->add(cmd);
}

void AutoImplementer::db_add_print_label(shared<Block> block, const string &s) {
	auto c = tree->add_constant(TypeString);
	c->as_string() = s;
	db_add_print_node(block, add_node_const(c));
}

void AutoImplementer::db_add_print_label_node(shared<Block> block, const string &s, shared<Node> node) {
	auto c = tree->add_constant(TypeString);
	c->as_string() = s;

	auto ff = tree->required_func_global("print");
	auto cmd = add_node_call(ff);
	cmd->set_param(0, parser->con.link_operator_id(OperatorID::ADD, add_node_const(c), parser->con.add_converter_str(node, false)));
	block->add(cmd);
}



void AutoImplementer::do_error_implicit(Function *f, const string &str) {
	int token_id = max(f->token_id, f->name_space->token_id);
	tree->do_error(format("[auto generating %s] : %s", f->signature(), str), token_id);
}





// skip the "self" parameter!
Function *AutoImplementer::add_func_header(Class *t, const string &name, const Class *return_type, const Array<const Class*> &param_types, const Array<string> &param_names, Function *cf, Flags flags, const shared_array<Node> &def_params) {
	Function *f = tree->add_function(name, return_type, t, flags); // always member-function??? no...?
	f->auto_declared = true;
	f->token_id = t->token_id;
	for (auto&& [i,p]: enumerate(param_types)) {
		f->literal_param_type.add(p);
		auto v = f->block->add_var(param_names[i], p);
		flags_set(v->flags, Flags::CONST);
		f->num_params ++;
	}
	f->default_parameters = def_params;
	f->update_parameters_after_parsing();
	if (config.verbose)
		msg_write("ADD HEADER " + f->signature(TypeVoid));

	bool override = cf;
	t->add_function(tree, f, false, override);
	return f;
}

bool AutoImplementer::needs_new(Function *f) {
	if (!f)
		return true;
	return f->needs_overriding();
}

Array<string> AutoImplementer::class_func_param_names(Function *cf) {
	Array<string> names;
	auto *f = cf;
	for (int i=0; i<f->num_params; i++)
		names.add(f->var[i]->name);
	return names;
}

bool AutoImplementer::has_user_constructors(const Class *t) {
	for (auto *cc: t->get_constructors())
		if (!cc->needs_overriding())
			return true;
	return false;
}

void AutoImplementer::remove_inherited_constructors(Class *t) {
	for (int i=t->functions.num-1; i>=0; i--)
		if (t->functions[i]->name == Identifier::Func::INIT and t->functions[i]->needs_overriding())
			t->functions.erase(i);
}

void AutoImplementer::redefine_inherited_constructors(Class *t) {
	for (auto *pcc: t->parent->get_constructors()) {
		auto c = t->get_same_func(Identifier::Func::INIT, pcc);
		if (needs_new(c)) {
			add_func_header(t, Identifier::Func::INIT, TypeVoid, pcc->literal_param_type, class_func_param_names(pcc), c, Flags::NONE, pcc->default_parameters);
		}
	}
}

void AutoImplementer::add_full_constructor(Class *t) {
	Array<string> names;
	Array<const Class*> types;
	for (auto &e: t->elements) {
		if (!e.hidden()) {
			names.add(e.name);
			types.add(e.type);
		}
	}
	auto f = add_func_header(t, Identifier::Func::INIT, TypeVoid, types, names);
	flags_set(f->flags, Flags::__INIT_FILL_ALL_PARAMS);
}

bool AutoImplementer::class_can_fully_construct(const Class *t) {
	if (t->vtable.num > 0)
		return false;
	if (t->elements.num > FULL_CONSTRUCTOR_MAX_PARAMS)
		return false;
	int num_el = 0;
	for (auto &e: t->elements) {
		if (e.hidden())
			continue;
		if (!e.type->get_assign() and e.type->uses_call_by_reference()) {
			msg_write(format("class %s auto constructor prevented by element %s %s", t->name, e.name, e.type->name));
			return false;
		}
		num_el ++;
	}
	return num_el > 0;
}

bool AutoImplementer::class_can_default_construct(const Class *t) {
	if (!t->needs_constructor())
		return true;
	if (t->get_default_constructor())
		return true;
	if (t->is_struct() and !flags_has(t->flags, Flags::NOAUTO))
		return true;
	if (t->is_array())
		return class_can_default_construct(t->param[0]);
	return false;
}

bool AutoImplementer::class_can_destruct(const Class *t) {
	if (!t->needs_destructor())
		return true;
	if (t->get_destructor())
		return true;
	if (t->is_struct() and !flags_has(t->flags, Flags::NOAUTO))
		return true;
	if (t->is_array())
		return class_can_destruct(t->param[0]);
	return false;
}

bool AutoImplementer::class_can_assign(const Class *t) {
	if (t->is_pointer_raw() or t->is_reference())
		return true;
	if (t->get_assign())
		return true;
	if (t->is_struct() and !flags_has(t->flags, Flags::NOAUTO))
		return true;
	if (t->is_array())
		return class_can_assign(t->param[0]);
	return false;
}

// _should_ we make it possible to assign?
bool AutoImplementer::class_can_elements_assign(const Class *t) {
	for (auto &e: t->elements)
		if (!e.hidden())
			if (!class_can_assign(e.type))
				return false;
	return true;
}

bool AutoImplementer::class_can_equal(const Class *t) {
	if (t->is_pointer_raw() or t->is_reference())
		return true;
	if (t->get_member_func(Identifier::Func::EQUAL, TypeBool, {t}))
		return true;
	return false;
}

void AutoImplementer::add_missing_function_headers_for_class(Class *t) {
	if (t->owner != tree)
		return;
	if (t->is_pointer_raw() or t->is_reference())
		return;

	if (t->is_super_array()) {
		_add_missing_function_headers_for_super_array(t);
	} else if (t->is_array()) {
		_add_missing_function_headers_for_array(t);
	} else if (t->is_dict()) {
		_add_missing_function_headers_for_dict(t);
	} else if (t->is_pointer_shared()) {
		_add_missing_function_headers_for_shared(t);
	} else if (t->is_pointer_owned() or t->is_pointer_owned_not_null()) {
		_add_missing_function_headers_for_owned(t);
	} else if (t->is_pointer_xfer()) {
		_add_missing_function_headers_for_xfer(t);
	} else if (t->is_product()) {
		_add_missing_function_headers_for_product(t);
	} else if (t->is_callable_fp()) {
		_add_missing_function_headers_for_callable_fp(t);
	} else if (t->is_callable_bind()) {
		_add_missing_function_headers_for_callable_bind(t);
	} else if (t->is_enum()) {
		_add_missing_function_headers_for_enum(t);
	} else if (t->is_optional()) {
		_add_missing_function_headers_for_optional(t);
	} else { // regular classes
		_add_missing_function_headers_for_regular(t);
	}
}

Function* AutoImplementer::prepare_auto_impl(const Class *t, Function *f) {
	if (!f)
		return nullptr;
	if (!f->auto_declared)
		return nullptr;
	flags_clear(f->flags, Flags::NEEDS_OVERRIDE); // we're about to implement....
	return f;
}

// completely create and implement
void AutoImplementer::implement_functions(const Class *t) {
	if (t->owner != tree)
		return;
	if (t->is_pointer_raw() or t->is_reference())
		return;

	auto sub_classes = t->classes; // might change

	if (t->is_super_array()) {
		_implement_functions_for_super_array(t);
	} else if (t->is_array()) {
		_implement_functions_for_array(t);
	} else if (t->is_dict()) {
		_implement_functions_for_dict(t);
	} else if (t->is_pointer_shared()) {
		_implement_functions_for_shared(t);
	} else if (t->is_pointer_owned() or t->is_pointer_owned_not_null()) {
		_implement_functions_for_owned(t);
	} else if (t->is_pointer_xfer()) {
		_implement_functions_for_xfer(t);
	} else if (t->is_enum()) {
		_implement_functions_for_enum(t);
	} else if (t->is_callable_fp()) {
		_implement_functions_for_callable_fp(t);
	} else if (t->is_callable_bind()) {
		_implement_functions_for_callable_bind(t);
	} else if (t->is_optional()) {
		_implement_functions_for_optional(t);
	} else if (t->is_product()) {
		_implement_functions_for_product(t);
	} else {
		// regular
		_implement_functions_for_regular(t);
	}

	// recursion
	//for (auto *c: sub_classes)
	//	implement_functions(c);
}

extern const Class* TypeDynamicArray;
extern const Class* TypeCallableBase;

void AutoImplementer::complete_type(Class *t, int array_size, int token_id) {

	auto params = t->param;
	// ->derive_from() will overwrite params!!!

	t->array_length = max(array_size, 0);
	if (t->is_super_array() or t->is_dict()) {
		t->derive_from(TypeDynamicArray); // we already set its size!
		if (!class_can_default_construct(params[0]))
			tree->do_error(format("can not create a dynamic array from type '%s', missing default constructor", params[0]->long_name()), token_id);
		t->param = params;
		add_missing_function_headers_for_class(t);
	} else if (t->is_array()) {
		if (!class_can_default_construct(params[0]))
			tree->do_error(format("can not create an array from type '%s', missing default constructor", params[0]->long_name()), token_id);
		t->param = params;
		add_missing_function_headers_for_class(t);
	} else if (t->is_pointer_raw()) {
		flags_set(t->flags, Flags::FORCE_CALL_BY_VALUE);
	} else if (t->is_reference()) {
		flags_set(t->flags, Flags::FORCE_CALL_BY_VALUE);
	} else if (t->is_pointer_xfer()) {
		flags_set(t->flags, Flags::FORCE_CALL_BY_VALUE);
	} else if (t->is_pointer_shared()) {
		//t->derive_from(TypeSharedPointer);
		//flags_set(t->flags, Flags::FORCE_CALL_BY_VALUE);
		t->param = params;
		add_missing_function_headers_for_class(t);
	} else if (t->is_pointer_owned() or t->is_pointer_owned_not_null()) {
		flags_set(t->flags, Flags::FORCE_CALL_BY_VALUE);
		t->param = params;
		add_missing_function_headers_for_class(t);
	} else if (t->is_optional()) {
		if (!class_can_default_construct(params[0]))
			tree->do_error(format("can not create an optional from type '%s', missing default constructor", params[0]->long_name()), token_id);
		add_missing_function_headers_for_class(t);
	} else if (t->type == Class::Type::FUNCTION) {
		t->derive_from(TypeFunction);
		t->param = params;
	} else if (t->is_callable_fp()) {
		t->derive_from(TypeCallableBase);
		t->functions.clear(); // don't inherit call() with specific types!
		t->param = params;
		add_missing_function_headers_for_class(t);
	} else if (t->is_callable_bind()) {
		t->derive_from(TypeCallableBase);
		t->functions.clear(); // don't inherit call() with specific types!
		t->param = params;
		//add_missing_function_headers_for_class(t); // later... depending on the bind variables
	} else if (t->is_product()) {
		int offset = 0;
		for (auto&& [i,cc]: enumerate(params)) {
			t->elements.add(ClassElement(format("e%d", i), cc, offset));
			offset += cc->size;
		}
		add_missing_function_headers_for_class(t);
	} else if (t->is_enum()) {
		t->flags = Flags::FORCE_CALL_BY_VALUE; // FORCE_CALL_BY_VALUE
		kaba::add_class(t);
		//add_missing_function_headers_for_class(t); // later... might need to parse @noauto first
	}
}


}
