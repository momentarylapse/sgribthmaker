/*
 * Block.cpp
 *
 *  Created on: May 9, 2021
 *      Author: michi
 */

#include "Block.h"
#include "Node.h"
#include "Function.h"
#include "Flags.h"
#include "Variable.h"
#include "SyntaxTree.h"

namespace kaba {

Block::Block(Function *f, Block *_parent, const Class *t) :
	Node(NodeKind::Block, (int_p)this, t)
{
	level = 0;
	function = f;
	parent = _parent;
	if (parent)
		level = parent->level + 1;
	_start = _end = nullptr;
	_label_start = _label_end = -1;
}


void Block::add(shared<Node> c) {
	if (c)
		params.add(c);
}

void Block::set(int index, shared<Node> c) {
	params[index] = c;
}

bool Block::is_trust_me() const {
	if (flags_has(flags, Flags::TrustMe))
		return true;
	if (parent)
		return parent->is_trust_me();
	return false;
}

bool Block::is_in_try() const {
	if (flags_has(flags, Flags::Try))
		return true;
	if (parent)
		return parent->is_in_try();
	return false;
}

Variable *Block::add_var(const string &name, const Class *type, Flags flags) {
	return insert_var(function->var.num, name, type, flags);
}

Variable *Block::insert_var(int index, const string &name, const Class *type, Flags flags) {
	if (get_var(name))
		function->owner()->do_error(format("variable '%s' already declared in this context", name));
	Variable *v = new Variable(name, type);
	v->flags = flags;
	function->var.insert(v, index);
	vars.add(v);
	return v;
}

Variable *Block::get_var(const string &name) const {
	for (auto *v: vars)
		if (v->name == name)
			return v;
	if (parent)
		return parent->get_var(name);
	return nullptr;
}

const Class *Block::name_space() const {
	return function->name_space;
}


}
