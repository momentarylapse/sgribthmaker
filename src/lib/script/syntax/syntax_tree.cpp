#include "../script.h"
#include "../asm/asm.h"
#include "../../file/file.h"
#include <stdio.h>

namespace Script{

//#define ScriptDebug


/*#define PRESCRIPT_DB_LEVEL	2
#define db_f(msg,level)		msg_db_f(msg,level+PRESCRIPT_DB_LEVEL)*/

bool type_is_simple_class(Type *t); // -> script_data.cpp

static int PreConstantNr, NamedConstantNr;

inline bool type_match(Type *type, bool is_class, Type *wanted);
inline bool type_match_with_cast(Type *type, bool is_class, bool is_modifiable, Type *wanted, int &penalty, int &cast);
inline void CommandMakeOperator(Command *cmd, Command *p1, Command *p2, int op);

 
#define GetPointerType(sub)	CreateNewType(sub->name + "*", PointerSize, true, false, false, 0, sub)
#define GetReferenceType(sub)	CreateNewType(sub->name + "*", PointerSize, true, true, false, 0, sub)

Command *cp_command(SyntaxTree *ps, Command *c)
{
	Command *cmd = ps->AddCommand();
	*cmd = *c;
	return cmd;
}

Command *cp_command_deep(SyntaxTree *ps, Command *c)
{
	Command *cmd = cp_command(ps, c);
	for (int i=0;i<c->num_params;i++)
		cmd->param[i] = cp_command_deep(ps, c->param[i]);
	return cmd;
}

void command_make_ref(SyntaxTree *ps, Command *c, Command *param)
{
	c->kind = KindReference;
	c->num_params = 1;
	c->param[0] = param;
	c->type = ps->GetPointerType(param->type);
}

void ref_command(SyntaxTree *ps, Command *c)
{
	Command *t = cp_command(ps, c);
	command_make_ref(ps, c, t);
}

void command_make_deref(SyntaxTree *ps, Command *c, Command *param)
{
	c->kind = KindDereference;
	c->num_params = 1;
	c->param[0] = param;
	c->type = param->type->parent;
}

void deref_command(SyntaxTree *ps, Command *c)
{
	Command *t = cp_command(ps, c);
	command_make_deref(ps, c, t);
}

void shift_command(SyntaxTree *ps, Command *c, bool deref, int shift, Type *type)
{
	Command *sub = cp_command(ps, c);
	c->kind = deref ? KindDerefAddressShift : KindAddressShift;
	c->link_nr = shift;
	c->type = type;
	c->num_params = 1;
	c->param[0] = sub;
}

void reset_pre_script(SyntaxTree *ps)
{
	msg_db_f("reset_pre_script", 2);
}

SyntaxTree::SyntaxTree(Script *_script)
{
	Filename = "-empty script-";
	Buffer = "";
	Exp.buffer = NULL;
	FlagShow = false;
	FlagShowPrae = false;
	FlagDisassemble = false;
	FlagCompileOS = false;
	FlagCompileInitialRealMode = false;
	FlagOverwriteVariablesOffset = false;
	FlagImmortal = false;
	FlagNoExecution = false;
	AsmMetaInfo = NULL;
	RootOfAllEvil.name = "RootOfAllEvil";
	RootOfAllEvil.num_params = 0;
	RootOfAllEvil.return_type = TypeVoid;
	cur_func = NULL;
	script = _script;

	// "include" default stuff
	NumOwnTypes = 0;
	for (int i=0;i<PreTypes.num;i++)
		Types.add(PreTypes[i]);
}


void SyntaxTree::LoadAndParseFile(const string &filename, bool just_analyse)
{
	msg_db_f("LoadAndParseFile",4);
	
	Filename = filename.sys_filename();

	LoadToBuffer(Directory + Filename, just_analyse);

	
	PreCompiler(just_analyse);

	Parser();
	
	if (FlagShowPrae)
		Show();

	ConvertCallByReference();

	/*if (FlagShow)
		Show();*/

	Simplify();
	
	PreProcessor(NULL);

	if (FlagShow)
		Show();

	Exp.clear();
}



// ################################################################################################
//                                        Syntax-Analyse
// ################################################################################################


void line_out(SyntaxTree *ps)
{
	char str[1024];
	strcpy(str, ps->Exp.cur_line->exp[0].name);
	for (int i=1;ps->Exp.cur_line->exp.num;i++){
		strcat(str, "  ");
		strcat(str, ps->Exp.cur_line->exp[i].name);
	}
}


char Temp[1024];

static int shift_right=0;

static void stringout(const string &str)
{
	msg_write(str);
}

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

static void right()
{
#ifdef ScriptDebug
	msg_right();
	shift_right+=2;
#endif
}

static void left()
{
#ifdef ScriptDebug
	msg_left();
	shift_right-=2;
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

string Kind2Str(int kind)
{
	if (kind == KindVarLocal)			return "local variable";
	if (kind == KindVarGlobal)			return "global variable";
	if (kind == KindVarFunction)		return "function as variable";
	if (kind == KindVarExternal)		return "external program-variable";
	if (kind == KindConstant)			return "constant";
	if (kind == KindRefToConst)			return "reference to const";
	if (kind == KindFunction)			return "function";
	if (kind == KindCompilerFunction)	return "compiler function";
	if (kind == KindOperator)			return "operator";
	if (kind == KindPrimitiveOperator)	return "PRIMITIVE operator";
	if (kind == KindBlock)				return "command block";
	if (kind == KindAddressShift)		return "address shift";
	if (kind == KindArray)				return "array element";
	if (kind == KindPointerAsArray)		return "pointer as array element";
	if (kind == KindReference)			return "address operator";
	if (kind == KindDereference)		return "dereferencing";
	if (kind == KindDerefAddressShift)	return "deref address shift";
	if (kind == KindType)				return "type";
	if (kind == KindVarTemp)			return "temp";
	if (kind == KindDerefVarTemp)		return "deref temp";
	if (kind == KindRegister)			return "register";
	if (kind == KindAddress)			return "address";
	if (kind == KindMemory)				return "memory";
	if (kind == KindLocalAddress)		return "local address";
	if (kind == KindLocalMemory)		return "local memory";
	if (kind == KindDerefRegister)		return "deref register";
	if (kind == KindMarker)				return "marker";
	if (kind == KindAsmBlock)			return "assembler block";
	if (kind == KindRefToLocal)			return "ref to local";
	if (kind == KindRefToGlobal)		return "ref to global";
	if (kind == KindRefToConst)			return "ref to const";
	if (kind == KindDerefVarLocal)		return "deref local";
	return format("UNKNOWN KIND: %d", kind);
}

string LinkNr2Str(SyntaxTree *s,int kind,int nr)
{
	if (kind == KindVarLocal)			return s->cur_func->var[nr].name;
	if (kind == KindVarGlobal)			return s->RootOfAllEvil.var[nr].name;
	if (kind == KindVarFunction)		return s->Functions[nr]->name;
	if (kind == KindVarExternal)		return PreExternalVars[nr].name;
	if (kind == KindConstant)			return i2s(nr);//s->Constants[nr].type->var2str(s->Constants[nr].data);
	if (kind == KindFunction)			return s->Functions[nr]->name;
	if (kind == KindCompilerFunction)	return PreCommands[nr].name;
	if (kind == KindOperator)			return PreOperators[nr].str();
	if (kind == KindPrimitiveOperator)	return PrimitiveOperators[nr].name;
	if (kind == KindBlock)				return i2s(nr);
	if (kind == KindAddressShift)		return i2s(nr);
	if (kind == KindArray)				return "(no LinkNr)";
	if (kind == KindPointerAsArray)		return "(no LinkNr)";
	if (kind == KindReference)			return "(no LinkNr)";
	if (kind == KindDereference)		return "(no LinkNr)";
	if (kind == KindDerefAddressShift)	return i2s(nr);
	if (kind == KindType)				return s->Types[nr]->name;
	if (kind == KindRegister)			return Asm::GetRegName(nr);
	if (kind == KindAddress)			return d2h(&nr, PointerSize);
	if (kind == KindMemory)				return d2h(&nr, PointerSize);
	if (kind == KindLocalAddress)		return d2h(&nr, PointerSize);
	if (kind == KindLocalMemory)		return d2h(&nr, PointerSize);
	return i2s(nr);
}

void SyntaxTree::DoError(const string &str, int overwrite_line)
{
	// what data do we have?
	int line = -1;
	int pos = 0;
	string expr;
	if (Exp.cur_line){
		line = Exp.cur_line->physical_line;
		if (Exp.cur_exp >= 0){
			expr = Exp.cur;
			pos = Exp.cur_line->exp[Exp.cur_exp].pos;
		}
	}
	if (overwrite_line >= 0){
		line = overwrite_line;
		pos = 0;
	}

	throw Exception(str, expr, line, pos, script);
}

void SyntaxTree::CreateAsmMetaInfo()
{
	msg_db_f("CreateAsmMetaInfo",5);
	//msg_error("zu coden: CreateAsmMetaInfo");
	if (!AsmMetaInfo){
		AsmMetaInfo = new Asm::MetaInfo;
		AsmMetaInfo->Mode16 = FlagCompileInitialRealMode;
		AsmMetaInfo->CodeOrigin = 0; // FIXME:  &Opcode[0] ????
	}
	AsmMetaInfo->Opcode = script->Opcode;
	AsmMetaInfo->global_var.clear();
	for (int i=0;i<RootOfAllEvil.var.num;i++){
		Asm::GlobalVar v;
		v.Name = RootOfAllEvil.var[i].name;
		v.Pos = script->g_var[i];
		AsmMetaInfo->global_var.add(v);
	}
}



bool next_extern = false;
bool next_const = false;

int SyntaxTree::AddVar(const string &name, Type *type, Function *f)
{
/*	// "extern" variable -> link to main program
	if ((next_extern) && (f == &RootOfAllEvil)){
		for (int i=NumTruePreExternalVars;i<PreGlobalVar.num;i++)
			if (strcmp(name, PreGlobalVar[i].Name) == 0){
				f->Var[i].Type = type;
				return i;
			}
	}
should be done somwhere else (ParseVariableDefSingle) */
	so("                                AddVar");
	LocalVariable v;
	v.name = name;
	v.type = type;
	f->var.add(v);
	return f->var.num - 1;
}

// constants

int SyntaxTree::AddConstant(Type *type)
{
	so("                                AddConstant");
	Constants.resize(Constants.num + 1);
	Constant *c = &Constants.back();
	c->name = "-none-";
	c->type = type;
	int s = max(type->size, (int)PointerSize);
	if (type == TypeString)
		s = 256;
	c->data = new char[s];
	return Constants.num - 1;
}

Block *SyntaxTree::AddBlock()
{
	so("AddBlock");
	Block *b = new Block;
	b->index = Blocks.num;
	b->root = -1;
	Blocks.add(b);
	return b;
}

// functions

Function *SyntaxTree::AddFunction(const string &name, Type *type)
{
	so("AddFunction");
	Function *f = new Function();
	Functions.add(f);
	f->name = name;
	f->block = AddBlock();
	f->num_params = 0;
	f->var.clear();
	f->return_type = type;
	f->literal_return_type = type;
	f->_class = NULL;
	return f;
}
Command *SyntaxTree::AddCommand()
{
	so("AddCommand");
	Command *c = new Command;
	Commands.add(c);
	c->type = TypeVoid;
	c->kind = KindUnknown;
	c->num_params = 0;
	c->instance = NULL;
	c->script = script;
	return c;
}


inline Command *add_command_compilerfunc(SyntaxTree *ps, int cf)
{
	Command *c = ps->AddCommand();
	ps->CommandSetCompilerFunction(cf, c);
	return c;
}

inline void CommandSetClassFunc(SyntaxTree *ps, Type *class_type, Command *c, ClassFunction &f, Command *inst)
{
	c->kind = f.kind;
	c->link_nr = f.nr;
	c->instance = inst;
	if (f.kind == KindCompilerFunction){
		c->type = PreCommands[f.nr].return_type;
		//c->NumParams = PreCommand[f.Nr].Param.num;
	}else if (f.kind == KindFunction){
		c->script = class_type->owner->script;
		c->type = class_type->owner->Functions[f.nr]->return_type;
	}
}

inline Command *add_command_classfunc(SyntaxTree *ps, Type *class_type, ClassFunction &f, Command *inst)
{
	Command *c = ps->AddCommand();
	CommandSetClassFunc(ps, class_type, c, f, inst);
	return c;
}

inline void CommandSetConst(SyntaxTree *ps, Command *c, int nc)
{
	c->kind = KindConstant;
	c->link_nr = nc;
	c->type = ps->Constants[nc].type;
	c->num_params = 0;
}

inline Command *add_command_const(SyntaxTree *ps, int nc)
{
	Command *c = ps->AddCommand();
	CommandSetConst(ps, c, nc);
	return c;
}

int SyntaxTree::WhichPrimitiveOperator(const string &name)
{
	for (int i=0;i<NumPrimitiveOperators;i++)
		if (name == PrimitiveOperators[i].name)
			return i;
	return -1;
}

int SyntaxTree::WhichExternalVariable(const string &name)
{
	// wrong order -> "extern" varbiables are dominant...
	for (int i=PreExternalVars.num-1;i>=0;i--)
		if (name == PreExternalVars[i].name)
			return i;

	return -1;
}

int SyntaxTree::WhichType(const string &name)
{
	for (int i=0;i<Types.num;i++)
		if (name == Types[i]->name)
			return i;

	return -1;
}

Array<int> MultipleFunctionList;

int SyntaxTree::WhichCompilerFunction(const string &name)
{
	MultipleFunctionList.clear();
	for (int i=0;i<PreCommands.num;i++)
		if (name == PreCommands[i].name)
			MultipleFunctionList.add(i);
			//return i;
	if (MultipleFunctionList.num > 0)
		return MultipleFunctionList[0];
	return -1;
}

inline void exlink_make_var_local(SyntaxTree *ps, Type *t, int var_no)
{
	ps->GetExistenceLink.type = t;
	ps->GetExistenceLink.link_nr = var_no;
	ps->GetExistenceLink.kind = KindVarLocal;
	ps->GetExistenceLink.num_params = 0;
	ps->GetExistenceLink.script = ps->script;
	ps->GetExistenceLink.instance = NULL;
}

bool SyntaxTree::GetExistence(const string &name, Function *f)
{
	msg_db_f("GetExistence", 3);
	MultipleFunctionList.clear();
	Function *lf=f;
	GetExistenceLink.type = TypeUnknown;
	GetExistenceLink.num_params = 0;
	GetExistenceLink.script = script;
	GetExistenceLink.instance = NULL;

	// first test local variables
	if (lf){
		foreachi(LocalVariable &v, f->var, i){
			if (v.name == name){
				exlink_make_var_local(this, v.type, i);
				return true;
			}
		}
	}

	// then global variables (=local variables in "RootOfAllEvil")
	lf = &RootOfAllEvil;
	foreachi(LocalVariable &v, lf->var, i)
		if (v.name == name){
			GetExistenceLink.type = v.type;
			GetExistenceLink.link_nr = i;
			GetExistenceLink.kind = KindVarGlobal;
			return true;
		}

	// at last the external variables
	int w = WhichExternalVariable(name);
	if (w >= 0){
		SetExternalVariable(w, &GetExistenceLink);
		return true;
	}

	// then the (self-coded) functions
	foreachi(Function *f, Functions, i)
		if (f->name == name){
			GetExistenceLink.kind = KindFunction;
			GetExistenceLink.link_nr = i;
			GetExistenceLink.type = f->literal_return_type;
			GetExistenceLink.num_params = f->num_params;
			return true;
		}

	// then the compiler functions
	w = WhichCompilerFunction(name);
	if (w >= 0){
		GetExistenceLink.kind = KindCompilerFunction;
		GetExistenceLink.link_nr = w;
		GetExistenceLink.type = PreCommands[w].return_type;
		GetExistenceLink.num_params = PreCommands[w].param.num;
		return true;
	}

	// operators
	w = WhichPrimitiveOperator(name);
	if (w >= 0){
		GetExistenceLink.kind = KindPrimitiveOperator;
		GetExistenceLink.link_nr = w;
		return true;
	}

	// types
	w = WhichType(name);
	if (w >= 0){
		GetExistenceLink.kind = KindType;
		GetExistenceLink.link_nr = w;
		return true;
	}

	// in include files (only global)...
	foreach(Script *i, Includes)
		if (i->syntax->GetExistence(name, NULL)){
			if (i->syntax->GetExistenceLink.script != i) // nicht rekursiv!!!
				continue;
			memcpy(&GetExistenceLink, &(i->syntax->GetExistenceLink), sizeof(Command));
			GetExistenceLink.script = i;
			//msg_error(string2("\"%s\" in Include gefunden!  %s", name, GetExistenceLink.Type->Name));
			return true;
		}

	// ...unknown
	GetExistenceLink.type = TypeUnknown;
	GetExistenceLink.kind = KindUnknown;
	GetExistenceLink.link_nr = 0;
	return false;
}

void SyntaxTree::CommandSetCompilerFunction(int CF, Command *Com)
{
	msg_db_f("CommandSetCompilerFunction", 4);
	if (FlagCompileOS)
		if (!PreCommands[CF].is_special)
			DoError(format("external function call (%s) not allowed with #os", PreCommands[CF].name.c_str()));
	
// a function the compiler knows
	Com->kind = KindCompilerFunction;
	Com->link_nr = CF;

	Com->num_params = PreCommands[CF].param.num;
	for (int p=0;p<Com->num_params;p++){
		Com->param[p] = AddCommand(); // temporary...
		Com->param[p]->type = PreCommands[CF].param[p].type;
	}
	Com->type = PreCommands[CF].return_type;
}

#define is_variable(kind)	(((kind) == KindVarLocal) || ((kind) == KindVarGlobal) || ((kind) == KindVarExternal))

void SyntaxTree::SetExternalVariable(int gv, Command *c)
{
	c->num_params = 0;
	c->kind = KindVarExternal;
	c->link_nr = gv;
	c->type = PreExternalVars[gv].type;
}

// find the type of a (potential) constant
//  "1.2" -> float
Type *SyntaxTree::GetConstantType()
{
	msg_db_f("GetConstantType", 4);
	PreConstantNr = -1;
	NamedConstantNr = -1;

	// predefined constants
	foreachi(PreConstant &c, PreConstants, i)
		if (Exp.cur == c.name){
			PreConstantNr = i;
			return c.type;
		}

	// named constants
	foreachi(Constant &c, Constants, i)
		if (Exp.cur == c.name){
			NamedConstantNr = i;
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
				//if ((type != TypeFloat) || (Exp.cur[c] != 'f')) // f in floats erlauben
					if ((c != 0) || (Exp.cur[c] != '-')) // Vorzeichen erlauben
						return TypeUnknown;
			}
		}

	// super array [...]
	if (Exp.cur == "["){
		//msg_error("super array constant");
		DoError("super array constant");
	}
	return type;
}

static int _some_int_;
static float _some_float_;
static char _some_string_[2048];

bool pre_const_is_ref(Type *type)
{
	return ((type->size > 4) && (!type->is_pointer));
}

void *SyntaxTree::GetConstantValue()
{
	Type *type = GetConstantType();
// named constants
	if (PreConstantNr >= 0){
		if (pre_const_is_ref(type))
			return PreConstants[PreConstantNr].value;
		else
			return &PreConstants[PreConstantNr].value;
	}
	if (NamedConstantNr >= 0)
		return Constants[NamedConstantNr].data;
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

// expression naming a type
Type *SyntaxTree::GetType(const string &name,bool force)
{
	Type *type=NULL;
	for (int i=0;i<Types.num;i++)
		if (name == Types[i]->name)
			type = Types[i];
	if (force){
		if (!type){
			DoError("unknown type");
			return NULL;
		}
	}
	if (type)
		Exp.next();
	return type;
}

// create a new type?
void SyntaxTree::AddType(Type **type)
{
	for (int i=0;i<Types.num;i++)
		if ((*type)->name == Types[i]->name){
			(*type)=Types[i];
			return;
		}
	Type *t = new Type;
	(*t) = (**type);
	t->owner = this;
	t->name = (*type)->name;
	so("AddType: " + t->name);
	(*type) = t;
	Types.add(t);
	NumOwnTypes ++;

	if (t->is_super_array)
		script_make_super_array(t, this);
}

Type *SyntaxTree::CreateNewType(const string &name, int size, bool is_pointer, bool is_silent, bool is_array, int array_size, Type *sub)
{
	Type nt, *pt = &nt;
	nt.is_array = is_array && (array_size >= 0);
	nt.is_super_array = is_array && (array_size < 0);
	nt.array_length = max(array_size, 0);
	nt.is_pointer = is_pointer;
	nt.is_silent = is_silent;
	nt.name = name;
	nt.size = size;
	nt.parent = sub;
	AddType(&pt);
	return pt;
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
	Command *ob = cp_command(ps, Operand);

	//msg_write(LinkNr2Str(ps, Operand->Kind, Operand->Nr));

	// the function
	Operand->script = t->owner->script;
    Operand->kind = t->function[f_no].kind;
	Operand->link_nr = t->function[f_no].nr;
	if (t->function[f_no].kind == KindCompilerFunction){
		Operand->type = PreCommands[t->function[f_no].nr].return_type;
		Operand->num_params = PreCommands[t->function[f_no].nr].param.num;
		ps->GetFunctionCall(PreCommands[t->function[f_no].nr].name, Operand, f);
	}else if (t->function[f_no].kind == KindFunction){
		Operand->type = t->owner->Functions[t->function[f_no].nr]->literal_return_type;
		Operand->num_params = t->owner->Functions[t->function[f_no].nr]->num_params;
		ps->GetFunctionCall(t->owner->Functions[t->function[f_no].nr]->name, Operand, f);
		//ps->DoError("script member function call not implemented");
	}
	Operand->instance = ob;
}

void SyntaxTree::GetOperandExtensionElement(Command *Operand, Function *f)
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
	bool ok = false;
	for (int e=0;e<type->element.num;e++)
		if (Exp.cur == type->element[e].name){
			shift_command(this, Operand, deref, type->element[e].offset, type->element[e].type);
			ok = true;
			break;
		}

	// class function?
	if (!ok){
		for (int e=0;e<type->function.num;e++)
			if (Exp.cur == type->function[e].name){
				if (!deref)
					ref_command(this, Operand);
				Exp.next();
				DoClassFunction(this, Operand, type, e, f);
				ok = true;
				Exp.rewind();
				break;
			}
	}

	if (!ok)
		DoError("unknown element of " + type->name);

	Exp.next();
}

void SyntaxTree::GetOperandExtensionArray(Command *Operand, Function *f)
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

