#include "../../base/base.h"
#include "type.h"

namespace Script{

Type *ScriptGetPreType(const string &name)
{
	for (int i=0;i<PreTypes.num;i++)
		if (name == PreTypes[i]->name)
			return PreTypes[i];
	return TypeUnknown;
}

bool Type::UsesCallByReference()
{	return ((!force_call_by_value) && (!is_pointer)) || (is_array);	}

bool Type::UsesReturnByMemory()
{	return ((!force_call_by_value) && (!is_pointer)) || (is_array);	}



bool Type::is_simple_class()
{
	if (!UsesCallByReference())
		return true;
	/*if (is_array)
		return false;*/
	if (is_super_array)
		return false;
	if (GetConstructor())
		return false;
	if (GetDestructor())
		return false;
	if (GetFunc("__assign__") >= 0)
		return false;
	foreach(ClassElement &e, element)
		if (!e.type->is_simple_class())
			return false;
	return true;
}

int Type::GetFunc(const string &name)
{
	foreachi(ClassFunction &f, function, i)
		if (f.name == name)
			return i;
	return -1;
}

ClassFunction *Type::GetConstructor()
{
	foreach(ClassFunction &f, function)
		if ((f.name == "__init__") && (f.return_type == TypeVoid) && (f.param_type.num == 0))
			return &f;
	return NULL;
}

ClassFunction *Type::GetDestructor()
{
	foreach(ClassFunction &f, function)
		if ((f.name == "__delete__") && (f.return_type == TypeVoid) && (f.param_type.num == 0))
			return &f;
	return NULL;
}

string Type::var2str(void *p)
{
	if (this == TypeInt)
		return i2s(*(int*)p);
	else if (this == TypeFloat)
		return f2s(*(float*)p, 3);
	else if (this == TypeBool)
		return b2s(*(bool*)p);
	else if (is_pointer)
		return p2s(*(void**)p);
	else if (this == TypeString)
		return "\"" + *(string*)p + "\"";
	else if (is_super_array){
		string s;
		DynamicArray *da = (DynamicArray*)p;
		for (int i=0; i<da->num; i++){
			if (i > 0)
				s += ", ";
			s += parent->var2str(((char*)da->data) + i * da->element_size);
		}
		return "[" + s + "]";
	}else if (element.num > 0){
		string s;
		foreachi(ClassElement &e, element, i){
			if (i > 0)
				s += ", ";
			s += e.type->var2str(((char*)p) + e.offset);
		}
		return "(" + s + ")";

	}else if (is_array){
			string s;
			for (int i=0; i<array_length; i++){
				if (i > 0)
					s += ", ";
				s += parent->var2str(((char*)p) + i * parent->size);
			}
			return "[" + s + "]";
		}
	return string((char*)p, size).hex();
}

}

