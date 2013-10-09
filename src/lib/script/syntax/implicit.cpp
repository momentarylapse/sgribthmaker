#include "../script.h"
#include "../asm/asm.h"
#include <stdio.h>
#include "../../file/file.h"

namespace Script{

void SyntaxTree::ImplementAddVirtualTable(Command *self, Function *f, Type *t)
{
	if (t->vtable){
		Command *p = shift_command(self, true, 0, TypePointer);
		int nc = AddConstant(TypePointer);
		(*(void**)Constants[nc].data) = t->vtable;
		Command *cmd_0 = add_command_const(nc);
		Command *c = add_command_operator(p, cmd_0, OperatorAssign);
		f->block->command.add(c);
	}
}

void SyntaxTree::ImplementAddChildConstructors(Command *self, Function *f, Type *t)
{
	int i0 = t->parent ? t->parent->element.num : 0;
	foreachi(ClassElement &e, t->element, i){
		if (i < i0)
			continue;
		ClassFunction *ff = e.type->GetDefaultConstructor();
		if (!ff)
			continue;
		Command *p = shift_command(self, true, e.offset, e.type);
		Command *c = add_command_classfunc(t, ff, ref_command(p));
		f->block->command.add(c);
	}
}

void SyntaxTree::ImplementImplicitConstructor(Function *f, Type *t, bool allow_parent_constructor)
{
	Command *self = add_command_local_var(f->get_var("self"), t->GetPointer());

	if (t->is_super_array){
		foreach(ClassFunction &ff, t->function)
			if (ff.name == "__mem_init__"){
				int nc = AddConstant(TypeInt);
				*(int*)Constants[nc].data = t->parent->size;
				Command *c = add_command_classfunc(t, &ff, self);
				c->param[0] = add_command_const(nc);
				c->num_params = 1;
				f->block->command.add(c);
			}
	}else{

		// parent constructor
		if ((t->parent) && (allow_parent_constructor)){
			ClassFunction *ff = t->parent->GetDefaultConstructor();
			if (ff){
				Command *c = add_command_classfunc(t, ff, cp_command(self));
				f->block->command.add(c);
			}
		}

		// add vtable reference
		if (t->vtable)
			ImplementAddVirtualTable(self, f, t);

		// call child constructors
		ImplementAddChildConstructors(self, f, t);
	}
}

#if 0
void SyntaxTree::CreateImplicitComplexConstructor(Function *f, Type *t)
{
	ClassFunction *pcc = t->parent->GetComplexConstructor();
	Function *pcf = pcc->script->syntax->Functions[pcc->nr];
	// create function
	Function *f = AddFunction("__init__", TypeVoid);
	for (int i=0;i<pcf->num_params;i++)
		f->AddVar(pcf->var[i].name, pcf->var[i].type);
	f->num_params = pcf->num_params;
	int fn = Functions.num - 1;
	f->Update(t);

	Command *self = add_command_local_var(f->get_var("self"), t->GetPointer());

	// parent constructor
	Command *c = add_command_classfunc(t, *pcc, cp_command(self));
	for (int i=0;i<pcf->num_params;i++)
		c->param[i] = add_command_local_var(i, pcf->var[i].type);
	c->num_params = pcf->num_params;
	f->block->command.add(c);

	// add vtable reference
	if (t->vtable)
		ImplementAddVirtualTable(self, f, t);

	// call child constructors
	ImplementAddChildConstructors(self, f, t);

	t->AddFunction(this, fn);
}
#endif


void SyntaxTree::ImplementImplicitDestructor(Function *f, Type *t)
{
	Command *self = add_command_local_var(f->get_var("self"), t->GetPointer());

	if (t->is_super_array){
		foreach(ClassFunction &ff, t->function)
			if (ff.name == "clear"){
				Command *c = add_command_classfunc(t, &ff, self);
				f->block->command.add(c);
			}
	}else{

		// call child destructors
		int i0 = t->parent ? t->parent->element.num : 0;
		foreachi(ClassElement &e, t->element, i){
			if (i < i0)
				continue;
			ClassFunction *ff = e.type->GetDestructor();
			if (!ff)
				continue;
			Command *p = shift_command(self, true, e.offset, e.type);
			Command *c = add_command_classfunc(t, ff, ref_command(p));
			f->block->command.add(c);
		}

		// parent destructor
		if (t->parent){
			ClassFunction *ff = t->parent->GetDestructor();
			if (ff){
				Command *c = add_command_classfunc(t, ff, cp_command(self));
				f->block->command.add(c);
			}
		}
	}
}

void SyntaxTree::ImplementImplicitAssign(Function *f, Type *t)
{
	Command *other = add_command_local_var(f->get_var("other"), t);

	Command *self = add_command_local_var(f->get_var("self"), t->GetPointer());

	if (t->is_super_array){

		ClassFunction *cf_resize = t->GetFunc("resize", TypeVoid, 1);
		if (!cf_resize){
			DoError(format("%s.__assign__(): no %s.resize() found", t->name.c_str(), t->name.c_str()));
			return;
		}

		// self.resize(other.num)
		Command *other_num = shift_command(other, false, config.PointerSize, TypeInt);

		Command *cmd_resize = add_command_classfunc(t, cf_resize, cp_command(self));
		cmd_resize->num_params = 1;
		cmd_resize->param[0] = other_num;
		f->block->command.add(cmd_resize);

		// for int i, 0, other.num
		//    self[i].__assign__(other[i])

		f->AddVar("i", TypeInt);

		Command *for_var = add_command_local_var(2, TypeInt);


		// for_var = 0
		int nc = AddConstant(TypeInt);
		(*(int*)Constants[nc].data) = 0;
		Command *cmd_0 = add_command_const(nc);
		Command *cmd_assign0 = add_command_operator(for_var, cmd_0, OperatorIntAssign);
		f->block->command.add(cmd_assign0);

		// while(for_var < self.num)
		Command *cmd_cmp = add_command_operator(for_var, cp_command_deep(other_num), OperatorIntSmaller);

		Command *cmd_while = add_command_compilerfunc(CommandFor);
		cmd_while->param[0] = cmd_cmp;
		f->block->command.add(cmd_while);

		Block *b = AddBlock();
		Command *cb = AddCommand(KindBlock, b->index, TypeVoid);

		// el := self.data[for_var]
		Command *deref_self = deref_command(cp_command(self));
		Command *self_data = shift_command(deref_self, false, 0, t->parent->GetPointer());
		Command *cmd_el = add_command_parray(self_data, for_var, t->parent);

		// el2 := other.data[for_var]
		Command *other_data = shift_command(other, false, 0, t->parent->GetPointer());
		Command *cmd_el2 = add_command_parray(other_data, for_var, t->parent);


		Command *cmd_assign = LinkOperator(OperatorAssign, cmd_el, cmd_el2);
		if (!cmd_assign)
			DoError(format("%s.__assign__(): no %s.__assign__() found", t->name.c_str(), t->parent->name.c_str()));
		b->command.add(cmd_assign);

		// ...for_var += 1
		Command *cmd_inc = add_command_operator(for_var, cmd_0 /*dummy*/, OperatorIntIncrease);
		b->command.add(cmd_inc);
		f->block->command.add(cb);
	}else{

		// call child assignment
		foreach(ClassElement &e, t->element){
			Command *p = shift_command(self, true, e.offset, e.type);
			Command *o = shift_command(cp_command(other), false, e.offset, e.type); // needed for call-by-ref conversion!

			Command *cmd_assign = LinkOperator(OperatorAssign, p, o);
			if (!cmd_assign)
				DoError(format("%s.__assign__(): no %s.__assign__ for element \"%s\"", t->name.c_str(), e.type->name.c_str(), e.name.c_str()));
			f->block->command.add(cmd_assign);
		}
	}
}


void SyntaxTree::ImplementImplicitArrayClear(Function *f, Type *t)
{
	f->AddVar("for_var", TypeInt);

	Command *self = add_command_local_var(f->get_var("self"), t->GetPointer());

	Command *self_num = shift_command(cp_command(self), true, config.PointerSize, TypeInt);

	Command *for_var = add_command_local_var(1, TypeInt);

// delete...
	ClassFunction *f_del = t->parent->GetDestructor();
	if (f_del){
		// for_var = 0
		int nc = AddConstant(TypeInt);
		(*(int*)Constants[nc].data) = 0;
		Command *cmd_0 = add_command_const(nc);
		Command *cmd_assign = add_command_operator(for_var, cmd_0, OperatorIntAssign);
		f->block->command.add(cmd_assign);

		// while(for_var < self.num)
		Command *cmd_cmp = add_command_operator(for_var, self_num, OperatorIntSmaller);

		Command *cmd_while = add_command_compilerfunc(CommandFor);
		cmd_while->param[0] = cmd_cmp;
		f->block->command.add(cmd_while);

		Block *b = AddBlock();
		Command *cb = AddCommand(KindBlock, b->index, TypeVoid);

		// el := self.data[for_var]
		Command *deref_self = deref_command(cp_command(self));
		Command *self_data = shift_command(deref_self, false, 0, t->parent->GetPointer());
		Command *cmd_el = add_command_parray(self_data, for_var, t->parent);

		// __delete__
		Command *cmd_delete = add_command_classfunc(t, f_del, ref_command(cmd_el));
		b->command.add(cmd_delete);

		// ...for_var += 1
		Command *cmd_inc = add_command_operator(for_var, cmd_0 /*dummy*/, OperatorIntIncrease);
		b->command.add(cmd_inc);
		f->block->command.add(cb);
	}

	// clear
	Command *cmd_clear = add_command_classfunc(t, t->GetFunc("__mem_clear__", TypeVoid, 0), self);
	f->block->command.add(cmd_clear);
}


void SyntaxTree::ImplementImplicitArrayResize(Function *f, Type *t)
{
	f->AddVar("for_var", TypeInt);
	f->AddVar("num_old", TypeInt);

	Command *num = add_command_local_var(0, TypeInt);

	Command *self = add_command_local_var(1, t->GetPointer());

	Command *self_num = shift_command(cp_command(self), true, config.PointerSize, TypeInt);

	Command *for_var = add_command_local_var(2, TypeInt);

	Command *num_old = add_command_local_var(3, TypeInt);

	// num_old = self.num
	Command *cmd_copy_num = add_command_operator(num_old, self_num, OperatorIntAssign);
	f->block->command.add(cmd_copy_num);

// delete...
	ClassFunction *f_del = t->parent->GetDestructor();
	if (f_del){
		// for_var = num
		Command *cmd_assign = add_command_operator(for_var, num, OperatorIntAssign);
		f->block->command.add(cmd_assign);

		// while(for_var < self.num)
		Command *cmd_cmp = add_command_operator(for_var, self_num, OperatorIntSmaller);

		Command *cmd_while = add_command_compilerfunc(CommandFor);
		cmd_while->param[0] = cmd_cmp;
		f->block->command.add(cmd_while);

		Block *b = AddBlock();
		Command *cb = AddCommand(KindBlock, b->index, TypeVoid);

		// el := self.data[for_var]
		Command *deref_self = deref_command(cp_command(self));
		Command *self_data = shift_command(deref_self, false, 0, t->parent->GetPointer());
		Command *cmd_el = add_command_parray(self_data, for_var, t->parent);

		// __delete__
		Command *cmd_delete = add_command_classfunc(t, f_del, ref_command(cmd_el));
		b->command.add(cmd_delete);

		// ...for_var += 1
		Command *cmd_inc = add_command_operator(for_var, num /*dummy*/, OperatorIntIncrease);
		b->command.add(cmd_inc);
		f->block->command.add(cb);
	}

	// resize
	Command *c_resize = add_command_classfunc(t, t->GetFunc("__mem_resize__", TypeVoid, 1), self);
	c_resize->num_params = 1;
	c_resize->param[0] = num;
	f->block->command.add(c_resize);

	// new...
	ClassFunction *f_init = t->parent->GetDefaultConstructor();
	if (f_init){
		// for_var = num_old
		Command *cmd_assign = add_command_operator(for_var, num_old, OperatorIntAssign);
		f->block->command.add(cmd_assign);

		// while(for_var < self.num)
		Command *cmd_cmp = add_command_operator(for_var, self_num, OperatorIntSmaller);

		Command *cmd_while = add_command_compilerfunc(CommandFor);
		cmd_while->param[0] = cmd_cmp;
		f->block->command.add(cmd_while);

		Block *b = AddBlock();
		Command *cb = AddCommand(KindBlock, b->index, TypeVoid);

		// el := self.data[for_var]
		Command *deref_self = deref_command(cp_command(self));
		Command *self_data = shift_command(deref_self, false, 0, t->parent->GetPointer());
		Command *cmd_el = add_command_parray(self_data, for_var, t->parent);

		// __init__
		Command *cmd_init = add_command_classfunc(t, f_init, ref_command(cmd_el));
		b->command.add(cmd_init);

		// ...for_var += 1
		Command *cmd_inc = add_command_operator(for_var, num /*dummy*/, OperatorIntIncrease);
		b->command.add(cmd_inc);
		f->block->command.add(cb);
	}
}

void SyntaxTree::ImplementImplicitArrayAdd(Function *f, Type *t)
{
	Command *item = add_command_local_var(0, t->parent);

	Command *self = add_command_local_var(1, t->GetPointer());

	Command *self_num = shift_command(cp_command(self), true, config.PointerSize, TypeInt);


	// resize(self.num + 1)
	int nc = AddConstant(TypeInt);
	(*(int*)Constants[nc].data) = 1;
	Command *cmd_1 = add_command_const(nc);
	Command *cmd_add = add_command_operator(self_num, cmd_1, OperatorIntAdd);
	Command *cmd_resize = add_command_classfunc(t, t->GetFunc("resize", TypeVoid, 1), self);
	cmd_resize->num_params = 1;
	cmd_resize->param[0] = cmd_add;
	f->block->command.add(cmd_resize);



	// el := self.data[self.num - 1]
	Command *cmd_sub = add_command_operator(cp_command(self_num), cmd_1, OperatorIntSubtract);
	Command *deref_self = deref_command(cp_command(self));
	Command *self_data = shift_command(deref_self, false, 0, t->parent->GetPointer());
	Command *cmd_el = add_command_parray(self_data, cmd_sub, t->parent);

	Command *cmd_assign = LinkOperator(OperatorAssign, cmd_el, item);
	if (!cmd_assign)
		DoError(format("%s.add(): no %s.__assign__ for elements", t->name.c_str(), t->parent->name.c_str()));
	f->block->command.add(cmd_assign);
}

void add_func_header(SyntaxTree *s, Type *t, const string &name, Type *return_type, Type *param_type, const string &param_name)
{
	Function *f = s->AddFunction(name, return_type);
	f->implement_later = true;
	if (param_type != TypeVoid){
		Variable v;
		v.name = param_name;
		v.type = param_type;
		f->var.add(v);
		f->num_params ++;
	}
	f->Update(t);
	t->AddFunction(s, s->Functions.num - 1, -1, false);
}

void SyntaxTree::AddFunctionHeadersForClass(Type *t)
{
	if (t->owner != this)
		return;
	if (t->is_pointer)
		return;
	msg_write("add funcs " + t->name);


	if (t->is_super_array){
		add_func_header(this, t, "__init__", TypeVoid, TypeVoid, "");
		add_func_header(this, t, "__delete__", TypeVoid, TypeVoid, "");
		add_func_header(this, t, "clear", TypeVoid, TypeVoid, "");
		add_func_header(this, t, "resize", TypeVoid, TypeInt, "num");
		add_func_header(this, t, "add", TypeVoid, t->parent, "x");
		add_func_header(this, t, "__assign__", TypeVoid, t, "other");
	}else if (!t->is_simple_class()){//needs_init){
		if (!t->GetDefaultConstructor())
			add_func_header(this, t, "__init__", TypeVoid, TypeVoid, "");
		if (!t->GetDestructor())
			add_func_header(this, t, "__delete__", TypeVoid, TypeVoid, "");
		if (!t->GetFunc("__assign__", TypeVoid, 1)){
			add_func_header(this, t, "__assign__", TypeVoid, t, "other");
			// implement only if parent has also done so
			/*if (t->parent){
				msg_write(t->parent->GetFunc("__assign__"));
				if (t->parent->GetFunc("__assign__") >= 0)
					CreateImplicitAssign(t);
			}else{
				msg_write("!p");
				CreateImplicitAssign(t);
			}*/
		}
	}
}

Function* class_get_func(Type *t, const string &name, Type *return_type, int num_params)
{
	ClassFunction *cf = t->GetFunc(name, return_type, num_params);
	if (cf)
		return cf->script->syntax->Functions[cf->nr];
	t->owner->DoError("class_get_func... " + t->name + "." + name);
	return NULL;
}

void SyntaxTree::ImplementImplicitFunctions(Type *t)
{
	if (t->owner != this)
		return;
	if (t->is_pointer)
		return;

	msg_write("implicit " + t->name);

	// needs complex functions?
	/*bool needs_init = false;
	foreach(t->Element, e)
		foreach(e->Type->Function, f)
			if (strcmp(f->Name, "__init__") == 0)
				needs_init = true;
	if (t->IsSuperArray)
		needs_init = true;*/

	if (t->is_super_array){
		ImplementImplicitConstructor(class_get_func(t, "__init__", TypeVoid, 0), t);
		ImplementImplicitDestructor(class_get_func(t, "__delete__", TypeVoid, 0), t);
		ImplementImplicitArrayClear(class_get_func(t, "clear", TypeVoid, 0), t);
		ImplementImplicitArrayResize(class_get_func(t, "resize", TypeVoid, 1), t);
		ImplementImplicitArrayAdd(class_get_func(t, "add", TypeVoid, 1), t);
		ImplementImplicitAssign(class_get_func(t, "__assign__", TypeVoid, 1), t);
	}else if (!t->is_simple_class()){//needs_init){
		ImplementImplicitConstructor(class_get_func(t, "__init__", TypeVoid, 0), t);
		ImplementImplicitDestructor(class_get_func(t, "__delete__", TypeVoid, 0), t);
		ImplementImplicitAssign(class_get_func(t, "__assign__", TypeVoid, 1), t);
		/*if (!t->GetComplexConstructor())
			if (t->parent)
				if (t->parent->GetComplexConstructor())
					CreateImplicitComplexConstructor(t);
		if (!t->GetDestructor())
			CreateImplicitDestructor(t);
		if (t->GetAssign()){
			// implement only if parent has also done so
			if (t->parent){
				msg_write(t->parent->GetFunc("__assign__"));
				if (t->parent->GetFunc("__assign__") >= 0)
					CreateImplicitAssign(t);
			}else{
				msg_write("!p");
				CreateImplicitAssign(t);
			}
		}*/
	}
}


}