	Command *t = cp_command(this, Operand);
	Operand->num_params = 2;
	Operand->param[0] = t;
	Command *array = Operand;

	// pointer?
	so(Operand->type->name);
	if (pparray){
		DoError("test... anscheinend gibt es [] auf * super array");
		//array = cp_command(this, Operand);
		Operand->kind = KindPointerAsArray;
		Operand->type = t->type->parent;
		deref_command(this, Operand);
		array = Operand->param[0];
	}else if (Operand->type->is_super_array){
		Operand->kind = KindPointerAsArray;
		Operand->type = t->type->parent;
		shift_command(this, t, false, 0, GetPointerType(t->type->parent));
	}else if (Operand->type->is_pointer){
		Operand->kind = KindPointerAsArray;
		Operand->type = t->type->parent->parent;
	}else{
		Operand->kind = KindArray;
		Operand->type = t->type->parent;
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
}

// find any ".", "->", or "[...]"'s    or operators?
void SyntaxTree::GetOperandExtension(Command *Operand, Function *f)
{
	msg_db_f("GetOperandExtension", 4);

	// nothing?
	int op = WhichPrimitiveOperator(Exp.cur);
	if ((Exp.cur != ".") && (Exp.cur != "[") && (Exp.cur != "->") && (op < 0))
		return;

	if (Exp.cur == "->")
		DoError("\"->\" deprecated,  use \".\" instead");

	if (Exp.cur == "."){
		// class element?

		GetOperandExtensionElement(Operand, f);

	}else if (Exp.cur == "["){
		// array?

		GetOperandExtensionArray(Operand, f);


	}else if (op >= 0){
		// unary operator?

		for (int i=0;i<PreOperators.num;i++)
			if (PreOperators[i].primitive_id == op)
				if ((PreOperators[i].param_type_1 == Operand->type) && (PreOperators[i].param_type_2 == TypeVoid)){
					so("  => unaerer Operator");
					so(LinkNr2Str(this,KindOperator,i));
					Command *t = cp_command(this, Operand);
					CommandMakeOperator(Operand, t, NULL, i);
					Exp.next();
					return;
				}
		return;
	}

	// recursion
	GetOperandExtension(Operand, f);
}

inline bool direct_type_match(Type *a, Type *b)
{
	return ( (a==b) || ( (a->is_pointer) && (b->is_pointer) ) );
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
		else if ((GetExistence(Exp.cur, f)) && ((GetExistenceLink.kind == KindVarGlobal) || (GetExistenceLink.kind == KindVarLocal) || (GetExistenceLink.kind == KindVarExternal)))
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

			ref_command(this, link);
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
		ref_command(this, Operand);
	}else if (Exp.cur == "*"){ // * -> dereference
		so("<Dereferenzierung *>");
		Exp.next();
		Operand = GetOperand(f);
		if (!Operand->type->is_pointer){
			Exp.rewind();
			DoError("only pointers can be dereferenced using \"*\"");
		}
		deref_command(this, Operand);
	}else{
		// direct operand
		if (GetExistence(Exp.cur, f)){
			Operand = cp_command(this, &GetExistenceLink);
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
				CommandMakeOperator(Operand, sub_command, NULL, o);
				so(PreOperators[o].str());
				return Operand;
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
	GetOperandExtension(Operand,f);

	so("Operand endet mit " + Exp.get_name(Exp.cur_exp - 1));
	return Operand;
}

// only "primitive" operator -> no type information
Command *SyntaxTree::GetOperator(Function *f)
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

inline void CommandMakeOperator(Command *cmd, Command *p1, Command *p2, int op)
{
	cmd->kind = KindOperator;
	cmd->link_nr = op;
	cmd->num_params = ((PreOperators[op].param_type_1 == TypeVoid) || (PreOperators[op].param_type_2 == TypeVoid)) ? 1 : 2; // unary / binary
	cmd->param[0] = p1;
	cmd->param[1] = p2;
	cmd->type = PreOperators[op].return_type;
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
		Command *sub_cmd = cp_command(ps, param);
		ps->CommandSetCompilerFunction(TypeCasts[tc].command, param);
		param->param[0] = sub_cmd;
		so("  ...keine Konstante: Wandel-Befehl wurde hinzugefuegt!");
	}
}

bool SyntaxTree::LinkOperator(int op_no, Command *param1, Command *param2, Command **cmd)
{
	msg_db_f("LinkOp",4);
	bool left_modifiable = PrimitiveOperators[op_no].left_modifiable;
	string op_func_name = PrimitiveOperators[op_no].function_name;

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
				ref_command(this, inst);
				CommandSetClassFunc(this, p1, *cmd, f, inst);
				(*cmd)->num_params = 1;
				(*cmd)->param[0] = param2;
				return true;
			}
		}

	// exact match?
	for (int i=0;i<PreOperators.num;i++)
		if (op_no == PreOperators[i].primitive_id)
			if (type_match(p1, equal_classes, PreOperators[i].param_type_1) && type_match(p2, equal_classes, PreOperators[i].param_type_2)){
				CommandMakeOperator(*cmd, param1, param2, i);
				return true;
			}

	// exact match as class function but missing a "&"?
	foreach(ClassFunction &f, p1->function)
		if (f.name == op_func_name){
			if (f.param_type[0]->is_pointer && f.param_type[0]->is_silent)
				if (direct_type_match(p2, f.param_type[0]->parent)){
					Command *inst = param1;
					ref_command(this, inst);
					CommandSetClassFunc(this, p1, *cmd, f, inst);
					(*cmd)->num_params = 1;
					(*cmd)->param[0] = param2;
					ref_command(this, (*cmd)->param[0]);
					return true;
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
			ref_command(this, inst);
			CommandSetClassFunc(this, p1, *cmd, p1->function[op_found], inst);
			(*cmd)->num_params = 1;
			(*cmd)->param[0] = param2;
		}else{
			CommandMakeOperator(*cmd, param1, param2, op_found);
		}
		return true;
	}

	return false;
}

