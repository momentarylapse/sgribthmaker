
#ifndef TYPE_H_
#define TYPE_H_


namespace Script{

class Script;
class SyntaxTree;
class Type;
struct Function;


struct ClassElement{
	string name;
	Type *type;
	int offset;
	bool hidden;
	ClassElement();
};
struct ClassFunction{
	string name;
	Script *script;
	int nr; // index in Functions[]
	// _func_(x)  ->  p.func(x)
	Array<Type*> param_type;
	Type *return_type;
	int virtual_index;
	bool needs_overriding;
	ClassFunction();
	ClassFunction(const string &name, Type *return_type, Script *s, int no);
	Function *GetFunc();
};

typedef void *VirtualTable;

class Type{
public:
	Type();
	//Type(const string &name, int size, SyntaxTree *owner);
	~Type();
	string name;
	int size; // complete size of type
	int array_length;
	bool is_array, is_super_array; // mutially exclusive!
	bool is_pointer, is_silent; // pointer silent (&)
	bool fully_parsed;
	Array<ClassElement> element;
	Array<ClassFunction> function;
	Type *parent;
	SyntaxTree *owner; // to share and be able to delete...
	Array<void*> vtable;
	void *_vtable_location_compiler_; // may point to const/opcode
	void *_vtable_location_target_; // (opcode offset adjusted)

	bool force_call_by_value;
	bool UsesCallByReference();
	bool UsesReturnByMemory();
	bool is_simple_class();
	bool is_size_known();
	Type *GetArrayElement();
	bool usable_as_super_array();
	bool needs_constructor();
	bool needs_destructor();
	bool IsDerivedFrom(Type *root) const;
	bool DeriveFrom(Type *root, bool increase_size);
	Type *GetPointer();
	Type *GetRoot();
	void AddFunction(SyntaxTree *s, int func_no, bool as_virtual = false, bool override = false);
	ClassFunction *GetFunc(const string &name, Type *return_type, int num_params, Type *param0 = NULL);
	ClassFunction *GetDefaultConstructor();
	ClassFunction *GetComplexConstructor();
	ClassFunction *GetDestructor();
	ClassFunction *GetAssign();
	ClassFunction *GetVirtualFunction(int virtual_index);
	ClassFunction *GetGet(Type *index);
	void LinkVirtualTable();
	void LinkExternalVirtualTable(void *p);
	void *CreateInstance();
	string var2str(void *p);
};
extern Type *TypeUnknown;
extern Type *TypeReg128; // dummy for compilation
extern Type *TypeReg64; // dummy for compilation
extern Type *TypeReg32; // dummy for compilation
extern Type *TypeReg16; // dummy for compilation
extern Type *TypeReg8; // dummy for compilation
extern Type *TypeVoid;
extern Type *TypePointer;
extern Type *TypeClass;
extern Type *TypeBool;
extern Type *TypeInt;
extern Type *TypeInt64;
extern Type *TypeFloat32;
extern Type *TypeFloat64;
extern Type *TypeChar;
extern Type *TypeCString;
extern Type *TypeString;

extern Type *TypeComplex;
extern Type *TypeVector;
extern Type *TypeRect;
extern Type *TypeColor;
extern Type *TypeQuaternion;

};

#endif /* TYPE_H_ */
