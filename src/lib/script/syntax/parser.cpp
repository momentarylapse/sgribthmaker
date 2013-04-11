#include "../script.h"
#include "../asm/asm.h"
#include "../../file/file.h"
#include <stdio.h>

namespace Script{


static int FoundConstantNr;
static Script *FoundConstantScript;


void ref_command_old(SyntaxTree *ps, Command *c);
void deref_command_old(SyntaxTree *ps, Command *c);
void command_make_ref(SyntaxTree *ps, Command *c, Command *param);
void CommandSetConst(SyntaxTree *ps, Command *c, int nc);
void exlink_make_var_local(SyntaxTree *ps, Type *t, int var_no);
void conv_cbr(SyntaxTree *ps, Command *&c, int var);

extern bool next_extern;
extern bool next_const;


#define is_variable(kind)	(((kind) == KindVarLocal) || ((kind) == KindVarGlobal))

inline bool type_match(Type *type, bool is_class, Type *wanted);
inline bool direct_type_match(Type *a, Type *b)
{
	return ( (a==b) || ( (a->is_pointer) && (b->is_pointer) ) );
}
inline bool type_match_with_cast(Type *type, bool is_class, bool is_modifiable, Type *wanted, int &penalty, int &cast);

static void so(const char *str)
{
#ifdef ScriptDebug
	/*if (strlen(str)>256)
		str[256]=0;*/
	msg_write(str);
#endif
}

static void so(const string &str)
{
#ifdef ScriptDebug
	/*if (strlen(str)>256)
		str[256]=0;*/
	msg_write(str);
#endif
}

static void so(int i)
{
#ifdef ScriptDebug
	msg_write(i);
#endif
}


int s2i2(const string &str)
{
	if ((str.num > 1) && (str[0]=='0')&&(str[1]=='x')){
		int r=0;
		for (int i=2;i<str.num;i++){
			r *= 16;
			if ((str[i]>='0')&&(str[i]<='9'))
				r+=str[i]-48;
			if ((str[i]>='a')&&(str[i]<='f'))
				r+=str[i]-'a'+10;
			if ((str[i]>='A')&&(str[i]<='F'))
				r+=str[i]-'A'+10;
		}
		return r;
	}else
		return	str._int();
}

// find the type of a (potential) constant
//  "1.2" -> float
Type *SyntaxTree::GetConstantType()
{
	msg_db_f("GetConstantType", 4);
	FoundConstantNr = -1;
	FoundConstantScript = NULL;

	// named constants
	foreachi(Constant &c, Constants, i)
		if (Exp.cur == c.name){
			FoundConstantNr = i;
			FoundConstantScript = script;
			return c.type;
		}


	// included named constants
	foreach(Script *inc, Includes)
		foreachi(Constant &c, inc->syntax->Constants, i)
			if (Exp.cur == c.name){
				FoundConstantNr = i;
				FoundConstantScript = inc;
				return c.type;
			}

	// character "..."
	if ((Exp.cur[0] == '\'') && (Exp.cur.back() == '\''))
		return TypeChar;

	// string "..."
	if ((Exp.cur[0] == '"') && (Exp.cur.back() == '"'))
		return FlagCompileOS ? TypeCString : TypeString;

	// numerical (int/float)
	Type *type = TypeInt;
	bool hex = (Exp.cur.num > 1) && (Exp.cur[0] == '0') && (Exp.cur[1] == 'x');
	for (int c=0;c<Exp.cur.num;c++)
		if ((Exp.cur[c] < '0') || (Exp.cur[c] > '9')){
			if (hex){
				if ((c >= 2) && (Exp.cur[c] < 'a') && (Exp.cur[c] > 'f'))
					return TypeUnknown;
			}else if (Exp.cur[c] == '.'){
				type = TypeFloat;
			}else{
				if ((c != 0) || (Exp.cur[c] != '-')) // allow sign
					return TypeUnknown;
			}
		}

	// super array [...]
	if (Exp.cur == "["){
		DoError("super array constant");
	}
	return type;
}

static int _some_int_;
static float _some_float_;
static char _some_string_[2048];

void *SyntaxTree::GetConstantValue()
{
	Type *type = GetConstantType();
// named constants
	if (FoundConstantNr >= 0)
		return FoundConstantScript->syntax->Constants[FoundConstantNr].data;
// literal
	if (type == TypeChar){
		_some_int_ = Exp.cur[1];
		return &_some_int_;
	}
	if ((type == TypeString) || (type == TypeCString)){
		for (int i=0;i<Exp.cur.num - 2;i++)
			_some_string_[i] = Exp.cur[i+1];
		_some_string_[Exp.cur.num - 2] = 0;
		return _some_string_;
	}
	if (type == TypeInt){
		_some_int_ = s2i2(Exp.cur);
		return &_some_int_;
	}
	if (type == TypeFloat){
		_some_float_ = Exp.cur._float();
		return &_some_float_;
	}
	return NULL;
}


void DoClassFunction(SyntaxTree *ps, Command *Operand, Type *t, int f_no, Function *f)
{
	msg_db_f("DoClassFunc", 1);
#if 0
	switch(Operand->kind){
		case KindVarLocal:
		case KindVarGlobal:
		case KindVarExternal:
		case KindConstant:
		case KindPointerShift:
		case KindArray:
		case KindDerefPointerShift:
		/*case KindRefToLocal:
		case KindRefToGlobal:
		case KindRefToConst:*/
			break;
		default:
			ps->DoError(string("class functions only allowed for object variables, not for: ", Kind2Str(Operand->kind)));
	}
#endif

	// create a command for the object
	Command *ob = ps->cp_command(Operand);

	//msg_write(LinkNr2Str(ps, Operand->Kind, Operand->Nr));

	// the function
	ClassFunction &cf = t->function[f_no];
	Operand->script = cf.script;
    Operand->kind = KindFunction;
	Operand->link_nr = cf.nr;
	Function *ff = cf.script->syntax->Functions[cf.nr];
	Operand->type = ff->literal_return_type;
	Operand->num_params = ff->num_params;
	ps->GetFunctionCall(ff->name, Operand, f);
	Operand->instance = ob;
}

Command *SyntaxTree::GetOperandExtensionElement(Command *Operand, Function *f)
{
	msg_db_f("GetOperandExtensionElement", 4);
	Exp.next();
	Type *type = Operand->type;

	// pointer -> dereference
	bool deref = false;
	if (type->is_pointer){
		type = type->parent;
		deref = true;
	}

	// find element
	for (int e=0;e<type->element.num;e++)
		if (Exp.cur == type->element[e].name){
			Exp.next();
			return 	shift_command(Operand, deref, type->element[e].offset, type->element[e].type);
		}

	// class function?
	for (int e=0;e<type->function.num;e++)
		if (Exp.cur == type->function[e].name){
			if (!deref)
				ref_command_old(this, Operand);
			Exp.next();
			DoClassFunction(this, Operand, type, e, f);
			return Operand;
		}

	DoError("unknown element of " + type->name);
}

Command *SyntaxTree::GetOperandExtensionArray(Command *Operand, Function *f)
{
	msg_db_f("GetOperandExtensionArray", 4);

	// allowed?
	bool allowed = ((Operand->type->is_array) || (Operand->type->is_super_array));
	bool pparray = false;
	if (!allowed)
		if (Operand->type->is_pointer){
			if ((!Operand->type->parent->is_array) && (!Operand->type->parent->is_super_array))
				DoError(format("using pointer type \"%s\" as an array (like in C) is not allowed any more", Operand->type->name.c_str()));
			allowed = true;
			pparray = (Operand->type->parent->is_super_array);
		}
	if (!allowed)
		DoError(format("type \"%s\" is neither an array nor a pointer to an array", Operand->type->name.c_str()));
	Exp.next();

	Command *array = AddCommand();
	array->num_params = 2;
	array->param[0] = Operand;

	// pointer?
	so(Operand->type->name);
	if (pparray){
		DoError("test... anscheinend gibt es [] auf * super array");
		//array = cp_command(this, Operand);
/*		Operand->kind = KindPointerAsArray;
		Operand->type = t->type->parent;
		deref_command_old(this, Operand);
		array = Operand->param[0];*/
	}else if (Operand->type->is_super_array){
		array->kind = KindPointerAsArray;
		array->type = Operand->type->parent;
		array->param[0] = shift_command(Operand, false, 0, GetPointerType(array->type));
	}else if (Operand->type->is_pointer){
		array->kind = KindPointerAsArray;
		array->type = Operand->type->parent->parent;
	}else{
		array->kind = KindArray;
		array->type = Operand->type->parent;
	}

	// array index...
	Command *index = GetCommand(f);
	array->param[1] = index;
	if (index->type != TypeInt){
		Exp.rewind();
		DoError(format("type of index for an array needs to be (int), not (%s)", index->type->name.c_str()));
	}
	if (Exp.cur != "]")
		DoError("\"]\" expected after array index");
	Exp.next();
	return array;
}

// find any ".", "->", or "[...]"'s    or operators?
Command *SyntaxTree::GetOperandExtension(Command *Operand, Function *f)
{
	msg_db_f("GetOperandExtension", 4);

	// nothing?
	int op = WhichPrimitiveOperator(Exp.cur);
	if ((Exp.cur != ".") && (Exp.cur != "[") && (Exp.cur != "->") && (op < 0))
		return Operand;

	if (Exp.cur == "->")
		DoError("\"->\" deprecated,  use \".\" instead");

	if (Exp.cur == "."){
		// class element?

		Operand = GetOperandExtensionElement(Operand, f);

	}else if (Exp.cur == "["){
		// array?

		Operand = GetOperandExtensionArray(Operand, f);


	}else if (op >= 0){
		// unary operator? (++,--)

		for (int i=0;i<PreOperators.num;i++)
			if (PreOperators[i].primitive_id == op)
				if ((PreOperators[i].param_type_1 == Operand->type) && (PreOperators[i].param_type_2 == TypeVoid)){
					so("  => unaerer Operator");
					Exp.next();
					return add_command_operator(Operand, NULL, i);
				}
		return Operand;
	}

	// recursion
	return GetOperandExtension(Operand, f);
}

bool SyntaxTree::GetSpecialFunctionCall(const string &f_name, Command *Operand, Function *f)
{
	msg_db_f("GetSpecialFuncCall", 4);

	// sizeof
	if ((Operand->kind == KindCompilerFunction) && (Operand->link_nr == CommandSizeof)){

		so("sizeof");
		Exp.next();
		int nc = AddConstant(TypeInt);
		CommandSetConst(this, Operand, nc);

		int nt = WhichType(Exp.cur);
		Type *type;
		if (nt >= 0)
			(*(int*)(Constants[nc].data)) = Types[nt]->size;
		else if ((GetExistence(Exp.cur, f)) && ((GetExistenceLink.kind == KindVarGlobal) || (GetExistenceLink.kind == KindVarLocal)))
			(*(int*)(Constants[nc].data)) = GetExistenceLink.type->size;
		else if (type == GetConstantType())
			(*(int*)(Constants[nc].data)) = type->size;
		else
			DoError("type-name or variable name expected in sizeof(...)");
		Exp.next();
		if (Exp.cur != ")")
			DoError("\")\" expected after parameter list");
		Exp.next();

		so(*(int*)(Constants[nc].data));
		return true;
	}

	// sizeof
	if ((Operand->kind == KindCompilerFunction) && (Operand->link_nr == CommandReturn))
		DoError("return");

	return false;
}


// cmd needs to have Param[]'s existing with correct Type!
void SyntaxTree::FindFunctionSingleParameter(int p, Type **WantedType, Function *f, Command *cmd)
{
	msg_db_f("FindFuncSingleParam", 4);
	Command *Param = GetCommand(f);

	WantedType[p] = TypeUnknown;
	if (cmd->kind == KindFunction){
		Function *ff = cmd->script->syntax->Functions[cmd->link_nr];
		if (p < ff->num_params)
			WantedType[p] = ff->literal_param_type[p];
	}else if (cmd->kind == KindCompilerFunction){
		if (p < PreCommands[cmd->link_nr].param.num)
			WantedType[p] = PreCommands[cmd->link_nr].param[p].type;
	}
	// link parameters
	cmd->param[p] = Param;
}

void SyntaxTree::FindFunctionParameters(int &np, Type **WantedType, Function *f, Command *cmd)
{
	if (Exp.cur != "(")
		DoError("\"(\" expected in front of function parameter list");
	msg_db_f("FindFunctionParameters", 4);
	Exp.next();

	// list of parameters
	np = 0;
	for (int p=0;p<SCRIPT_MAX_PARAMS;p++){
		if (Exp.cur == ")")
			break;
		np ++;
		// find parameter

		FindFunctionSingleParameter(p, WantedType, f, cmd);

		if (Exp.cur != ","){
			if (Exp.cur == ")")
				break;
			DoError("\",\" or \")\" expected after parameter for function");
		}
		Exp.next();
	}
	Exp.next(); // ')'
}

void apply_type_cast(SyntaxTree *ps, int tc, Command *param);


// check, if the command <link> links to really has type <type>
//   ...and try to cast, if not
void SyntaxTree::CheckParamLink(Command *link, Type *type, const string &f_name, int param_no)
{
	msg_db_f("CheckParamLink", 4);
	// type cast needed and possible?
	Type *pt = link->type;
	Type *wt = type;

	// "silent" pointer (&)?
	if ((wt->is_pointer) && (wt->is_silent)){
		if (direct_type_match(pt, wt->parent)){
			so("<silent Ref &>");

			ref_command_old(this, link);
		}else if ((pt->is_pointer) && (direct_type_match(pt->parent, wt->parent))){
			so("<silent Ref & of *>");

			// no need to do anything...
		}else{
			Exp.rewind();
			DoError(format("(c) parameter %d for function \"%s\" has type (%s), (%s) expected", param_no + 1, f_name.c_str(), pt->name.c_str(), wt->name.c_str()));
		}

	// normal type cast
	}else if (!direct_type_match(pt, wt)){
		int tc = -1;
		for (int i=0;i<TypeCasts.num;i++)
			if ((direct_type_match(TypeCasts[i].source, pt)) && (direct_type_match(TypeCasts[i].dest, wt)))
				tc = i;

		if (tc >= 0){
			so("TypeCast");
			apply_type_cast(this, tc, link);
		}else{
			DoError(format("(a) parameter %d for function \"%s\" has type (%s), (%s) expected", param_no + 1, f_name.c_str(), pt->name.c_str(), wt->name.c_str()));
		}
	}
}

// creates <Operand> to be the function call
//  on entry <Operand> only contains information from GetExistence (Kind, Nr, Type, NumParams)
void SyntaxTree::GetFunctionCall(const string &f_name, Command *Operand, Function *f)
{
	msg_db_f("GetFunctionCall", 4);

	// function as a variable?
	if (Exp.cur_exp >= 2)
	if ((Exp.get_name(Exp.cur_exp - 2) == "&") && (Exp.cur != "(")){
		if (Operand->kind == KindFunction){
			so("Funktion als Variable!");
			Operand->kind = KindVarFunction;
			Operand->type = TypePointer;
			Operand->num_params = 0;
			return;
		}else{
			Exp.rewind();
			//DoError("\"(\" expected in front of parameter list");
			DoError("only script functions can be referenced");
		}
	}


	// "special" functions
    if (Operand->kind == KindCompilerFunction)
	    if (Operand->link_nr == CommandSizeof){
			GetSpecialFunctionCall(f_name, Operand, f);
			return;
	    }

	so(Operand->type->name);
	// link operand onto this command
//	so(cmd->NumParams);



	// find (and provisorically link) the parameters in the source
	int np;
	Type *WantedType[SCRIPT_MAX_PARAMS];

	bool needs_brackets = ((Operand->type != TypeVoid) || (Operand->num_params != 1));
	if (needs_brackets){
		FindFunctionParameters(np, WantedType, f, Operand);

	}else{
		np = 1;
		FindFunctionSingleParameter(0, WantedType, f, Operand);
	}


	// return: parameter type by function
	if ((Operand->kind == KindCompilerFunction) && (Operand->link_nr == CommandReturn))
		WantedType[0] = f->literal_return_type;

	// test compatibility
	if (np != Operand->num_params){
		Exp.rewind();
		DoError(format("function \"%s\" expects %d parameters, %d were found",f_name.c_str(), Operand->num_params, np));
	}
	for (int p=0;p<np;p++){
		CheckParamLink(Operand->param[p], WantedType[p], f_name, p);
	}
}

Command *SyntaxTree::GetOperand(Function *f)
{
	msg_db_f("GetOperand", 4);
	Command *Operand = NULL;
	so(Exp.cur);

	// ( -> one level down and combine commands
	if (Exp.cur == "("){
		Exp.next();
		Operand = GetCommand(f);
		if (Exp.cur != ")")
			DoError("\")\" expected");
		Exp.next();
	}else if (Exp.cur == "&"){ // & -> address operator
		so("<Adress-Operator &>");
		Exp.next();
		Operand = GetOperand(f);
		ref_command_old(this, Operand);
	}else if (Exp.cur == "*"){ // * -> dereference
		so("<Dereferenzierung *>");
		Exp.next();
		Operand = GetOperand(f);
		if (!Operand->type->is_pointer){
			Exp.rewind();
			DoError("only pointers can be dereferenced using \"*\"");
		}
		deref_command_old(this, Operand);
	}else{
		// direct operand
		if (GetExistence(Exp.cur, f)){
			Operand = cp_command(&GetExistenceLink);
			string f_name =  Exp.cur;
			so("=> " + Kind2Str(Operand->kind));
			Exp.next();
			// variables get linked directly...

			// operand is executable
			if ((Operand->kind == KindFunction) || (Operand->kind == KindCompilerFunction)){
				GetFunctionCall(f_name, Operand, f);

			}else if (Operand->kind == KindPrimitiveOperator){
				// unary operator
				int _ie=Exp.cur_exp-1;
				so("  => unaerer Operator");
				int po = Operand->link_nr, o=-1;
				Command *sub_command = GetOperand(f);
				Type *r = TypeVoid;
				Type *p2 = sub_command->type;

				// exact match?
				bool ok=false;
				for (int i=0;i<PreOperators.num;i++)
					if ((unsigned)po == PreOperators[i].primitive_id)
						if ((PreOperators[i].param_type_1 == TypeVoid) && (type_match(p2, false, PreOperators[i].param_type_2))){
							o = i;
							r = PreOperators[i].return_type;
							ok = true;
							break;
						}


				// needs type casting?
				if (!ok){
					int pen2;
					int c2, c2_best;
					int pen_min = 100;
					for (int i=0;i<PreOperators.num;i++)
						if (po == PreOperators[i].primitive_id)
							if ((PreOperators[i].param_type_1 == TypeVoid) && (type_match_with_cast(p2, false, false, PreOperators[i].param_type_2, pen2, c2))){
								ok = true;
								if (pen2 < pen_min){
									r = PreOperators[i].return_type;
									o = i;
									pen_min = pen2;
									c2_best = c2;
								}
						}
					// cast
					if (ok){
						apply_type_cast(this, c2_best, sub_command);
					}
				}


				if (!ok){
					Exp.cur_exp = _ie;
					DoError("unknown unitary operator  " + p2->name);
				}
				return add_command_operator(sub_command, NULL, o);
			}
		}else{
			Type *t = GetConstantType();
			if (t != TypeUnknown){
				so("=> Konstante");
				Operand = AddCommand();
				Operand->kind = KindConstant;
				// constant for parameter (via variable)
				Operand->type = t;
				Operand->link_nr = AddConstant(t);
				int size = t->size;
				if (t == TypeString)
					size = 256;
				memcpy(Constants[Operand->link_nr].data, GetConstantValue(), size);
				Exp.next();
			}else{
				//Operand.Kind=0;
				DoError("unknown operand");
			}
		}

	}

	// Arrays, Strukturen aufloessen...
	Operand = GetOperandExtension(Operand,f);

	so("Operand endet mit " + Exp.get_name(Exp.cur_exp - 1));
	return Operand;
}

// only "primitive" operator -> no type information
Command *SyntaxTree::GetPrimitiveOperator(Function *f)
{
	msg_db_f("GetOperator",4);
	so(Exp.cur);
	int op = WhichPrimitiveOperator(Exp.cur);
	if (op < 0)
		return NULL;

	// command from operator
	Command *cmd = AddCommand();
	cmd->kind = KindPrimitiveOperator;
	cmd->link_nr = op;
	// only provisional (only operator sign, parameters and their types by GetCommand!!!)

	Exp.next();
	return cmd;
}

/*inline int find_operator(int primitive_id, Type *param_type1, Type *param_type2)
{
	for (int i=0;i<PreOperator.num;i++)
		if (PreOperator[i].PrimitiveID == primitive_id)
			if ((PreOperator[i].ParamType1 == param_type1) && (PreOperator[i].ParamType2 == param_type2))
				return i;
	//_do_error_("");
	return 0;
}*/

// both operand types have to match the operator's types
//   (operator wants a pointer -> all pointers are allowed!!!)
//   (same for classes of same type...)
inline bool type_match(Type *type, bool is_class, Type *wanted)
{
	if (type == wanted)
		return true;
	if ((type->is_pointer) && (wanted == TypePointer))
		return true;
	if ((is_class) && (wanted == TypeClass))
		return true;
	if ((type->is_super_array) && (wanted == TypeSuperArray))
		return true;
	return false;
}

inline bool type_match_with_cast(Type *type, bool is_class, bool is_modifiable, Type *wanted, int &penalty, int &cast)
{
	penalty = 0;
	cast = -1;
	if (type_match(type, is_class, wanted))
	    return true;
	if (is_modifiable) // is a variable getting assigned.... better not cast
		return false;
	for (int i=0;i<TypeCasts.num;i++)
		if ((direct_type_match(TypeCasts[i].source, type)) && (direct_type_match(TypeCasts[i].dest, wanted))){ // type_match()?
			penalty = TypeCasts[i].penalty;
			cast = i;
			return true;
		}
	return false;
}

void apply_type_cast(SyntaxTree *ps, int tc, Command *param)
{
	if (tc < 0)
		return;
	so(format("Benoetige automatischen TypeCast: %s -> %s", TypeCasts[tc].source->name.c_str(), TypeCasts[tc].dest->name.c_str()));
	if (param->kind == KindConstant){
		char *data_old = ps->Constants[param->link_nr].data;
		char *data_new = (char*)TypeCasts[tc].func(data_old);
		if ((TypeCasts[tc].dest->is_array) || (TypeCasts[tc].dest->is_super_array)){
			// arrays as return value -> reference!
			int size = TypeCasts[tc].dest->size;
			if (TypeCasts[tc].dest == TypeString)
				size = 256;
			delete[] data_old;
			ps->Constants[param->link_nr].data = new char[size];
			data_new = *(char**)data_new;
			memcpy(ps->Constants[param->link_nr].data, data_new, size);
		}else
			memcpy(ps->Constants[param->link_nr].data, data_new, TypeCasts[tc].dest->size);
		ps->Constants[param->link_nr].type = TypeCasts[tc].dest;
		param->type = TypeCasts[tc].dest;
		so("  ...Konstante wurde direkt gewandelt!");
	}else{
		Command *sub_cmd = ps->cp_command(param);
		if (TypeCasts[tc].kind == KindFunction){
			param->kind = KindFunction;
			param->link_nr = TypeCasts[tc].func_no;
			param->script = TypeCasts[tc].script;
			param->num_params = 1;
			param->param[0] = sub_cmd;
			param->instance = NULL;
			param->type = TypeCasts[tc].dest;
		}else if (TypeCasts[tc].kind == KindCompilerFunction){
			ps->CommandSetCompilerFunction(TypeCasts[tc].func_no, param);
			param->param[0] = sub_cmd;
		}
		so("  ...keine Konstante: Wandel-Befehl wurde hinzugefuegt!");
	}
}

Command *SyntaxTree::LinkOperator(int op_no, Command *param1, Command *param2)
{
	msg_db_f("LinkOp",4);
	bool left_modifiable = PrimitiveOperators[op_no].left_modifiable;
	string op_func_name = PrimitiveOperators[op_no].function_name;
	Command *op = NULL;

	Type *p1 = param1->type;
	Type *p2 = param2->type;
	bool equal_classes = false;
	if (p1 == p2)
		if (!p1->is_super_array)
			if (p1->element.num > 0)
				equal_classes = true;


	// exact match as class function?
	foreach(ClassFunction &f, p1->function)
		if (f.name == op_func_name){
			if (type_match(p2, equal_classes, f.param_type[0])){
				Command *inst = param1;
				ref_command_old(this, inst);
				op = add_command_classfunc(p1, f, inst);
				op->num_params = 1;
				op->param[0] = param2;
				return op;
			}
		}

	// exact match?
	for (int i=0;i<PreOperators.num;i++)
		if (op_no == PreOperators[i].primitive_id)
			if (type_match(p1, equal_classes, PreOperators[i].param_type_1) && type_match(p2, equal_classes, PreOperators[i].param_type_2)){
				return add_command_operator(param1, param2, i);
			}

	// exact match as class function but missing a "&"?
	foreach(ClassFunction &f, p1->function)
		if (f.name == op_func_name){
			if (f.param_type[0]->is_pointer && f.param_type[0]->is_silent)
				if (direct_type_match(p2, f.param_type[0]->parent)){
					Command *inst = param1;
					ref_command_old(this, inst);
					op = add_command_classfunc(p1, f, inst);
					op->num_params = 1;
					op->param[0] = param2;
					ref_command_old(this, op->param[0]);
					return op;
				}
		}


	// needs type casting?
	int pen1, pen2;
	int c1, c2, c1_best, c2_best;
	int pen_min = 2000;
	int op_found = -1;
	bool op_is_class_func = false;
	for (int i=0;i<PreOperators.num;i++)
		if ((unsigned)op_no == PreOperators[i].primitive_id)
			if (type_match_with_cast(p1, equal_classes, left_modifiable, PreOperators[i].param_type_1, pen1, c1) && type_match_with_cast(p2, equal_classes, false, PreOperators[i].param_type_2, pen2, c2))
				if (pen1 + pen2 < pen_min){
					op_found = i;
					pen_min = pen1 + pen2;
					c1_best = c1;
					c2_best = c2;
				}
	foreachi(ClassFunction &f, p1->function, i)
		if (f.name == op_func_name)
			if (type_match_with_cast(p2, equal_classes, false, f.param_type[0], pen2, c2))
				if (pen2 < pen_min){
					op_found = i;
					pen_min = pen2;
					c1_best = -1;
					c2_best = c2;
					op_is_class_func = true;
				}
	// cast
	if (op_found >= 0){
		apply_type_cast(this, c1_best, param1);
		apply_type_cast(this, c2_best, param2);
		if (op_is_class_func){
			Command *inst = param1;
			ref_command_old(this, inst);
			op = add_command_classfunc(p1, p1->function[op_found], inst);
			op->num_params = 1;
			op->param[0] = param2;
		}else{
			return add_command_operator(param1, param2, op_found);
		}
		return op;
	}

	return NULL;
}

void SyntaxTree::LinkMostImportantOperator(Array<Command*> &Operand, Array<Command*> &Operator, Array<int> &op_exp)
{
	msg_db_f("LinkMostImpOp",4);
// find the most important operator (mio)
	int mio = 0;
	for (int i=0;i<Operator.num;i++){
		so(format("%d %d", Operator[i]->link_nr, Operator[i]->link_nr));
		if (PrimitiveOperators[Operator[i]->link_nr].level > PrimitiveOperators[Operator[mio]->link_nr].level)
			mio = i;
	}
	so(mio);

// link it
	Command *param1 = Operand[mio];
	Command *param2 = Operand[mio + 1];
	int op_no = Operator[mio]->link_nr;
	Operator[mio] = LinkOperator(op_no, param1, param2);
	if (!Operator[mio]){
		Exp.cur_exp = op_exp[mio];
		DoError(format("no operator found: (%s) %s (%s)", param1->type->name.c_str(), PrimitiveOperators[op_no].name.c_str(), param2->type->name.c_str()));
	}

// remove from list
	Operand[mio] = Operator[mio];
	Operator.erase(mio);
	op_exp.erase(mio);
	Operand.erase(mio + 1);
}

Command *SyntaxTree::GetCommand(Function *f)
{
	msg_db_f("GetCommand", 4);
	Array<Command*> Operand;
	Array<Command*> Operator;
	Array<int> op_exp;

	// find the first operand
	Operand.add(GetOperand(f));

	// find pairs of operators and operands
	for (int i=0;true;i++){
		op_exp.add(Exp.cur_exp);
		Command *op = GetPrimitiveOperator(f);
		if (!op)
			break;
		Operator.add(op);
		if (Exp.end_of_line()){
			//Exp.rewind();
			DoError("unexpected end of line after operator");
		}
		Operand.add(GetOperand(f));
	}


	// in each step remove/link the most important operator
	while(Operator.num > 0)
		LinkMostImportantOperator(Operand, Operator, op_exp);

	// complete command is now collected in Operand[0]
	return Operand[0];
}


void SyntaxTree::GetSpecialCommand(Block *block, Function *f)
{
	msg_db_f("GetSpecialCommand", 4);

	// special commands...
	if (Exp.cur == "for"){
		// variable
		Exp.next();
		Command *for_var;
		// internally declared?
		bool internally = false;
		if ((Exp.cur == "int") || (Exp.cur == "float")){
			Type *t = (Exp.cur == "int") ? TypeInt : TypeFloat;
			internally = true;
			Exp.next();
			int var_no = AddVar(Exp.cur, t, f);
			exlink_make_var_local(this, t, var_no);
 			for_var = cp_command(&GetExistenceLink);
		}else{
			GetExistence(Exp.cur, f);
 			for_var = cp_command(&GetExistenceLink);
			if ((!is_variable(for_var->kind)) || ((for_var->type != TypeInt) && (for_var->type != TypeFloat)))
				DoError("int or float variable expected after \"for\"");
		}
		Exp.next();

		// first value
		if (Exp.cur != ",")
			DoError("\",\" expected after variable in for");
		Exp.next();
		Command *val0 = GetCommand(f);
		if (val0->type != for_var->type){
			Exp.rewind();
			DoError(format("%s expected as first value of for", for_var->type->name.c_str()));
		}

		// last value
		if (Exp.cur != ",")
			DoError("\",\" expected after variable in for");
		Exp.next();
		Command *val1 = GetCommand(f);
		if (val1->type != for_var->type){
			Exp.rewind();
			DoError(format("%s expected as last value of for", for_var->type->name.c_str()));
		}

		// implement
		// for_var = val0
		Command *cmd_assign = add_command_operator(for_var, val0, OperatorIntAssign);
		block->command.add(cmd_assign);

		// while(for_var < val1)
		Command *cmd_cmp = add_command_operator(for_var, val1, OperatorIntSmaller);

		Command *cmd_while = add_command_compilerfunc(CommandFor);
		cmd_while->param[0] = cmd_cmp;
		block->command.add(cmd_while);
		ExpectNewline();
		// ...block
		Exp.next_line();
		ExpectIndent();
		int loop_block_no = Blocks.num; // should get created...soon
		GetCompleteCommand(block, f);

		// ...for_var += 1
		Command *cmd_inc;
		if (for_var->type == TypeInt){
			cmd_inc = add_command_operator(for_var, val1 /*dummy*/, OperatorIntIncrease);
		}else{
			int nc = AddConstant(TypeFloat);
			*(float*)Constants[nc].data = 1.0;
			Command *val_add = add_command_const(nc);
			cmd_inc = add_command_operator(for_var, val_add, OperatorFloatAddS);
		}
		Block *loop_block = Blocks[loop_block_no];
		loop_block->command.add(cmd_inc); // add to loop-block

		// <for_var> declared internally?
		// -> force it out of scope...
		if (internally)
			f->var[for_var->link_nr].name = "-out-of-scope-";

	}else if (Exp.cur == "forall"){
		// for index
		int var_no_index = AddVar("-for_index-", TypeInt, f);
		exlink_make_var_local(this, TypeInt, var_no_index);
 		Command *for_index = cp_command(&GetExistenceLink);

		// variable
		Exp.next();
		string var_name = Exp.cur;
		Exp.next();

		// super array
		if (Exp.cur != "in")
			DoError("\"in\" expected after variable in forall");
		Exp.next();
		Command *for_array = GetOperand(f);
		if (!for_array->type->is_super_array)
			DoError("list expected as second parameter in \"forall\"");
		//Exp.next();

		// variable...
		Type *var_type = for_array->type->parent;
		int var_no = AddVar(var_name, var_type, f);
		exlink_make_var_local(this, var_type, var_no);
 		Command *for_var = cp_command(&GetExistenceLink);

		// 0
		int nc = AddConstant(TypeInt);
		*(int*)Constants[nc].data = 0;
		Command *val0 = add_command_const(nc);

		// implement
		// for_index = 0
		Command *cmd_assign = add_command_operator(for_index, val0, OperatorIntAssign);
		block->command.add(cmd_assign);

		// array.num
		Command *val1 = AddCommand();
		val1->kind = KindAddressShift;
		val1->link_nr = config.PointerSize;
		val1->type = TypeInt;
		val1->num_params = 1;
		val1->param[0] = for_array;

		// while(for_index < val1)
		Command *cmd_cmp = add_command_operator(for_index, val1, OperatorIntSmaller);

		Command *cmd_while = add_command_compilerfunc(CommandFor);
		cmd_while->param[0] = cmd_cmp;
		block->command.add(cmd_while);
		ExpectNewline();
		// ...block
		Exp.next_line();
		ExpectIndent();
		int loop_block_no = Blocks.num; // should get created...soon
		GetCompleteCommand(block, f);

		// ...for_index += 1
		Command *cmd_inc = add_command_operator(for_index, val1 /*dummy*/, OperatorIntIncrease);
		Block *loop_block = Blocks[loop_block_no];
		loop_block->command.add(cmd_inc); // add to loop-block

		// &for_var
		Command *for_var_ref = AddCommand();
		command_make_ref(this, for_var_ref, for_var);

		// &array.data[for_index]
		Command *array_el = AddCommand();
		array_el->kind = KindPointerAsArray;
		array_el->num_params = 2;
		array_el->param[0] = shift_command(for_array, false, 0, GetPointerType(var_type));
		array_el->param[1] = for_index;
		array_el->type = var_type;
		Command *array_el_ref = AddCommand();
		command_make_ref(this, array_el_ref, array_el);

		// &for_var = &array[for_index]
		Command *cmd_var_assign = add_command_operator(for_var_ref, array_el_ref, OperatorPointerAssign);
		loop_block->command.insert(cmd_var_assign, 0);

		// ref...
		f->var[var_no].type = GetPointerType(var_type);
		foreach(Command *c, loop_block->command)
			conv_cbr(this, c, var_no);

		// force for_var out of scope...
		f->var[for_var->link_nr].name = "-out-of-scope-";
		f->var[for_index->link_nr].name = "-out-of-scope-";

	}else if (Exp.cur == "while"){
		Exp.next();
		Command *cmd_cmp = GetCommand(f);
		CheckParamLink(cmd_cmp, TypeBool, "while", 0);
		ExpectNewline();

		Command *cmd_while = add_command_compilerfunc(CommandWhile);
		cmd_while->param[0] = cmd_cmp;
		block->command.add(cmd_while);
		// ...block
		Exp.next_line();
		ExpectIndent();
		GetCompleteCommand(block, f);
 	}else if (Exp.cur == "break"){
		Exp.next();
		Command *cmd = add_command_compilerfunc(CommandBreak);
		block->command.add(cmd);
	}else if (Exp.cur == "continue"){
		Exp.next();
		Command *cmd = add_command_compilerfunc(CommandContinue);
		block->command.add(cmd);
	}else if (Exp.cur == "if"){
		int ind = Exp.cur_line->indent;
		Exp.next();
		Command *cmd_cmp = GetCommand(f);
		CheckParamLink(cmd_cmp, TypeBool, "if", 0);
		ExpectNewline();

		Command *cmd_if = add_command_compilerfunc(CommandIf);
		cmd_if->param[0] = cmd_cmp;
		block->command.add(cmd_if);
		// ...block
		Exp.next_line();
		ExpectIndent();
		GetCompleteCommand(block, f);
		Exp.next_line();

		// else?
		if ((!Exp.end_of_file()) && (Exp.cur == "else") && (Exp.cur_line->indent >= ind)){
			cmd_if->link_nr = CommandIfElse;
			Exp.next();
			// iterative if
			if (Exp.cur == "if"){
				// sub-if's in a new block
				Block *new_block = AddBlock();
				// parse the next if
				GetCompleteCommand(new_block, f);
				// command for the found block
				Command *cmd_block = AddCommand();
				cmd_block->kind = KindBlock;
				cmd_block->link_nr = new_block->index;
				// ...
				block->command.add(cmd_block);
				return;
			}
			ExpectNewline();
			// ...block
			Exp.next_line();
			ExpectIndent();
			GetCompleteCommand(block, f);
			//Exp.next_line();
		}else{
			Exp.cur_line --;
			Exp.cur_exp = Exp.cur_line->exp.num - 1;
			Exp.cur = Exp.cur_line->exp[Exp.cur_exp].name;
		}
	}
}

/*void ParseBlock(sBlock *block, sFunction *f)
{
}*/

// we already are in the line to analyse ...indentation for a new block should compare to the last line
void SyntaxTree::GetCompleteCommand(Block *block, Function *f)
{
	msg_db_f("GetCompleteCommand", 4);
	// cur_exp = 0!

	Type *tType = GetType(Exp.cur, false);
	int last_indent = Exp.indent_0;

	// block?  <- indent
	if (Exp.indented){
		Exp.indented = false;
		Exp.cur_exp = 0; // bad hack...
		Exp.cur = Exp.cur_line->exp[Exp.cur_exp].name;
		msg_db_f("Block", 4);
		Block *new_block = AddBlock();
		new_block->root = block->index;

		Command *c = AddCommand();
		c->kind = KindBlock;
		c->link_nr = new_block->index;
		block->command.add(c);

		for (int i=0;true;i++){
			if (((i > 0) && (Exp.cur_line->indent < last_indent)) || (Exp.end_of_file()))
				break;

			GetCompleteCommand(new_block, f);
			Exp.next_line();
		}
		Exp.cur_line --;
		Exp.indent_0 = Exp.cur_line->indent;
		Exp.indented = false;
		Exp.cur_exp = Exp.cur_line->exp.num - 1;
		Exp.cur = Exp.cur_line->exp[Exp.cur_exp].name;

	// assembler block
	}else if (Exp.cur == "-asm-"){
		Exp.next();
		so("<Asm-Block>");
		Command *c = add_command_compilerfunc(CommandAsm);
		block->command.add(c);

	// local (variable) definitions...
	// type of variable
	}else if (tType){
		for (int l=0;!Exp.end_of_line();l++){
			ParseVariableDefSingle(tType, f);

			// assignment?
			if (Exp.cur == "="){
				Exp.rewind();
				Command *c = GetCommand(f);
				block->command.add(c);
			}
			if (Exp.end_of_line())
				break;
			if ((Exp.cur != ",") && (!Exp.end_of_line()))
				DoError("\",\", \"=\" or newline expected after definition of local variable");
			Exp.next();
		}
		return;
	}else{


	// commands (the actual code!)
		if ((Exp.cur == "for") || (Exp.cur == "forall") || (Exp.cur == "while") || (Exp.cur == "break") || (Exp.cur == "continue") || (Exp.cur == "if")){
			GetSpecialCommand(block, f);

		}else{

			// normal commands
			Command *c = GetCommand(f);

			// link
			block->command.add(c);
		}
	}

	ExpectNewline();
}

// look for array definitions and correct pointers
void SyntaxTree::TestArrayDefinition(Type **type, bool is_pointer)
{
	msg_db_f("TestArrayDef", 4);
	if (is_pointer){
		(*type) = GetPointerType((*type));
	}
	if (Exp.cur == "["){
		Type nt;
		int array_size;
		string or_name = (*type)->name;
		int or_name_length = or_name.num;
		so("-Array-");
		Exp.next();

		// no index -> super array
		if (Exp.cur == "]"){
			array_size = -1;

		}else{

			// find array index
			Command *c = GetCommand(&RootOfAllEvil);
			PreProcessCommand(NULL, c);

			if ((c->kind != KindConstant) || (c->type != TypeInt))
				DoError("only constants of type \"int\" allowed for size of arrays");
			array_size = *(int*)Constants[c->link_nr].data;
			//Exp.next();
			if (Exp.cur != "]")
				DoError("\"]\" expected after array size");
		}
		Exp.next();
		// recursion
		TestArrayDefinition(type, false); // is_pointer=false, since pointers have been handled

		// create array       (complicated name necessary to get correct ordering   int a[2][4] = (int[4])[2])
		if (array_size < 0){
			(*type) = CreateNewType(or_name + "[]" +  (*type)->name.substr(or_name_length, -1),
			                        config.SuperArraySize, false, false, true, array_size, (*type));
			CreateImplicitFunctions((*type), cur_func);
		}else{
			(*type) = CreateNewType(or_name + format("[%d]", array_size) + (*type)->name.substr(or_name_length, -1),
			                        (*type)->size * array_size, false, false, true, array_size, (*type));
		}
		if (Exp.cur == "*"){
			so("nachtraeglich Pointer");
			Exp.next();
			TestArrayDefinition(type, true);
		}
	}
}


void SyntaxTree::ParseImport()
{
	msg_db_f("ParseImport", 4);
	Exp.next(); // 'use' / 'import'

	string name = Exp.cur;
	if (name.find(".kaba") >= 0){

		string filename = Filename.dirname() + name.substr(1, name.num - 2); // remove "
		filename = filename.no_recursion();

		msg_right();
		Script *include;
		try{
			include = Load(filename, true, script->JustAnalyse);
		}catch(Exception &e){
			string msg = "in imported file:\n\"" + e.message + "\"";
			DoError(msg);
		}

		msg_left();
		AddIncludeData(include);
	}else{
		DoError("can't import packages ,,,yet");
	}
}


void SyntaxTree::ParseEnum()
{
	msg_db_f("ParseEnum", 4);
	Exp.next(); // 'enum'
	ExpectNewline();
	int value = 0;
	Exp.next_line();
	ExpectIndent();
	for (int i=0;!Exp.end_of_file();i++){
		for (int j=0;!Exp.end_of_line();j++){
			int nc = AddConstant(TypeInt);
			Constant *c = &Constants[nc];
			c->name = Exp.cur;
			Exp.next();

			// explicit value
			if (Exp.cur == ":"){
				Exp.next();
				ExpectNoNewline();
				Type *type = GetConstantType();
				if (type == TypeInt)
					value = *(int*)GetConstantValue();
				else
					DoError("integer constant expected after \":\" for explicit value of enum");
				Exp.next();
			}
			*(int*)c->data = value ++;

			if (Exp.end_of_line())
				break;
			if ((Exp.cur != ","))
				DoError("\",\" or newline expected after enum definition");
			Exp.next();
			ExpectNoNewline();
		}
		Exp.next_line();
		if (Exp.unindented)
			break;
	}
	Exp.cur_line --;
}

static int ExternalFuncPreCommandIndex;

void SyntaxTree::ParseClassFunction(Type *t, bool as_extern)
{
	ParseFunction(t, as_extern);

	ClassFunction cf;
	if (as_extern){
		DoError("external class function... todo");
		/*PreCommand *c = &PreCommands[ExternalFuncPreCommandIndex];
		cf.name = c->name.substr(t->name.num + 1, -1);
		cf.kind = KindCompilerFunction;
		cf.nr = ExternalFuncPreCommandIndex;
		cf.return_type = c->return_type;
		foreach(PreCommandParam &p, c->param)
			cf.param_type.add(p.type);*/
	}else{
		Function *f = Functions.back();
		cf.name = f->name.substr(t->name.num + 1, -1);
		cf.nr = Functions.num - 1;
		cf.return_type = f->return_type;
		cf.script = script;
		for (int i=0;i<f->num_params;i++)
			cf.param_type.add(f->var[i].type);
	}
	t->function.add(cf);
}

inline bool type_needs_alignment(Type *t)
{
	if (t->is_array)
		return type_needs_alignment(t->parent);
	return (t->size >= 4);
}

void SyntaxTree::ParseClass()
{
	msg_db_f("ParseClass", 4);

	int indent0 = Exp.cur_line->indent;
	int _offset = 0;
	Exp.next(); // 'class'
	string name = Exp.cur;
	Exp.next();

	// create class and type
	int nt0 = Types.num;
	Type *t = CreateNewType(name, 0, false, false, false, 0, NULL);
	if (nt0 == Types.num){
		Exp.rewind();
		DoError("class already exists");
	}

	// parent class
	if (Exp.cur == ":"){
		so("vererbung der struktur");
		Exp.next();
		Type *ancestor = GetType(Exp.cur, true);
		bool found = false;
		if (ancestor->element.num > 0){
			// inheritance of elements
			t->element = ancestor->element;
			_offset = ancestor->size;
			found = true;
		}
		if (ancestor->function.num > 0){
			// inheritance of functions
			foreach(ClassFunction &f, ancestor->function)
				if ((f.name != "__init__") && (f.name != "__delete__") && (f.name != "__assign__"))
					t->function.add(f);
			found = true;
		}
		if (!found)
			DoError(format("parental type in class definition after \":\" has to be a class, but (%s) is not", ancestor->name.c_str()));
	}
	ExpectNewline();

	// elements
	for (int num=0;true;num++){
		Exp.next_line();
		if (Exp.cur_line->indent <= indent0) //(unindented)
			break;
		if (Exp.end_of_file())
			break;

		// extern?
		next_extern = false;
		if (Exp.cur == "extern"){
			next_extern = true;
			Exp.next();
		}
		int ie = Exp.cur_exp;

		Type *tType = GetType(Exp.cur, true);
		for (int j=0;!Exp.end_of_line();j++){
			//int indent = Exp.cur_line->indent;

			ClassElement el;
			bool is_pointer = false;
			Type *type = tType;
			if (Exp.cur == "*"){
				Exp.next();
				is_pointer = true;
			}
			el.name = Exp.cur;
			Exp.next();
			TestArrayDefinition(&type, is_pointer);
			el.type = type;

			// is a function?
			bool is_function = false;
			if (Exp.cur == "(")
			    is_function = true;
			if (is_function){
				Exp.cur_exp = ie;
				Exp.cur = Exp.cur_line->exp[Exp.cur_exp].name;
				ParseClassFunction(t, next_extern);

				break;
			}


			if (type_needs_alignment(type))
				_offset = mem_align(_offset, 4);
			so(format("Class-Element: %s %s  Offset: %d", type->name.c_str(), el.name.c_str(), _offset));
			if ((Exp.cur != ",") && (!Exp.end_of_line()))
				DoError("\",\" or newline expected after class element");
			el.offset = _offset;
			_offset += type->size;
			t->element.add(el);
			if (Exp.end_of_line())
				break;
			Exp.next();
		}
	}
	foreach(ClassElement &e, t->element)
		if (type_needs_alignment(e.type))
			_offset = mem_align(_offset, 4);
	t->size = _offset;


	CreateImplicitFunctions(t, false);

	Exp.cur_line --;
}

void SyntaxTree::ExpectNoNewline()
{
	if (Exp.end_of_line())
		DoError("unexpected newline");
}

void SyntaxTree::ExpectNewline()
{
	if (!Exp.end_of_line())
		DoError("newline expected");
}

void SyntaxTree::ExpectIndent()
{
	if (!Exp.indented)
		DoError("additional indent expected");
}

void SyntaxTree::ParseGlobalConst(const string &name, Type *type)
{
	msg_db_f("ParseGlobalConst", 6);
	if (Exp.cur != "=")
		DoError("\"=\" expected after const name");
	Exp.next();

	// find const value
	Command *cv = GetCommand(&RootOfAllEvil);
	PreProcessCommand(NULL, cv);

	if ((cv->kind != KindConstant) || (cv->type != type))
		DoError(format("only constants of type \"%s\" allowed as value for this constant", type->name.c_str()));

	// give our const the name
	Constant *c = &Constants[cv->link_nr];
	c->name = name;
}

void AddExternalVar(const string &name, Type *type)
{
#if 0
	so("extern");
	// already existing?
	bool found = false;
	for (int i=0;i<PreExternalVars.num;i++)
		if (PreExternalVars[i].is_semi_external)
			if (PreExternalVars[i].name == name){
				PreExternalVars[i].type = type;
				found = true;
				break;
			}

		// not found -> create provisorium (not linkable.... but parsable)
		if (!found){
			// ScriptLinkSemiExternalVar()
			PreExternalVar v;
			v.name = name;
			v.pointer = NULL;
			v.type = type;
			v.is_semi_external = true;
			PreExternalVars.add(v);
		}
#endif
}

void CopyFuncDataToExternal(Function *f, PreCommand *c, bool is_class_func)
{
#if 0
	c->is_class_function = is_class_func;
	c->return_type = f->return_type;
	c->param.clear();
	for (int j=0;j<f->num_params;j++){
		PreCommandParam p;
		p.name = f->var[j].name;
		p.type = f->var[j].type;
		c->param.add(p);
	}
#endif
}

void AddExternalFunc(SyntaxTree *ps, Function *f, Type *class_type)
{
	ps->DoError("external function... todo");
#if 0
	so("extern");

	string func_name = f->name;

	// already existing?
	bool found = false;
	for (int i=0;i<PreCommands.num;i++)
		if (PreCommands[i].is_semi_external)
			if (PreCommands[i].name == func_name){
				ExternalFuncPreCommandIndex = i;
				CopyFuncDataToExternal(f, &PreCommands[i], class_type != NULL);
				found = true;
				break;
			}

	// not found -> create provisorium (not linkable.... but parsable)
	if (!found){
		PreCommand c;
		c.name = func_name;
		c.func = NULL;
		CopyFuncDataToExternal(f, &c, class_type != NULL);
		c.is_semi_external = true;
		ExternalFuncPreCommandIndex = PreCommands.num;
		PreCommands.add(c);
	}

	// delete as function
	ps->Functions.pop();
#endif
}

Type *SyntaxTree::ParseVariableDefSingle(Type *type, Function *f, bool as_param)
{
	msg_db_f("ParseVariableDefSingle", 6);

	bool is_pointer = false;
	string name;

	// pointer?
	if (Exp.cur == "*"){
		Exp.next();
		is_pointer = true;
	}

	// name
	name = Exp.cur;
	Exp.next();
	so("Variable: " + name);

	// array?
	TestArrayDefinition(&type, is_pointer);

	// add
	if (next_extern){
		DoError("external vars not supported ...yet");
		AddExternalVar(name, type);
	}else if (next_const){
		ParseGlobalConst(name, type);
	}else
		AddVar(name, type, f);
	return type;
}

void SyntaxTree::ParseVariableDef(bool single, Function *f)
{
	msg_db_f("ParseVariableDef", 4);
	Type *type = GetType(Exp.cur, true);

	for (int j=0;true;j++){
		ExpectNoNewline();

		ParseVariableDefSingle(type, f);

		if ((Exp.cur != ",") && (!Exp.end_of_line()))
			DoError("\",\" or newline expected after definition of a global variable");

		// last one?
		if (Exp.end_of_line())
			break;

		Exp.next(); // ','
	}
}


void SyntaxTree::ParseFunction(Type *class_type, bool as_extern)
{
	msg_db_f("ParseFunction", 4);

// return type
	Type *return_type = GetType(Exp.cur, true);

	// pointer?
	if (Exp.cur == "*"){
		Exp.next();
		return_type = GetPointerType(return_type);
	}

	so(Exp.cur);
	Function *f = AddFunction(Exp.cur, return_type);
	cur_func = f;
	next_extern = false;

	Exp.next();
	Exp.next(); // '('

// parameter list

	if (Exp.cur != ")")
		for (int k=0;k<SCRIPT_MAX_PARAMS;k++){
			// like variable definitions

			f->num_params ++;

			// type of parameter variable
			Type *param_type = GetType(Exp.cur, true);
			Type *pt = ParseVariableDefSingle(param_type, f, true);
			f->var.back().type = pt;

			if (Exp.cur == ")")
				break;

			if (Exp.cur != ",")
				DoError("\",\" or \")\" expected after parameter");
			Exp.next(); // ','
		}
	Exp.next(); // ')'

	// save "original" param types (Var[].Type gets altered for call by reference)
	for (int i=0;i<f->num_params;i++)
		f->literal_param_type[i] = f->var[i].type;

	if (!Exp.end_of_line())
		DoError("newline expected after parameter list");


	// return by memory
	if (return_type->UsesReturnByMemory())
		AddVar("-return-", GetPointerType(return_type), f);

	// class function
	f->_class = class_type;
	if (class_type){
		AddVar("self", GetPointerType(class_type), f);

		// convert name to Class.Function
		f->name = class_type->name + "." +  f->name;
	}

	if (as_extern){
		AddExternalFunc(this, f, class_type);
		cur_func = NULL;
		return;
	}

	ExpressionBuffer::Line *this_line = Exp.cur_line;


// instructions
	while(true){
		Exp.next_line();
		Exp.indented = false;

		// end of file
		if (Exp.end_of_file())
			break;

		// end of function
		if (Exp.cur_line->indent <= this_line->indent)
			break;

		// command or local definition
		GetCompleteCommand(f->block, f);
	}
	cur_func = NULL;

	Exp.cur_line --;
}

// convert text into script data
void SyntaxTree::Parser()
{
	msg_db_f("Parser", 4);

	RootOfAllEvil.name = "RootOfAllEvil";
	cur_func = NULL;

	// syntax analysis

	Exp.reset_parser();

	// global definitions (enum, class, variables and functions)
	while (!Exp.end_of_file()){
		next_extern = false;
		next_const = false;

		// extern?
		if (Exp.cur == "extern"){
			next_extern = true;
			Exp.next();
		}

		// const?
		if (Exp.cur == "const"){
			next_const = true;
			Exp.next();
		}


		if ((Exp.cur == "import") || (Exp.cur == "use")){
			ParseImport();

		// enum
		}else if (Exp.cur == "enum"){
			ParseEnum();

		// class
		}else if ((Exp.cur == "struct") || (Exp.cur == "class")){
			ParseClass();

		}else{

			// type of definition
			GetType(Exp.cur, true);
			Exp.rewind();
			bool is_function = false;
			for (int j=1;j<Exp.cur_line->exp.num-1;j++)
				if (strcmp(Exp.cur_line->exp[j].name, "(") == 0)
				    is_function = true;

			// own function?
			if (is_function){
				ParseFunction(NULL, next_extern);

			// global variables
			}else{
				ParseVariableDef(false, &RootOfAllEvil);
			}
		}
		Exp.next_line();
	}
}

}