void SyntaxTree::LinkMostImportantOperator(int &NumOperators, Command **Operand, Command **Operator, int *op_exp)
{
	msg_db_f("LinkMostImpOp",4);
// find the most important operator (mio)
	int mio=0;
	for (int i=0;i<NumOperators;i++){
		so(format("%d %d", Operator[i]->link_nr, Operator[i]->link_nr));
		if (PrimitiveOperators[Operator[i]->link_nr].level > PrimitiveOperators[Operator[mio]->link_nr].level)
			mio=i;
	}
	so(mio);

// link it
	Command *param1 = Operand[mio];
	Command *param2 = Operand[mio + 1];
	int op_no = Operator[mio]->link_nr;
	if (!LinkOperator(op_no, param1, param2, &Operator[mio])){
		Exp.cur_exp = op_exp[mio];
		DoError(format("no operator found: (%s) %s (%s)", param1->type->name.c_str(), PrimitiveOperators[op_no].name.c_str(), param2->type->name.c_str()));
	}

// remove from list
	Operand[mio]=Operator[mio];
	for (int i=mio;i<NumOperators-1;i++){
		Operator[i]=Operator[i+1];
		Operand[i+1]=Operand[i+2];
		op_exp[i] = op_exp[i+1];
	}
	NumOperators--;
}

Command *SyntaxTree::GetCommand(Function *f)
{
	msg_db_f("GetCommand", 4);
	int NumOperands = 0;
	Array<Command*> Operand;
	Array<Command*> Operator;
	Array<int> op_exp;

	// find the first operand
	Operand.add(GetOperand(f));
	NumOperands ++;

	// find pairs of operators and operands
	for (int i=0;true;i++){
		op_exp.add(Exp.cur_exp);
		Command *op = GetOperator(f);
		if (op){
			Operator.add(op);
			if (Exp.end_of_line()){
				//Exp.rewind();
				DoError("unexpected end of line after operator");
			}
			Operand.add(GetOperand(f));
			NumOperands++;
		}else{
			so("(kein weiterer Operator)");
			so(Exp.cur);
			break;
		}
	}


	// in each step remove/link the most important operator
	int NumOperators=NumOperands-1;
	for (int i=0;i<NumOperands-1;i++){
		LinkMostImportantOperator(NumOperators, &Operand[0], &Operator[0], &op_exp[0]);
	}

	Command *ret = Operand[0];
	Operand.clear();
	Operator.clear();
	op_exp.clear();

	// complete command is now collected in Operand[0]

	so("-fertig");
	so("Command endet mit " + Exp.get_name(Exp.cur_exp - 1));
	return ret;
}

void conv_cbr(SyntaxTree *ps, Command *&c, int var);

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
 			for_var = cp_command(this, &GetExistenceLink);
		}else{
			GetExistence(Exp.cur, f);
 			for_var = cp_command(this, &GetExistenceLink);
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
		Command *cmd_assign = AddCommand();
		CommandMakeOperator(cmd_assign, for_var, val0, OperatorIntAssign);
		block->command.add(cmd_assign);
			
		// while(for_var < val1)
		Command *cmd_cmp = AddCommand();
		CommandMakeOperator(cmd_cmp, for_var, val1, OperatorIntSmaller);
			
		Command *cmd_while = add_command_compilerfunc(this, CommandFor);
		cmd_while->param[0] = cmd_cmp;
		block->command.add(cmd_while);
		ExpectNewline();
		// ...block
		Exp.next_line();
		ExpectIndent();
		int loop_block_no = Blocks.num; // should get created...soon
		GetCompleteCommand(block, f);
			
		// ...for_var += 1
		Command *cmd_inc = AddCommand();
		if (for_var->type == TypeInt){
			CommandMakeOperator(cmd_inc, for_var, val1 /*dummy*/, OperatorIntIncrease);
		}else{
			int nc = AddConstant(TypeFloat);
			*(float*)Constants[nc].data = 1.0;
			Command *val_add = add_command_const(this, nc);
			CommandMakeOperator(cmd_inc, for_var, val_add, OperatorFloatAddS);
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
 		Command *for_index = cp_command(this, &GetExistenceLink);
		
		// variable
		Exp.next();
		string var_name = Exp.cur;
		Exp.next();

		// super array
		if (Exp.cur != "in")
			DoError("\"in\" expected after variable in forall");
		Exp.next();
		GetExistence(Exp.cur, f);
		Command *for_array = cp_command(this, &GetExistenceLink);
		if ((!is_variable(for_array->kind)) || (!for_array->type->is_super_array))
			DoError("list variable expected as second parameter in \"forall\"");
		Exp.next();

		// variable...
		Type *var_type = for_array->type->parent;
		int var_no = AddVar(var_name, var_type, f);
		exlink_make_var_local(this, var_type, var_no);
 		Command *for_var = cp_command(this, &GetExistenceLink);

		// 0
		int nc = AddConstant(TypeInt);
		*(int*)Constants[nc].data = 0;
		Command *val0 = add_command_const(this, nc);

		// implement
		// for_index = 0
		Command *cmd_assign = AddCommand();
		CommandMakeOperator(cmd_assign, for_index, val0, OperatorIntAssign);
		block->command.add(cmd_assign);

		// array.num
		Command *val1 = AddCommand();
		val1->kind = KindAddressShift;
		val1->link_nr = PointerSize;
		val1->type = TypeInt;
		val1->num_params = 1;
		val1->param[0] = for_array;
			
		// while(for_index < val1)
		Command *cmd_cmp = AddCommand();
		CommandMakeOperator(cmd_cmp, for_index, val1, OperatorIntSmaller);
			
		Command *cmd_while = add_command_compilerfunc(this, CommandFor);
		cmd_while->param[0] = cmd_cmp;
		block->command.add(cmd_while);
		ExpectNewline();
		// ...block
		Exp.next_line();
		ExpectIndent();
		int loop_block_no = Blocks.num; // should get created...soon
		GetCompleteCommand(block, f);
			
		// ...for_index += 1
		Command *cmd_inc = AddCommand();
		CommandMakeOperator(cmd_inc, for_index, val1 /*dummy*/, OperatorIntIncrease);
		Block *loop_block = Blocks[loop_block_no];
		loop_block->command.add(cmd_inc); // add to loop-block

		// &for_var
		Command *for_var_ref = AddCommand();
		command_make_ref(this, for_var_ref, for_var);

		// &array[for_index]
		Command *array_el = AddCommand();
		array_el->kind = KindPointerAsArray;
		array_el->num_params = 2;
		array_el->param[0] = for_array;
		array_el->param[1] = for_index;
		array_el->type = var_type;
		Command *array_el_ref = AddCommand();
		command_make_ref(this, array_el_ref, array_el);

		// &for_var = &array[for_index]
		Command *cmd_var_assign = AddCommand();
		CommandMakeOperator(cmd_var_assign, for_var_ref, array_el_ref, OperatorPointerAssign);
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
			
		Command *cmd_while = add_command_compilerfunc(this, CommandWhile);
		cmd_while->param[0] = cmd_cmp;
		block->command.add(cmd_while);
		// ...block
		Exp.next_line();
		ExpectIndent();
		GetCompleteCommand(block, f);
 	}else if (Exp.cur == "break"){
		Exp.next();
		Command *cmd = add_command_compilerfunc(this, CommandBreak);
		block->command.add(cmd);
	}else if (Exp.cur == "continue"){
		Exp.next();
		Command *cmd = add_command_compilerfunc(this, CommandContinue);
		block->command.add(cmd);
	}else if (Exp.cur == "if"){
		int ind = Exp.cur_line->indent;
		Exp.next();
		Command *cmd_cmp = GetCommand(f);
		CheckParamLink(cmd_cmp, TypeBool, "if", 0);
		ExpectNewline();
			
		Command *cmd_if = add_command_compilerfunc(this, CommandIf);
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
		Command *c = add_command_compilerfunc(this, CommandAsm);
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
			(*type) = CreateNewType(	or_name + "[]" +  (*type)->name.substr(or_name_length, -1),
			                        	SuperArraySize, false, false, true, array_size, (*type));
			CreateImplicitFunctions((*type), cur_func);
		}else{
			(*type) = CreateNewType(	or_name + format("[%d]", array_size) + (*type)->name.substr(or_name_length, -1),
			                        	(*type)->size * array_size, false, false, true, array_size, (*type));
		}
		if (Exp.cur == "*"){
			so("nachtraeglich Pointer");
			Exp.next();
			TestArrayDefinition(type, true);
		}
	}
}


// read the file and do a lexical analysis
void SyntaxTree::LoadToBuffer(const string &filename,bool just_analyse)
{
	msg_db_f("LoadToBuffer",4);

// read file
	CFile *f = OpenFile(filename);
	if (!f){
		Exp.cur_line = NULL;
		DoError("script file not loadable");
	}
	Buffer = f->ReadComplete();
	FileClose(f);

	Exp.Analyse(this, Buffer.c_str());


	Buffer.clear();
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
		PreCommand *c = &PreCommands[ExternalFuncPreCommandIndex];
		cf.name = c->name.substr(t->name.num + 1, -1);
		cf.kind = KindCompilerFunction;
		cf.nr = ExternalFuncPreCommandIndex;
		cf.return_type = c->return_type;
		foreach(PreCommandParam &p, c->param)
			cf.param_type.add(p.type);
	}else{
		Function *f = Functions.back();
		cf.name = f->name.substr(t->name.num + 1, -1);
		cf.kind = KindFunction;
		cf.nr = Functions.num - 1;
		cf.return_type = f->return_type;
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
				_offset = mem_align(_offset);
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
			_offset = mem_align(_offset);
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

void AddExternalVar(const string &name, Type *type)
{
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
	if (next_extern)
		AddExternalVar(name, type);
	else if (next_const){
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

void CopyFuncDataToExternal(Function *f, PreCommand *c, bool is_class_func)
{
	c->is_class_function = is_class_func;
	c->return_type = f->return_type;
	c->param.clear();
	for (int j=0;j<f->num_params;j++){
		PreCommandParam p;
		p.name = f->var[j].name;
		p.type = f->var[j].type;
		c->param.add(p);
	}
}

void AddExternalFunc(SyntaxTree *ps, Function *f, Type *class_type)
{
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
}

void SyntaxTree::ParseFunction(Type *class_type, bool as_extern)
{
	msg_db_f("ParseFunction", 4);
	
// return type
	Type *type = GetType(Exp.cur, true);

	// pointer?
	if (Exp.cur == "*"){
		Exp.next();
		type = GetPointerType(type);
	}

	so(Exp.cur);
	Function *f = AddFunction(Exp.cur, type);
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
	shift_right = 0;

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

		// enum
		if (Exp.cur == "enum"){
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

void conv_cbr(SyntaxTree *ps, Command *&c, int var)
{
	msg_db_f("conv_cbr", 4);
	//so(Kind2Str(c->Kind));
	
	// recursion...
	so(c->num_params);
	for (int i=0;i<c->num_params;i++)
		conv_cbr(ps, c->param[i], var);
	if (c->kind == KindBlock){
		foreach(Command *cc, ps->Blocks[c->link_nr]->command)
			conv_cbr(ps, cc, var);
	}
	if (c->instance)
		conv_cbr(ps, c->instance, var);
	so("a");

	// convert
	if ((c->kind == KindVarLocal) && (c->link_nr == var)){
		so("conv");
		c = cp_command(ps, c);
		c->type = ps->GetPointerType(c->type);
		deref_command(ps, c);
	}
}

#if 0
void conv_return(SyntaxTree *ps, command *c)
{
	// recursion...
	for (int i=0;i<c->num_params;i++)
		conv_return(ps, c->param[i]);
	
	if ((c->kind == KindCompilerFunction) && (c->link_nr == CommandReturn)){
		msg_write("conv ret");
		ref_command(ps, c);
	}
}
#endif


// remove &*x and (*x)[] and (*x).y
void easyfy(SyntaxTree *ps, Command *c, int l)
{
	msg_db_f("easyfy", 4);
	//msg_write(l);
	//msg_write("a");
	
	// recursion...
	for (int i=0;i<c->num_params;i++)
		easyfy(ps, c->param[i], l+1);
	if (c->kind == KindBlock)
		for (int i=0;i<ps->Blocks[c->link_nr]->command.num;i++)
			easyfy(ps, ps->Blocks[c->link_nr]->command[i], l+1);
	if (c->instance)
		easyfy(ps, c->instance, l+1);
	
	//msg_write("b");


	// convert
	if (c->kind == KindReference){
		if (c->param[0]->kind == KindDereference){
			so("rem 2");
			// remove 2 knots...
			Command *t = c->param[0]->param[0];
			*c = *t;
		}
	}else if ((c->kind == KindAddressShift) || (c->kind == KindArray)){
		if (c->param[0]->kind == KindDereference){
			so("rem 1 (unify)");
			// unify 2 knots (remove 1)
			Command *t = c->param[0]->param[0];
			c->kind = (c->kind == KindAddressShift) ? KindDerefAddressShift : KindPointerAsArray;
			c->param[0] = t;
		}
	}
	//msg_write("ok");
}

// convert "source code"...
//    call by ref params:  array, super array, class
//    return by ref:       array
void SyntaxTree::ConvertCallByReference()
{
	msg_db_f("ConvertCallByReference", 2);


	// convert functions
	foreach(Function *f, Functions){
		
		// parameter: array/class as reference
		for (int j=0;j<f->num_params;j++)
			if (f->var[j].type->UsesCallByReference()){
				f->var[j].type = GetPointerType(f->var[j].type);

				// internal usage...
				foreach(Command *c, f->block->command)
					conv_cbr(this, c, j);
			}

		// return: array as reference
		if ((f->return_type->is_array) /*|| (f->Type->IsSuperArray)*/){
			f->return_type = GetPointerType(f->return_type);
			/*for (int k=0;k<f->Block->Command.num;k++)
				conv_return(this, f->Block->Command[k]);*/
			// no need... return gets converted automatically (all calls...)
		}
	}

	msg_db_m("a", 3);

	// convert function calls
	foreach(Command *ccc, Commands){
		// Command array might be reallocated in the loop!
		Command *c = ccc;

		if (c->kind == KindCompilerFunction)
			if (c->link_nr == CommandReturn){
				if ((c->param[0]->type->is_array) /*|| (c->Param[j]->Type->IsSuperArray)*/){
					so("conv param (return)");
					so(c->param[0]->type->name);
					ref_command(this, c->param[0]);
				}
				_foreach_it_.update(); // TODO badness10000!!!!!!!!
				continue;
			}
		
		if ((c->kind == KindFunction)|| (c->kind == KindCompilerFunction)){
			// parameters: array/class as reference
			for (int j=0;j<c->num_params;j++)
				if (c->param[j]->type->UsesCallByReference()){
					so("conv param");
					so(c->param[j]->type->name);
					ref_command(this, c->param[j]);
				}

			// return: array reference (-> dereference)
			if ((c->type->is_array) /*|| (c->Type->IsSuperArray)*/){
				so("conv ret");
				so(c->type->name);
				c->type = GetPointerType(c->type);
				deref_command(this, c);
			}	
		}

		// special string / list operators
		if (c->kind == KindOperator){
			// parameters: super array as reference
			for (int j=0;j<c->num_params;j++)
				if ((c->param[j]->type->is_array) || (c->param[j]->type->is_super_array)){
					so("conv param (op)");
					so(c->param[j]->type->name);
					ref_command(this, c->param[j]);
				}
		}
		_foreach_it_.update(); // TODO !!!!!
	}
}


void SyntaxTree::Simplify()
{
	msg_db_f("Simplify", 2);
	
	// remove &*
	foreach(Function *f, Functions)
		foreach(Command *c, f->block->command)
			easyfy(this, c, 0);
}

// split arrays and address shifts into simpler commands...
void SyntaxTree::BreakDownComplicatedCommands()
{
	msg_db_f("BreakDownComplicatedCommands", 4);
	
	for (int i=0;i<Commands.num;i++){
		Command *c = Commands[i];
		if (c->kind == KindArray){
			so("array");

			Type *el_type = c->type;

// array el -> array
//          -> index
//
// * -> + -> & array
//        -> * -> size
//             -> index

			Command *c_index = c->param[1];
			// & array
			Command *c_ref_array = cp_command(this, c->param[0]);
			ref_command(this, c_ref_array);
			// create command for size constant
			int nc = AddConstant(TypeInt);
			*(int*)Constants[nc].data = el_type->size;
			Command *c_size = add_command_const(this, nc);
			// offset = size * index
			Command *c_offset = AddCommand();
			CommandMakeOperator(c_offset, c_index, c_size, OperatorIntMultiply);
			c_offset->type = TypeInt;//TypePointer;
			// address = &array + offset
			Command *c_address = AddCommand();
			CommandMakeOperator(c_address, c_ref_array, c_offset, OperatorIntAdd);
			c_address->type = GetPointerType(el_type);//TypePointer;
			// c = * address
			command_make_deref(this, c, c_address);
			c->type = el_type;
		}else if (c->kind == KindPointerAsArray){
			so("array");

			Type *el_type = c->type;

// array el -> array_pointer
//          -> index
//
// * -> + -> array_pointer
//        -> * -> size
//             -> index

			Command *c_index = c->param[1];
			Command *c_ref_array = c->param[0];
			// create command for size constant
			int nc = AddConstant(TypeInt);
			*(int*)Constants[nc].data = el_type->size;
			Command *c_size = add_command_const(this, nc);
			// offset = size * index
			Command *c_offset = AddCommand();
			CommandMakeOperator(c_offset, c_index, c_size, OperatorIntMultiply);
			c_offset->type = TypeInt;
			// address = &array + offset
			Command *c_address = AddCommand();
			CommandMakeOperator(c_address, c_ref_array, c_offset, OperatorIntAdd);
			c_address->type = GetPointerType(el_type);//TypePointer;
			// c = * address
			command_make_deref(this, c, c_address);
			c->type = el_type;
		}else if (c->kind == KindAddressShift){
			so("address shift");

			Type *el_type = c->type;

// struct el -> struct
//           -> shift (LinkNr)
//
// * -> + -> & struct
//        -> shift

			// & struct
			Command *c_ref_struct = cp_command(this, c->param[0]);
			ref_command(this, c_ref_struct);
			// create command for shift constant
			int nc = AddConstant(TypeInt);
			*(int*)Constants[nc].data = c->link_nr;
			Command *c_shift = add_command_const(this, nc);
			// address = &struct + shift
			Command *c_address = AddCommand();
			CommandMakeOperator(c_address, c_ref_struct, c_shift, OperatorIntAdd);
			c_address->type = GetPointerType(el_type);//TypePointer;
			// c = * address
			command_make_deref(this, c, c_address);
			c->type = el_type;
		}else if (c->kind == KindDerefAddressShift){
			so("deref address shift");

			Type *el_type = c->type;

// struct el -> struct_pointer
//           -> shift (LinkNr)
//
// * -> + -> struct_pointer
//        -> shift

			Command *c_ref_struct = c->param[0];
			// create command for shift constant
			int nc = AddConstant(TypeInt);
			*(int*)Constants[nc].data = c->link_nr;
			Command *c_shift = add_command_const(this, nc);
			// address = &struct + shift
			Command *c_address = AddCommand();
			CommandMakeOperator(c_address, c_ref_struct, c_shift, OperatorIntAdd);
			c_address->type = GetPointerType(el_type);//TypePointer;
			// c = * address
			command_make_deref(this, c, c_address);
			c->type = el_type;			
		}			
	}
}

void SyntaxTree::MapLocalVariablesToStack()
{
	msg_db_f("MapLocalVariablesToStack", 1);
	foreach(Function *f, Functions){
		f->_param_size = 2 * PointerSize; // space for return value and eBP
		if (Asm::InstructionSet.set == Asm::InstructionSetX86){
			if (f->return_type->size > 4)
				f->_param_size += 4;
			f->_var_size = 0;

			// map "self" to the first parameter
			if (f->_class){
				foreachi(LocalVariable &v, f->var, i)
					if (v.name == "self"){
						int s = mem_align(v.type->size);
						v._offset = f->_param_size;
						f->_param_size += s;
					}
			}

			foreachi(LocalVariable &v, f->var, i){
				if ((f->_class) && (v.name == "self"))
					continue;
				int s = mem_align(v.type->size);
				if (i < f->num_params){
					// parameters
					v._offset = f->_param_size;
					f->_param_size += s;
				}else{
					// "real" local variables
					v._offset = - f->_var_size - s;
					f->_var_size += s;
				}				
			}
		}else if (Asm::InstructionSet.set == Asm::InstructionSetAMD64){
			f->_var_size = 0;
			
			foreachi(LocalVariable &v, f->var, i){
				int s = mem_align(v.type->size);
				v._offset = - f->_var_size - s;
				f->_var_size += s;
			}
		}
	}
}

void CreateImplicitConstructor(SyntaxTree *ps, Type *t)
{
	// create function
	Function *f = ps->AddFunction(t->name + ".__init__", TypeVoid);
	int fn = ps->Functions.num - 1;
	f->_class = t;
	ps->AddVar("self", ps->GetPointerType(t), f);

	Command *self = ps->AddCommand();
	self->kind = KindVarLocal;
	self->link_nr = 0;
	self->type = ps->GetPointerType(t);

	if (t->is_super_array){
		foreach(ClassFunction &ff, t->function)
			if (ff.name == "__mem_init__"){
				int nc = ps->AddConstant(TypeInt);
				*(int*)ps->Constants[nc].data = t->parent->size;
				Command *c = add_command_classfunc(ps, t, ff, self);
				Command *p = add_command_const(ps, nc);
				c->param[0] = p;
				c->num_params = 1;
				f->block->command.add(c);
			}
	}else{

		// call child constructors
		foreach(ClassElement &e, t->element)
			foreach(ClassFunction &ff, e.type->function)
				if (ff.name == "__init__"){
					Command *p = ps->AddCommand();
					p->kind = KindDerefAddressShift;
					p->link_nr = e.offset;
					p->type = e.type;
					p->num_params = 1;
					p->param[0] = self;
					ref_command(ps, p);
					Command *c = add_command_classfunc(ps, t, ff, p);
					f->block->command.add(c);
				}
	}

	ClassFunction cf;
	cf.kind = KindFunction;
	cf.nr = fn;
	cf.name = "__init__";
	cf.return_type = TypeVoid;
	t->function.add(cf);
}

void CreateImplicitDestructor(SyntaxTree *ps, Type *t)
{
	// create function
	Function *f = ps->AddFunction(t->name + ".__delete__", TypeVoid);
	int fn = ps->Functions.num - 1;
	f->_class = t;
	ps->AddVar("self", ps->GetPointerType(t), f);

	Command *self = ps->AddCommand();
	self->kind = KindVarLocal;
	self->link_nr = 0;
	self->type = ps->GetPointerType(t);

	if (t->is_super_array){
		foreach(ClassFunction &ff, t->function)
			if (ff.name == "clear"){
				Command *c = add_command_classfunc(ps, t, ff, self);
				f->block->command.add(c);
			}
	}else{

		// call child destructors
		foreach(ClassElement &e, t->element)
			foreach(ClassFunction &ff, e.type->function)
				if (ff.name == "__delete__"){
					Command *p = ps->AddCommand();
					p->kind = KindDerefAddressShift;
					p->link_nr = e.offset;
					p->type = e.type;
					p->num_params = 1;
					p->param[0] = self;
					ref_command(ps, p);
					Command *c = add_command_classfunc(ps, t, ff, p);
					f->block->command.add(c);
				}
	}


	ClassFunction cf;
	cf.kind = KindFunction;
	cf.nr = fn;
	cf.name = "__delete__";
	cf.return_type = TypeVoid;
	t->function.add(cf);
}

void CreateImplicitAssign(SyntaxTree *ps, Type *t)
{
	// create function
	Function *f = ps->AddFunction(t->name + ".__assign__", TypeVoid);
	int fn = ps->Functions.num - 1;
	ps->AddVar("other", t, f);
	f->num_params = 1;
	f->literal_param_type[0] = t;
	f->_class = t;
	ps->AddVar("self", ps->GetPointerType(t), f);

	Command *deref_other = ps->AddCommand();
	deref_other->kind = KindVarLocal;
	deref_other->link_nr = 0;
	deref_other->type = t;
	Command *other = cp_command(ps, deref_other);
	ref_command(ps, other);

	Command *self = ps->AddCommand();
	self->kind = KindVarLocal;
	self->link_nr = 1;
	self->type = ps->GetPointerType(t);

	if (t->is_super_array){

		int nf = t->GetFunc("resize");
		if (nf < 0){
			ps->DoError(format("%s.__assign__(): no %s.resize() found", t->name.c_str(), t->name.c_str()));
			return;
		}

		// self.resize(other.num)
		Command *other_num = ps->AddCommand();
		other_num->kind = KindDerefAddressShift;
		other_num->link_nr = PointerSize;
		other_num->type = TypeInt;
		other_num->num_params = 1;
		other_num->param[0] = cp_command_deep(ps, other);

		Command *cmd_resize = add_command_classfunc(ps, t, t->function[nf], cp_command(ps, self));
		cmd_resize->num_params = 1;
		cmd_resize->param[0] = other_num;
		f->block->command.add(cmd_resize);

		// for int i, 0, other.num
		//    self[i].__assign__(other[i])

		ps->AddVar("i", TypeInt, f);

		Command *for_var = ps->AddCommand();
		for_var->kind = KindVarLocal;
		for_var->link_nr = 2;
		for_var->type = TypeInt;


		// for_var = 0
		int nc = ps->AddConstant(TypeInt);
		(*(int*)ps->Constants[nc].data) = 0;
		Command *cmd_0 = add_command_const(ps, nc);
		Command *cmd_assign0 = ps->AddCommand();
		CommandMakeOperator(cmd_assign0, for_var, cmd_0, OperatorIntAssign);
		f->block->command.add(cmd_assign0);

		// while(for_var < self.num)
		Command *cmd_cmp = ps->AddCommand();
		CommandMakeOperator(cmd_cmp, for_var, cp_command_deep(ps, other_num), OperatorIntSmaller);

		Command *cmd_while = add_command_compilerfunc(ps, CommandFor);
		cmd_while->param[0] = cmd_cmp;
		f->block->command.add(cmd_while);

		Command *cb = ps->AddCommand();
		Block *b = ps->AddBlock();
		cb->kind = KindBlock;
		cb->link_nr = b->index;

		// el := self[for_var]
		Command *deref_self = cp_command(ps, self);
		deref_command(ps, deref_self);
		Command *cmd_el = ps->AddCommand();
		cmd_el->kind = KindPointerAsArray;
		cmd_el->type = t->parent;
		cmd_el->param[0] = deref_self;
		cmd_el->param[1] = for_var;
		cmd_el->num_params = 2;

		// el2 := other[for_var]
		Command *cmd_el2 = ps->AddCommand();
		cmd_el2->kind = KindPointerAsArray;
		cmd_el2->type = t->parent;
		cmd_el2->param[0] = deref_other;
		cmd_el2->param[1] = for_var;
		cmd_el2->num_params = 2;


		Command *cmd_assign = ps->AddCommand();
		if (!ps->LinkOperator(OperatorAssign, cmd_el, cmd_el2, &cmd_assign)){
			ps->DoError(format("%s.__assign__(): no %s.__assign__() found", t->name.c_str(), t->parent->name.c_str()));
			return;
		}
		b->command.add(cmd_assign);

		// ...for_var += 1
		Command *cmd_inc = ps->AddCommand();
		CommandMakeOperator(cmd_inc, for_var, cmd_0 /*dummy*/, OperatorIntIncrease);
		b->command.add(cmd_inc);
		f->block->command.add(cb);
	}else{

		// call child assignment
		foreach(ClassElement &e, t->element){
			Command *p = ps->AddCommand();
			p->kind = KindDerefAddressShift;
			p->link_nr = e.offset;
			p->type = e.type;
			p->num_params = 1;
			p->param[0] = self;
			Command *o = ps->AddCommand();
			o->kind = KindDerefAddressShift;
			o->link_nr = e.offset;
			o->type = e.type;
			o->num_params = 1;
			o->param[0] = cp_command(ps, other); // needed for call-by-ref conversion!

			Command *cmd_assign = ps->AddCommand();
			if (!ps->LinkOperator(OperatorAssign, p, o, &cmd_assign)){
				ps->DoError(format("%s.__assign__(): no %s.__assign__ for element \"%s\"", t->name.c_str(), e.type->name.c_str(), e.name.c_str()));
				return;
			}
			f->block->command.add(cmd_assign);
		}
	}

	ClassFunction cf;
	cf.kind = KindFunction;
	cf.nr = fn;
	cf.name = "__assign__";
	cf.return_type = TypeVoid;
	cf.param_type.add(t);
	t->function.add(cf);
}


void CreateImplicitArrayClear(SyntaxTree *ps, Type *t)
{
	// create function
	Function *f = ps->AddFunction(t->name + ".clear", TypeVoid);
	int fn = ps->Functions.num - 1;
	f->_class = t;
	ps->AddVar("self", ps->GetPointerType(t), f);
	ps->AddVar("for_var", TypeInt, f);

	Command *self = ps->AddCommand();
	self->kind = KindVarLocal;
	self->link_nr = 0;
	self->type = ps->GetPointerType(t);

	Command *self_num = ps->AddCommand();
	self_num->kind = KindDerefAddressShift;
	self_num->link_nr = PointerSize;
	self_num->type = TypeInt;
	self_num->num_params = 1;
	self_num->param[0] = cp_command(ps, self);

	Command *for_var = ps->AddCommand();
	for_var->kind = KindVarLocal;
	for_var->link_nr = 1;
	for_var->type = TypeInt;

// delete...
	if (t->parent->GetFunc("__delete__") >= 0){
		// for_var = 0
		int nc = ps->AddConstant(TypeInt);
		(*(int*)ps->Constants[nc].data) = 0;
		Command *cmd_0 = add_command_const(ps, nc);
		Command *cmd_assign = ps->AddCommand();
		CommandMakeOperator(cmd_assign, for_var, cmd_0, OperatorIntAssign);
		f->block->command.add(cmd_assign);

		// while(for_var < self.num)
		Command *cmd_cmp = ps->AddCommand();
		CommandMakeOperator(cmd_cmp, for_var, self_num, OperatorIntSmaller);

		Command *cmd_while = add_command_compilerfunc(ps, CommandFor);
		cmd_while->param[0] = cmd_cmp;
		f->block->command.add(cmd_while);

		Command *cb = ps->AddCommand();
		Block *b = ps->AddBlock();
		cb->kind = KindBlock;
		cb->link_nr = b->index;

		// el := self[for_var]
		Command *deref_self = cp_command(ps, self);
		deref_command(ps, deref_self);
		Command *cmd_el = ps->AddCommand();
		cmd_el->kind = KindPointerAsArray;
		cmd_el->type = t->parent;
		cmd_el->param[0] = deref_self;
		cmd_el->param[1] = for_var;
		cmd_el->num_params = 2;
		ref_command(ps, cmd_el);

		// __delete__
		Command *cmd_delete = add_command_classfunc(ps, t, t->parent->function[t->parent->GetFunc("__delete__")], cmd_el);
		b->command.add(cmd_delete);

		// ...for_var += 1
		Command *cmd_inc = ps->AddCommand();
		CommandMakeOperator(cmd_inc, for_var, cmd_0 /*dummy*/, OperatorIntIncrease);
		b->command.add(cmd_inc);
		f->block->command.add(cb);
	}

	// clear
	Command *cmd_clear = add_command_classfunc(ps, t, t->function[t->GetFunc("__mem_clear__")], self);
	f->block->command.add(cmd_clear);


	ClassFunction cf;
	cf.kind = KindFunction;
	cf.nr = fn;
	cf.name = "clear";
	cf.return_type = TypeVoid;
	t->function.add(cf);
}


void CreateImplicitArrayResize(SyntaxTree *ps, Type *t)
{
	// create function
	Function *f = ps->AddFunction(t->name + ".resize", TypeVoid);
	int fn = ps->Functions.num - 1;
	ps->AddVar("num", TypeInt, f);
	f->num_params = 1;
	f->literal_param_type[0] = TypeInt;
	f->_class = t;
	ps->AddVar("self", ps->GetPointerType(t), f);
	ps->AddVar("for_var", TypeInt, f);
	ps->AddVar("num_old", TypeInt, f);

	Command *num = ps->AddCommand();
	num->kind = KindVarLocal;
	num->link_nr = 0;
	num->type = TypeInt;

	Command *self = ps->AddCommand();
	self->kind = KindVarLocal;
	self->link_nr = 1;
	self->type = ps->GetPointerType(t);

	Command *self_num = ps->AddCommand();
	self_num->kind = KindDerefAddressShift;
	self_num->link_nr = PointerSize;
	self_num->type = TypeInt;
	self_num->num_params = 1;
	self_num->param[0] = cp_command(ps, self);

	Command *for_var = ps->AddCommand();
	for_var->kind = KindVarLocal;
	for_var->link_nr = 2;
	for_var->type = TypeInt;

	Command *num_old = ps->AddCommand();
	num_old->kind = KindVarLocal;
	num_old->link_nr = 3;
	num_old->type = TypeInt;

	// num_old = self.num
	Command *cmd_copy_num = ps->AddCommand();
	CommandMakeOperator(cmd_copy_num, num_old, self_num, OperatorIntAssign);
	f->block->command.add(cmd_copy_num);

// delete...
	if (t->parent->GetFunc("__delete__") >= 0){
		// for_var = num
		Command *cmd_assign = ps->AddCommand();
		CommandMakeOperator(cmd_assign, for_var, num, OperatorIntAssign);
		f->block->command.add(cmd_assign);

		// while(for_var < self.num)
		Command *cmd_cmp = ps->AddCommand();
		CommandMakeOperator(cmd_cmp, for_var, self_num, OperatorIntSmaller);

		Command *cmd_while = add_command_compilerfunc(ps, CommandFor);
		cmd_while->param[0] = cmd_cmp;
		f->block->command.add(cmd_while);

		Command *cb = ps->AddCommand();
		Block *b = ps->AddBlock();
		cb->kind = KindBlock;
		cb->link_nr = b->index;

		// el := self[for_var]
		Command *deref_self = cp_command(ps, self);
		deref_command(ps, deref_self);
		Command *cmd_el = ps->AddCommand();
		cmd_el->kind = KindPointerAsArray;
		cmd_el->type = t->parent;
		cmd_el->param[0] = deref_self;
		cmd_el->param[1] = for_var;
		cmd_el->num_params = 2;
		ref_command(ps, cmd_el);

		// __delete__
		Command *cmd_delete = add_command_classfunc(ps, t, t->parent->function[t->parent->GetFunc("__delete__")], cmd_el);
		b->command.add(cmd_delete);

		// ...for_var += 1
		Command *cmd_inc = ps->AddCommand();
		CommandMakeOperator(cmd_inc, for_var, num /*dummy*/, OperatorIntIncrease);
		b->command.add(cmd_inc);
		f->block->command.add(cb);
	}

	// resize
	Command *c_resize = add_command_classfunc(ps, t, t->function[t->GetFunc("__mem_resize__")], self);
	c_resize->num_params = 1;
	c_resize->param[0] = num;
	f->block->command.add(c_resize);

	// new...
	if (t->parent->GetFunc("__init__") >= 0){
		// for_var = num_old
		Command *cmd_assign = ps->AddCommand();
		CommandMakeOperator(cmd_assign, for_var, num_old, OperatorIntAssign);
		f->block->command.add(cmd_assign);

		// while(for_var < self.num)
		Command *cmd_cmp = ps->AddCommand();
		CommandMakeOperator(cmd_cmp, for_var, self_num, OperatorIntSmaller);

		Command *cmd_while = add_command_compilerfunc(ps, CommandFor);
		cmd_while->param[0] = cmd_cmp;
		f->block->command.add(cmd_while);

		Command *cb = ps->AddCommand();
		Block *b = ps->AddBlock();
		cb->kind = KindBlock;
		cb->link_nr = b->index;

		// el := self[for_var]
		Command *deref_self = cp_command(ps, self);
		deref_command(ps, deref_self);
		Command *cmd_el = ps->AddCommand();
		cmd_el->kind = KindPointerAsArray;
		cmd_el->type = t->parent;
		cmd_el->param[0] = deref_self;
		cmd_el->param[1] = for_var;
		cmd_el->num_params = 2;
		ref_command(ps, cmd_el);

		// __init__
		Command *cmd_init = add_command_classfunc(ps, t, t->parent->function[t->parent->GetFunc("__init__")], cmd_el);
		b->command.add(cmd_init);

		// ...for_var += 1
		Command *cmd_inc = ps->AddCommand();
		CommandMakeOperator(cmd_inc, for_var, num /*dummy*/, OperatorIntIncrease);
		b->command.add(cmd_inc);
		f->block->command.add(cb);
	}


	ClassFunction cf;
	cf.kind = KindFunction;
	cf.nr = fn;
	cf.name = "resize";
	cf.return_type = TypeVoid;
	cf.param_type.add(TypeInt);
	t->function.add(cf);
}

void CreateImplicitArrayAdd(SyntaxTree *ps, Type *t)
{
	// create function
	Function *f = ps->AddFunction(t->name + ".add", TypeVoid);
	int fn = ps->Functions.num - 1;
	ps->AddVar("x", t->parent, f);
	f->num_params = 1;
	f->literal_param_type[0] = t->parent;
	f->_class = t;
	ps->AddVar("self", ps->GetPointerType(t), f);

	Command *item = ps->AddCommand();
	item->kind = KindVarLocal;
	item->link_nr = 0;
	item->type = t->parent;

	Command *self = ps->AddCommand();
	self->kind = KindVarLocal;
	self->link_nr = 1;
	self->type = ps->GetPointerType(t);

	Command *self_num = ps->AddCommand();
	self_num->kind = KindDerefAddressShift;
	self_num->link_nr = PointerSize;
	self_num->type = TypeInt;
	self_num->num_params = 1;
	self_num->param[0] = cp_command(ps, self);


	// resize(self.num + 1)
	int nc = ps->AddConstant(TypeInt);
	(*(int*)ps->Constants[nc].data) = 1;
	Command *cmd_1 = add_command_const(ps, nc);
	Command *cmd_add = ps->AddCommand();
	CommandMakeOperator(cmd_add, self_num, cmd_1, OperatorIntAdd);
	Command *cmd_resize = add_command_classfunc(ps, t, t->function[t->GetFunc("resize")], self);
	cmd_resize->num_params = 1;
	cmd_resize->param[0] = cmd_add;
	f->block->command.add(cmd_resize);



	// el := self[self.num - 1]
	Command *cmd_sub = ps->AddCommand();
	CommandMakeOperator(cmd_sub, cp_command(ps, self_num), cmd_1, OperatorIntSubtract);
	Command *deref_self = cp_command(ps, self);
	deref_command(ps, deref_self);
	Command *cmd_el = ps->AddCommand();
	cmd_el->kind = KindPointerAsArray;
	cmd_el->type = t->parent;
	cmd_el->param[0] = deref_self;
	cmd_el->param[1] = cmd_sub;
	cmd_el->num_params = 2;

	Command *cmd_assign = ps->AddCommand();
	if (!ps->LinkOperator(OperatorAssign, cmd_el, item, &cmd_assign)){
		ps->DoError(format("%s.add(): no %s.__assign__ for elements", t->name.c_str(), t->parent->name.c_str()));
		return;
	}
	f->block->command.add(cmd_assign);

	ClassFunction cf;
	cf.kind = KindFunction;
	cf.nr = fn;
	cf.name = "add";
	cf.return_type = TypeVoid;
	cf.param_type.add(t->parent);
	t->function.add(cf);
}



void SyntaxTree::CreateImplicitFunctions(Type *t, bool relocate_last_function)
{
	int num_funcs = Functions.num;

	if (t->owner != this)
		return;
	if (t->is_pointer)
		return;

	// needs complex functions?
	/*bool needs_init = false;
	foreach(t->Element, e)
		foreach(e->Type->Function, f)
			if (strcmp(f->Name, "__init__") == 0)
				needs_init = true;
	if (t->IsSuperArray)
		needs_init = true;*/

	if (t->is_super_array){
		if (t->GetFunc("clear") < 0)
			CreateImplicitArrayClear(this, t);
		if (t->GetFunc("resize") < 0)
			CreateImplicitArrayResize(this, t);
		if (t->GetFunc("add") < 0)
			CreateImplicitArrayAdd(this, t);
	}
	if (!type_is_simple_class(t)){//needs_init){
		if (t->GetFunc("__init__") < 0)
			CreateImplicitConstructor(this, t);
		if (t->GetFunc("__delete__") < 0)
			CreateImplicitDestructor(this, t);
		if (t->GetFunc("__assign__") < 0)
			CreateImplicitAssign(this, t);
	}

	if (relocate_last_function && (num_funcs != Functions.num)){
		//msg_error("relocate implicit function");
		// resort Function[]
		Function *f = Functions[num_funcs - 1];
		Functions.erase(num_funcs - 1);
		Functions.add(f);

		// relink commands
		foreach(Command *c, Commands)
			if (c->kind == KindFunction){
				if (c->script != script)
					continue;
				if (c->link_nr == num_funcs - 1)
					c->link_nr = Functions.num - 1;
				else if (c->link_nr > num_funcs - 1)
					c->link_nr --;
			}

		// relink class functions
		foreach(Type *t, Types)
			foreach(ClassFunction &f, t->function)
				if (f.kind == KindFunction){
					if (f.nr == num_funcs - 1)
						f.nr = Functions.num - 1;
					else if (f.nr > num_funcs - 1)
						f.nr --;
				}
	}
}

void SyntaxTree::CreateAllImplicitFunctions(bool relocate_last_function)
{
	foreach(Type *t, Types)
		CreateImplicitFunctions(t, relocate_last_function);
}

// no included scripts may be deleted before us!!!
SyntaxTree::~SyntaxTree()
{
	msg_db_f("~SyntaxTree", 4);
	
	Exp.clear();

	// delete all types created by this script
	for (int i=Types.num-NumOwnTypes;i<Types.num;i++)
		if (Types[i]->owner == this) // redundant...
			delete(Types[i]);

	
	msg_db_m("asm", 8);
	if (AsmMetaInfo)
		delete(AsmMetaInfo);
	
	msg_db_m("const", 8);
	foreach(Constant &c, Constants)
		if (c.owner == this)
			delete[](c.data);

	
	msg_db_m("cmd", 8);
	for (int i=0;i<Commands.num;i++)
		delete(Commands[i]);

	msg_db_m("rest", 8);

	for (int i=0;i<Blocks.num;i++)
		delete(Blocks[i]);
	for (int i=0;i<Functions.num;i++)
		delete(Functions[i]);
}

void SyntaxTree::ShowCommand(Command *c)
{
	msg_write("[" + Kind2Str(c->kind) + "] " + c->type->name + " " + LinkNr2Str(c->script->syntax,c->kind,c->link_nr));
	msg_right();
	if (c->instance)
		ShowCommand(c->instance);
	for (int p=0;p<c->num_params;p++)
		ShowCommand(c->param[p]);
	msg_left();
}

void SyntaxTree::ShowBlock(Block *b)
{
	msg_write("block");
	msg_right();
	foreach(Command *c, b->command){
		if (c->kind == KindBlock)
			ShowBlock(Blocks[c->link_nr]);
		else
			ShowCommand(c);
	}
	msg_left();
	msg_write("/block");
}

void SyntaxTree::ShowFunction(Function *f)
{
	msg_write("[function] " + f->return_type->name + " " + f->name);
	cur_func = f;
	ShowBlock(f->block);
}

void SyntaxTree::Show()
{
	msg_write("--------- Syntax of " + Filename + " ---------");
	msg_right();
	foreach(Function *f, Functions)
		ShowFunction(f);
	msg_left();
	msg_write("\n\n");
}

};