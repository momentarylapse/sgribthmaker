#include "../script.h"
#include "serializer.h"
#include "../../file/file.h"

namespace Script{


extern void script_db_out(const string &str);
extern void script_db_out(int i);

#define so		script_db_out

//#define debug_evil_corrections	1

//#ifdef ScriptDebug


extern int GlobalWaitingMode;
extern float GlobalTimeToWait;


static SerialCommandParam p_eax, p_eax_int, p_deref_eax;
static SerialCommandParam p_rax;
static SerialCommandParam p_ax, p_al, p_ah, p_al_bool, p_al_char;
static SerialCommandParam p_st0, p_st1;
static const SerialCommandParam p_none = {-1, NULL, NULL, 0};


void Serializer::add_reg_channel(int reg, int first, int last)
{
	RegChannel c = {Asm::RegRoot[reg], first, last};
	reg_channel.add(c);
}

void Serializer::add_temp(Type *t, SerialCommandParam &param, bool add_constructor)
{
	if (t != TypeVoid){
		TempVar v;
		v.first = -1;
		v.type = t;
		v.force_stack = (t->size > config.PointerSize) || (t->is_super_array) || (t->is_array) || (t->element.num > 0);
		v.entangled = 0;
		temp_var.add(v);
		param.kind = KindVarTemp;
		param.p = (char*)(long)(temp_var.num - 1);
		param.type = t;
		param.shift = 0;

		if (add_constructor)
			add_cmd_constructor(param, KindVarTemp);
		else
			InsertedConstructorTemp.add(param);
	}else{
		param = p_none;
	}
}

inline Type *get_subtype(Type *t)
{
	if (t->parent)
		return t->parent;
	msg_error("subtype wanted of... " + t->name);
	//msg_write(cur_func->Name);
	return TypeUnknown;
}

inline void deref_temp(SerialCommandParam &param, SerialCommandParam &deref)
{
	so("deref temp");
	so((long)param.p);
	//deref = param;
	deref.kind = KindDerefVarTemp;
	deref.p = param.p;
	deref.type = get_subtype(param.type);
	deref.shift = 0;
	so((long)deref.p);
}

inline SerialCommandParam param_shift(SerialCommandParam &param, int shift, Type *t)
{
	SerialCommandParam p = param;
	p.shift += shift;
	p.type = t;
	return p;
}

inline SerialCommandParam param_global(Type *type, void *v)
{
	SerialCommandParam p;
	p.type = type;
	p.kind = KindVarGlobal;
	p.p = (char*)v;
	p.shift = 0;
	return p;
}

inline SerialCommandParam param_local(Type *type, int offset)
{
	SerialCommandParam p;
	p.type = type;
	p.kind = KindVarLocal;
	p.p = (char*)(long)offset;
	p.shift = 0;
	return p;
}

inline SerialCommandParam param_const(Type *type, void *c)
{
	SerialCommandParam p;
	p.type = type;
	p.kind = KindConstant;
	p.p = (char*)c;
	p.shift = 0;
	return p;
}

inline SerialCommandParam param_marker(int m)
{
	SerialCommandParam p;
	p.type = TypeInt;
	p.kind = KindMarker;
	p.p = (char*)(long)m;
	p.shift = 0;
	return p;
}

inline SerialCommandParam param_reg(Type *type, int reg)
{
	SerialCommandParam p;
	p.kind = KindRegister;
	p.p = (char*)(long)reg;
	p.type = type;
	p.shift = 0;
	return p;
}

string param_out(SerialCommandParam &p)
{
	string str;
	if (p.kind >= 0){
		string n = p2s(p.p);
		if ((p.kind == KindRegister) || (p.kind == KindDerefRegister))
			n = Asm::GetRegName((long)p.p);
		str = "  " + Kind2Str(p.kind) + " " + n + " (" + p.type->name + ")";
		if (p.shift > 0)
			str += format(" + shift %d", p.shift);
	}
	return str;
}

string cmd2str(SerialCommand &c)
{
	//msg_db_f("cmd_out", 4);
	if (c.inst == inst_marker)
		return format("-- Marker %d --", (long)c.p1.p);
	if (c.inst == inst_asm)
		return "-- Asm --";
	string t = Asm::GetInstructionName(c.inst);
	t += param_out(c.p1);
	t += param_out(c.p2);
	return t;
}

void Serializer::cmd_list_out()
{
	msg_db_f("cmd_list_out", 4);
	msg_write("--------------------------------");
	for (int i=0;i<cmd.num;i++)
		msg_write(format("%3d: ", i) + cmd2str(cmd[i]));
	so("-----------");
	for (int i=0;i<reg_channel.num;i++)
		so(format("  %d   %d -> %d", reg_channel[i].reg_root, reg_channel[i].first, reg_channel[i].last));
	so("-----------");
	if (TempVarRangesDefined)
		for (int i=0;i<temp_var.num;i++)
			so(format("  %d   %d -> %d", i, temp_var[i].first, temp_var[i].last));
	so("--------------------------------");
}

inline int get_reg(int root, int size)
{
#if 1
	if ((size != 1) && (size != 4) && (size != 8)){
		msg_write(msg_get_trace());
		throw Asm::Exception("get_reg: bad reg size: " + i2s(size), "...", 0, 0);
	}
#endif
	return Asm::RegResize[root][size];
}

void Serializer::add_cmd(int inst, SerialCommandParam p1, SerialCommandParam p2)
{
	SerialCommand c;
	c.inst = inst;
	c.p1 = p1;
	c.p2 = p2;
	cmd.add(c);

	// call violates all used registers...
	if (inst == Asm::inst_call)
		for (int i=0;i<MapRegRoot.num;i++){
			add_reg_channel(get_reg(i, 4), cmd.num - 1, cmd.num - 1);
		}
}

void Serializer::add_cmd(int inst, SerialCommandParam p)
{
	add_cmd(inst, p, p_none);
}

void Serializer::add_cmd(int inst)
{
	add_cmd(inst, p_none, p_none);
}

void Serializer::move_last_cmd(int index)
{
	SerialCommand c = cmd.back();
	for (int i=cmd.num-1;i>index;i--)
		cmd[i] = cmd[i - 1];
	cmd[index] = c;

	// adjust temp vars
	if (TempVarRangesDefined){
		foreach(TempVar &v, temp_var){
			if (v.first >= index)
				v.first ++;
			if (v.last >= index)
				v.last ++;
		}
	}

	// adjust reg channels
	foreach(RegChannel &r, reg_channel){
		if (r.first >= index)
			r.first ++;
		if (r.last >= index)
			r.last ++;
	}
}

void Serializer::remove_cmd(int index)
{
	cmd.erase(index);

	// adjust temp vars
	foreach(TempVar &v, temp_var){
		if (v.first >= index)
			v.first --;
		if (v.last >= index)
			v.last --;
	}

	// adjust reg channels
	foreach(RegChannel &r, reg_channel){
		if (r.first >= index)
			r.first --;
		if (r.last >= index)
			r.last --;
	}
}

void Serializer::remove_temp_var(int v)
{
	foreach(SerialCommand &c, cmd){
		if ((c.p1.kind == KindVarTemp) || (c.p1.kind == KindDerefVarTemp))
			if ((long)c.p1.p > v)
				c.p1.p = (char*)((long)c.p1.p - 1);
		if ((c.p2.kind == KindVarTemp) || (c.p2.kind == KindDerefVarTemp))
			if ((long)c.p2.p > v)
				c.p2.p = (char*)((long)c.p2.p - 1);
	}
	temp_var.erase(v);
}

void Serializer::move_param(SerialCommandParam &p, int from, int to)
{
	if ((p.kind == KindVarTemp) || (p.kind == KindDerefVarTemp)){
		so("move_param temp");
		long v = (long)p.p;
		if (temp_var[v].last < max(from, to))
			temp_var[v].last = max(from, to);
		if (temp_var[v].first > min(from, to))
			temp_var[v].first = min(from, to);
	}else if ((p.kind == KindRegister) || (p.kind == KindDerefRegister)){
		so("move_param reg");
		long r = Asm::RegRoot[(long)p.p];
		bool found = false;
		foreach(RegChannel &rc, reg_channel)
			if ((r == rc.reg_root) && (from >= rc.first) && (from >= rc.first)){
				if (rc.last < max(from, to))
					rc.last = max(from, to);
				if (rc.first > min(from, to))
					rc.first = min(from, to);
				found = true;
			}
		if (!found){
			msg_error(format("move_param: no RegChannel...  reg_root=%d  from=%d", r, from));
			msg_write(script->syntax->Filename + " : " + cur_func->name);
		}
	}
}

int Serializer::add_marker(int m)
{
	SerialCommandParam p = p_none;
	if (m < 0)
		m = NumMarkers ++;
	p.kind = KindMarker;
	p.p = (char*)(long)m;
	add_cmd(inst_marker, p);
	return m;
}

int Serializer::add_marker_after_command(int level, int index)
{
	int n = NumMarkers ++;
	AddLaterData m = {StuffKindMarker, n, level, index};
	add_later.add(m);
	return n;
}

void Serializer::add_jump_after_command(int level, int index, int marker)
{
	AddLaterData j = {StuffKindJump, marker, level, index};
	add_later.add(j);
}

inline int reg_resize(int reg, int size)
{
	if (size == 2){
		msg_error("size = 2");
		msg_write(msg_get_trace());
		throw Asm::Exception("size=2", "kjlkjl", 0, 0);
		//Asm::DoError("size=2");
	}
	return get_reg(Asm::RegRoot[reg], size);
}


static Array<SerialCommandParam> CompilerFunctionParam;
static SerialCommandParam CompilerFunctionReturn = {-1, NULL, NULL};
static SerialCommandParam CompilerFunctionInstance = {-1, NULL, NULL};

void AddFuncParam(SerialCommandParam &p)
{
	CompilerFunctionParam.add(p);
}

void AddFuncReturn(SerialCommandParam &r)
{
	CompilerFunctionReturn = r;
}

void AddFuncInstance(SerialCommandParam &inst)
{
	CompilerFunctionInstance = inst;
}

void AddReference(Serializer *d, SerialCommandParam &param, Type *type, SerialCommandParam &ret);


void Serializer::add_function_call_x86(Script *script, int func_no)
{
	msg_db_f("AddFunctionCallX86", 4);
	call_used = true;
	Type *type = CompilerFunctionReturn.type;
	if (!type)
		type = TypeVoid;

	// return data too big... push address
	SerialCommandParam ret_temp, ret_ref;
	if (type->UsesReturnByMemory()){
		//add_temp(type, ret_temp);
		AddReference(/*ret_temp*/ CompilerFunctionReturn, TypePointer, ret_ref);
		//add_ref();
		//add_cmd(Asm::inst_lea, KindRegister, (char*)RegEaxCompilerFunctionReturn.kind, CompilerFunctionReturn.param);
	}

	// grow stack (down) for local variables of the calling function
//	add_cmd(- cur_func->_VarSize - LocalOffset - 8);
	long push_size = 0;
	
	// push parameters onto stack
	for (int p=CompilerFunctionParam.num-1;p>=0;p--){
		if (CompilerFunctionParam[p].type){
			int s = mem_align(CompilerFunctionParam[p].type->size, 4);
			for (int j=0;j<s/4;j++)
				add_cmd(Asm::inst_push, param_shift(CompilerFunctionParam[p], s - 4 - j * 4, TypeInt));
			push_size += s;
		}
	}

#ifdef NIX_IDE_VCS
	// more than 4 byte have to be returned -> give return address as very last parameter!
	if (type->UsesReturnByMemory())
		add_cmd(Asm::inst_push, ret_ref; // nachtraegliche eSP-Korrektur macht die Funktion
#endif
	
	// _cdecl: push class instance as first parameter
	if (CompilerFunctionInstance.type){
		add_cmd(Asm::inst_push, CompilerFunctionInstance);
		push_size += config.PointerSize;
	}
	
#ifndef NIX_IDE_VCS
	// more than 4 byte have to be returned -> give return address as very first parameter!
	if (type->UsesReturnByMemory())
		add_cmd(Asm::inst_push, ret_ref); // nachtraegliche eSP-Korrektur macht die Funktion
#endif

	void *func = (void*)script->func[func_no];
	if (!func)
		DoErrorLink("could not link function " + script->syntax->Functions[func_no]->name);
	add_cmd(Asm::inst_call, param_const(TypePointer, func)); // the actual call
	// function pointer will be shifted later...

	if (push_size > 127)
		add_cmd(Asm::inst_add, param_reg(TypePointer, Asm::RegEsp), param_const(TypeInt, (void*)push_size));
	else if (push_size > 0)
		add_cmd(Asm::inst_add, param_reg(TypePointer, Asm::RegEsp), param_const(TypeChar, (void*)push_size));

	// return > 4b already got copied to [ret] by the function!
	if ((type != TypeVoid) && (!type->UsesReturnByMemory())){
		if (type == TypeFloat)
			add_cmd(Asm::inst_fstp, CompilerFunctionReturn);
		else if (type->size == 1){
			add_cmd(Asm::inst_mov, CompilerFunctionReturn, p_al);
			add_reg_channel(Asm::RegEax, cmd.num - 2, cmd.num - 1);
		}else{
			add_cmd(Asm::inst_mov, CompilerFunctionReturn, p_eax);
			add_reg_channel(Asm::RegEax, cmd.num - 2, cmd.num - 1);
		}
	}
}

void Serializer::add_function_call_amd64(Script *script, int func_no)
{
	msg_db_f("AddFunctionCallAMD64", 4);
	call_used = true;
	Type *type = CompilerFunctionReturn.type;
	if (!type)
		type = TypeVoid;

	// return data too big... push address
	SerialCommandParam ret_temp, ret_ref;
	if (type->UsesReturnByMemory()){
		//add_temp(type, ret_temp);
		AddReference(/*ret_temp*/ CompilerFunctionReturn, TypePointer, ret_ref);
		//add_ref();
		//add_cmd(Asm::inst_lea, KindRegister, (char*)RegEaxCompilerFunctionReturn.kind, CompilerFunctionReturn.param);
	}

	// grow stack (down) for local variables of the calling function
//	add_cmd(- cur_func->_VarSize - LocalOffset - 8);
	long push_size = 0;

		
	// instance as first parameter
	if (CompilerFunctionInstance.type)
		CompilerFunctionParam.insert(CompilerFunctionInstance, 0);

	// return as _very_ first parameter
	if (type->UsesReturnByMemory()){
		//add_temp(type, ret_temp);
		AddReference(/*ret_temp*/ CompilerFunctionReturn, TypePointer, ret_ref);
		CompilerFunctionParam.insert(ret_ref, 0);
	}

	// map params...
	Array<SerialCommandParam> reg_param;
	Array<SerialCommandParam> stack_param;
	Array<SerialCommandParam> xmm_param;
	foreach(SerialCommandParam &p, CompilerFunctionParam){
		if ((p.type == TypeInt) || (p.type == TypeChar) || (p.type == TypeBool) || (p.type->is_pointer)){
			if (reg_param.num < 6){
				reg_param.add(p);
			}else{
				stack_param.add(p);
			}
		}else if (p.type == TypeFloat){
			if (xmm_param.num < 8){
				xmm_param.add(p);
			}else{
				stack_param.add(p);
			}
		}else
			DoError("parameter type currently not supported: " + p.type->name);
	}
	
	// push parameters onto stack
	push_size = 8 * stack_param.num;
	if (push_size > 127)
		add_cmd(Asm::inst_add, param_reg(TypePointer, Asm::RegRsp), param_const(TypeInt, (void*)push_size));
	else if (push_size > 0)
		add_cmd(Asm::inst_add, param_reg(TypePointer, Asm::RegRsp), param_const(TypeChar, (void*)push_size));
	foreachb(SerialCommandParam &p, stack_param)
		add_cmd(Asm::inst_push, p);
	MaxPushSize = max(MaxPushSize, push_size);
	
	// rdi, rsi,rdx, rcx, r8, r9 
	int param_regs_root[6] = {7, 6, 2, 1, 8, 9};
	foreachib(SerialCommandParam &p, reg_param, i){
		int root = param_regs_root[i];
		int reg = get_reg(root, p.type->size);
		if (reg >= 0){
			add_cmd(Asm::inst_mov, param_reg(p.type, reg), p);
		}else{
			// some registers are not 8bit'able
			add_cmd(Asm::inst_mov, p_al, p);
			add_cmd(Asm::inst_mov, param_reg(TypeReg32, get_reg(root, 4)), p_eax);
		}
	}
	// xmm0-7 
	foreachib(SerialCommandParam &p, xmm_param, i){
		int reg = Asm::RegXmm0 + i;
		add_cmd(Asm::inst_movss, param_reg(TypeReg128, reg), p);
	}

	void *func = (void*)script->func[func_no];
	if (!func)
		DoErrorLink("could not link function " + script->syntax->Functions[func_no]->name);
	//add_cmd(Asm::inst_call, param_const(TypePointer, func)); // the actual call
	add_cmd(Asm::inst_call, param_const(TypeReg32, func)); // the actual call
	// function pointer will be shifted later...

	// return > 4b already got copied to [ret] by the function!
	if ((type != TypeVoid) && (!type->UsesReturnByMemory())){
		if (type == TypeFloat)
			add_cmd(Asm::inst_movss, CompilerFunctionReturn, param_reg(TypeReg128, Asm::RegXmm0));
		else if (type->size == 1){
			add_cmd(Asm::inst_mov, CompilerFunctionReturn, p_al);
			add_reg_channel(Asm::RegEax, cmd.num - 2, cmd.num - 1);
		}else if (type->size == 4){
			add_cmd(Asm::inst_mov, CompilerFunctionReturn, p_eax);
			add_reg_channel(Asm::RegEax, cmd.num - 2, cmd.num - 1);
		}else{
			add_cmd(Asm::inst_mov, CompilerFunctionReturn, p_rax);
			add_reg_channel(Asm::RegEax, cmd.num - 2, cmd.num - 1);
		}
	}
}

void Serializer::AddFunctionCall(Script *script, int func_no)
{
	if (config.instruction_set== Asm::InstructionSetAMD64)
		add_function_call_amd64(script, func_no);
	else if (config.instruction_set == Asm::InstructionSetX86)
		add_function_call_x86(script, func_no);

	// clean up for next call
	CompilerFunctionParam.clear();
	CompilerFunctionReturn.type = NULL;
	CompilerFunctionInstance.type = NULL;
}


// creates res...
void Serializer::AddReference(SerialCommandParam &param, Type *type, SerialCommandParam &ret)
{
	msg_db_f("AddReference", 3);
	so(Kind2Str(param.kind));
	ret.type = type;
	ret.shift = 0;
	if (param.kind == KindRefToConst){
		so("  Reference-RefConst");
		ret.kind = KindConstant;
		ret.p = param.p;
	}else if ((param.kind == KindConstant) || (param.kind == KindVarGlobal)){
		so("  Reference-Const");
		ret.kind = KindConstant;
		ret.p = param.p;
	}else if (param.kind == KindDerefVarTemp){
		ret = param;
		param.kind = KindVarTemp;
	}else{
		add_temp(type, ret);
		if (config.instruction_set == Asm::InstructionSetAMD64){
			add_cmd(Asm::inst_lea, p_rax, param);
			add_cmd(Asm::inst_mov, ret, p_rax);
		}else{
			add_cmd(Asm::inst_lea, p_eax, param);
			add_cmd(Asm::inst_mov, ret, p_eax);
		}
		add_reg_channel(Asm::RegEax, cmd.num - 2, cmd.num - 1);
	}
}

void Serializer::AddDereference(SerialCommandParam &param, SerialCommandParam &ret, Type *force_type)
{
	msg_db_f("AddDereference", 4);
	/*add_temp(TypePointer, ret);
	SerialCommandParam temp;
	add_temp(TypePointer, temp);
	add_cmd(Asm::inst_mov, temp, param);
	temp.kind = KindDerefVarTemp;
	add_cmd(Asm::inst_mov, ret, temp);*/
	if (param.kind == KindVarTemp){
		deref_temp(param, ret);
	}else if (param.kind == KindRegister){
		ret = param;
		ret.kind = KindDerefRegister;
		ret.type = force_type ? force_type : get_subtype(param.type);
		ret.shift = 0;
	}else{
		//msg_error(string("unhandled deref ", Kind2Str(param.kind)));
		SerialCommandParam temp;
		add_temp(param.type, temp);
		add_cmd(Asm::inst_mov, temp, param);
		deref_temp(temp, ret);
	}
}

// create data for a (function) parameter
//   and compile its command if the parameter is executable itself
void Serializer::SerializeParameter(Command *link, int level, int index, SerialCommandParam &p)
{
	msg_db_f("SerializeParameter", 4);
	p.kind = link->kind;
	p.type = link->type;
	p.p = NULL;
	p.shift = 0;
	//Type *rt=link->;
	//so(Kind2Str(link->Kind));
	if (link->kind == KindVarFunction){
		so(" -var-func");
		if (syntax_tree->FlagCompileOS)
			p.p = (char*)((long)script->func[link->link_nr] - (long)&script->Opcode[0] + (syntax_tree->AsmMetaInfo)->CodeOrigin);
		else
			p.p = (char*)script->func[link->link_nr];
		p.kind = KindVarGlobal;
	}else if (link->kind == KindMemory){
		so(" -mem");
		p.p = (char*)(long)link->link_nr;
		p.kind = KindVarGlobal;
	}else if (link->kind == KindAddress){
		so(" -addr");
		p.p = (char*)&link->link_nr;
		p.kind = KindRefToConst;
	}else if (link->kind == KindVarGlobal){
		so(" -global");
		p.p = link->script->g_var[link->link_nr];
		if (!p.p)
			script->DoErrorLink("variable is not linkable: " + link->script->syntax->RootOfAllEvil.var[link->link_nr].name);
	}else if (link->kind == KindVarLocal){
		so(" -local");
		p.p = (char*)(long)cur_func->var[link->link_nr]._offset;
	}else if (link->kind == KindLocalMemory){
		so(" -local mem");
		p.p = (char*)(long)link->link_nr;
		p.kind = KindVarLocal;
	}else if (link->kind == KindLocalAddress){
		so(" -local addr");
		SerialCommandParam param;
		param.p = (char*)(long)link->link_nr;
		param.kind = KindVarLocal;
		param.type = TypePointer;
		param.shift = 0;

		AddReference(param, link->type, p);
	}else if (link->kind == KindConstant){
		so(" -const");
		if ((config.UseConstAsGlobalVar) || (syntax_tree->FlagCompileOS))
			p.kind = KindVarGlobal;
		else
			p.kind = KindRefToConst;
		p.p = script->cnst[link->link_nr];
	}else if ((link->kind==KindOperator) || (link->kind==KindFunction) || (link->kind==KindCompilerFunction)){
		p = SerializeCommand(link, level, index);
	}else if (link->kind == KindReference){
		//so(Kind2Str(link->Meta->Kind));
		so(" -ref");
		SerialCommandParam param;
		SerializeParameter(link->param[0], level, index, param);
		//printf("%d  -  %s\n",pk,Kind2Str(pk));
		AddReference(param, link->type, p);
	}else if (link->kind == KindDereference){
		so(" -deref...");
		SerialCommandParam param;
		SerializeParameter(link->param[0], level, index, param);
		/*if ((param.kind == KindVarLocal) || (param.kind == KindVarGlobal)){
			p.type = param.type->sub_type;
			if (param.kind == KindVarLocal)		p.kind = KindRefToLocal;
			if (param.kind == KindVarGlobal)	p.kind = KindRefToGlobal;
			p.p = param.p;
		}*/
		AddDereference(param, p);
	}else{
		DoError("unexpected type of parameter: " + Kind2Str(link->kind));
	}
}


void Serializer::SerializeOperator(Command *com, SerialCommandParam *param, SerialCommandParam &ret)
{
	msg_db_f("SerializeOperator", 4);
	switch(com->link_nr){
		case OperatorIntAssign:
		case OperatorFloatAssign:
		case OperatorPointerAssign:
			add_cmd(Asm::inst_mov, param[0], param[1]);
			break;
		case OperatorCharAssign:
		case OperatorBoolAssign:
			add_cmd(Asm::inst_mov, param[0], param[1]);
			break;
		case OperatorClassAssign:
			for (int i=0;i<signed(com->param[0]->type->size)/4;i++)
				add_cmd(Asm::inst_mov, param_shift(param[0], i * 4, TypeInt), param_shift(param[1], i * 4, TypeInt));
			for (int i=4*signed(com->param[0]->type->size/4);i<signed(com->param[0]->type->size);i++)
				add_cmd(Asm::inst_mov, param_shift(param[0], i, TypeChar), param_shift(param[1], i, TypeChar));
			break;
// int
		case OperatorIntAddS:
			add_cmd(Asm::inst_add, param[0], param[1]);
			break;
		case OperatorIntSubtractS:
			add_cmd(Asm::inst_sub, param[0], param[1]);
			break;
		case OperatorIntMultiplyS:
			add_cmd(Asm::inst_imul, param[0], param[1]);
			break;
		case OperatorIntDivideS:
			add_cmd(Asm::inst_mov, p_eax_int, param[0]);
			add_cmd(Asm::inst_mov, param_reg(TypeInt, Asm::RegEdx), p_eax_int);
			add_cmd(Asm::inst_sar, param_reg(TypeInt, Asm::RegEdx), param_const(TypeChar, (void*)0x1f));
			add_cmd(Asm::inst_idiv, p_eax_int, param[1]);
			add_cmd(Asm::inst_mov, param[0], p_eax_int);
			add_reg_channel(Asm::RegEax, cmd.num - 5, cmd.num - 1);
			add_reg_channel(Asm::RegEdx, cmd.num - 2, cmd.num - 2);
			break;
		case OperatorIntAdd:
			add_cmd(Asm::inst_mov, ret, param[0]);
			add_cmd(Asm::inst_add, ret, param[1]);
			break;
		case OperatorIntSubtract:
			add_cmd(Asm::inst_mov, ret, param[0]);
			add_cmd(Asm::inst_sub, ret, param[1]);
			break;
		case OperatorIntMultiply:
			add_cmd(Asm::inst_mov, p_eax_int, param[0]);
			add_cmd(Asm::inst_imul, p_eax_int, param[1]);
			add_cmd(Asm::inst_mov, ret, p_eax_int);
			add_reg_channel(Asm::RegEax, cmd.num - 3, cmd.num - 1);
			break;
		case OperatorIntDivide:
			add_cmd(Asm::inst_mov, p_eax_int, param[0]);
			add_cmd(Asm::inst_mov, param_reg(TypeInt, Asm::RegEdx), p_eax_int);
			add_cmd(Asm::inst_sar, param_reg(TypeInt, Asm::RegEdx), param_const(TypeChar, (void*)0x1f));
			add_cmd(Asm::inst_idiv, p_eax_int, param[1]);
			add_cmd(Asm::inst_mov, ret, p_eax_int);
			add_reg_channel(Asm::RegEax, cmd.num - 5, cmd.num - 1);
			add_reg_channel(Asm::RegEdx, cmd.num - 2, cmd.num - 2);
			break;
		case OperatorIntModulo:
			add_cmd(Asm::inst_mov, p_eax_int, param[0]);
			add_cmd(Asm::inst_mov, param_reg(TypeInt, Asm::RegEdx), p_eax_int);
			add_cmd(Asm::inst_sar, param_reg(TypeInt, Asm::RegEdx), param_const(TypeChar, (void*)0x1f));
			add_cmd(Asm::inst_idiv, p_eax_int, param[1]);
			add_cmd(Asm::inst_mov, ret, param_reg(TypeInt, Asm::RegEdx));
			add_reg_channel(Asm::RegEax, cmd.num - 5, cmd.num - 2);
			add_reg_channel(Asm::RegEdx, cmd.num - 2, cmd.num - 1);
			break;
		case OperatorIntEqual:
		case OperatorIntNotEqual:
		case OperatorIntGreater:
		case OperatorIntGreaterEqual:
		case OperatorIntSmaller:
		case OperatorIntSmallerEqual:
		case OperatorPointerEqual:
		case OperatorPointerNotEqual:
			add_cmd(Asm::inst_cmp, param[0], param[1]);
			if (com->link_nr==OperatorIntEqual)			add_cmd(Asm::inst_setz, ret);
			if (com->link_nr==OperatorIntNotEqual)		add_cmd(Asm::inst_setnz, ret);
			if (com->link_nr==OperatorIntGreater)		add_cmd(Asm::inst_setnle, ret);
			if (com->link_nr==OperatorIntGreaterEqual)	add_cmd(Asm::inst_setnl, ret);
			if (com->link_nr==OperatorIntSmaller)		add_cmd(Asm::inst_setl, ret);
			if (com->link_nr==OperatorIntSmallerEqual)	add_cmd(Asm::inst_setle, ret);
			if (com->link_nr==OperatorPointerEqual)		add_cmd(Asm::inst_setz, ret);
			if (com->link_nr==OperatorPointerNotEqual)	add_cmd(Asm::inst_setnz, ret);
			break;
		case OperatorIntBitAnd:
			add_cmd(Asm::inst_mov, ret, param[0]);
			add_cmd(Asm::inst_and, ret, param[1]);
			break;
		case OperatorIntBitOr:
			add_cmd(Asm::inst_mov, ret, param[0]);
			add_cmd(Asm::inst_or, ret, param[1]);
			break;
		case OperatorIntShiftRight:
			add_cmd(Asm::inst_mov, param_reg(TypeInt, Asm::RegEcx), param[1]);
			add_cmd(Asm::inst_mov, ret, param[0]);
			add_cmd(Asm::inst_shr, ret, param_reg(TypeChar, Asm::RegCl));
			add_reg_channel(Asm::RegEcx, cmd.num - 3, cmd.num - 1);
			break;
		case OperatorIntShiftLeft:
			add_cmd(Asm::inst_mov, param_reg(TypeInt, Asm::RegEcx), param[1]);
			add_cmd(Asm::inst_mov, ret, param[0]);
			add_cmd(Asm::inst_shl, ret, param_reg(TypeChar, Asm::RegCl));
			add_reg_channel(Asm::RegEcx, cmd.num - 3, cmd.num - 1);
			break;
		case OperatorIntNegate:
			add_cmd(Asm::inst_mov, ret, param_const(TypeInt, (void*)0x0));
			add_cmd(Asm::inst_sub, ret, param[0]);
			break;
		case OperatorIntIncrease:
			add_cmd(Asm::inst_add, param[0], param_const(TypeInt, (void*)0x1));
			break;
		case OperatorIntDecrease:
			add_cmd(Asm::inst_sub, param[0], param_const(TypeInt, (void*)0x1));
			break;
// float
		case OperatorFloatAddS:
		case OperatorFloatSubtractS:
		case OperatorFloatMultiplyS:
		case OperatorFloatDivideS:
			add_cmd(Asm::inst_fld, param[0]);
			if (com->link_nr==OperatorFloatAddS)			add_cmd(Asm::inst_fadd, param[1]);
			if (com->link_nr==OperatorFloatSubtractS)	add_cmd(Asm::inst_fsub, param[1]);
			if (com->link_nr==OperatorFloatMultiplyS)	add_cmd(Asm::inst_fmul, param[1]);
			if (com->link_nr==OperatorFloatDivideS)		add_cmd(Asm::inst_fdiv, param[1]);
			add_cmd(Asm::inst_fstp, param[0]);
			break;
		case OperatorFloatAdd:
		case OperatorFloatSubtract:
		case OperatorFloatMultiply:
		case OperatorFloatDivide:
			add_cmd(Asm::inst_fld, param[0]);
			if (com->link_nr==OperatorFloatAdd)			add_cmd(Asm::inst_fadd, param[1]);
			if (com->link_nr==OperatorFloatSubtract)		add_cmd(Asm::inst_fsub, param[1]);
			if (com->link_nr==OperatorFloatMultiply)		add_cmd(Asm::inst_fmul, param[1]);
			if (com->link_nr==OperatorFloatDivide)		add_cmd(Asm::inst_fdiv, param[1]);
			add_cmd(Asm::inst_fstp, ret);
			break;
		case OperatorFloatMultiplyFI:
			add_cmd(Asm::inst_fild, param[1]);
			add_cmd(Asm::inst_fmul, param[0]);
			add_cmd(Asm::inst_fstp, ret);
			break;
		case OperatorFloatMultiplyIF:
			add_cmd(Asm::inst_fild, param[0]);
			add_cmd(Asm::inst_fmul, param[1]);
			add_cmd(Asm::inst_fstp, ret);
			break;
		case OperatorFloatEqual:
			add_cmd(Asm::inst_fld, param[0]);
			add_cmd(Asm::inst_fld, param[1]);
			add_cmd(Asm::inst_fucompp, p_st0, p_st1);
			add_cmd(Asm::inst_fnstsw, p_ax);
			add_cmd(Asm::inst_and, p_ah, param_const(TypeChar, (void*)0x45));
			add_cmd(Asm::inst_cmp, p_ah, param_const(TypeChar, (void*)0x40));
			add_cmd(Asm::inst_setz, ret);
			add_reg_channel(Asm::RegEax, cmd.num - 4, cmd.num - 2);
			break;
		case OperatorFloatNotEqual:
			add_cmd(Asm::inst_fld, param[0]);
			add_cmd(Asm::inst_fld, param[1]);
			add_cmd(Asm::inst_fucompp, p_st0, p_st1);
			add_cmd(Asm::inst_fnstsw, p_ax);
			add_cmd(Asm::inst_and, p_ah, param_const(TypeChar, (void*)0x45));
			add_cmd(Asm::inst_cmp, p_ah, param_const(TypeChar, (void*)0x40));
			add_cmd(Asm::inst_setnz, ret);
			add_reg_channel(Asm::RegEax, cmd.num - 4, cmd.num - 2);
			break;
		case OperatorFloatGreater:
			add_cmd(Asm::inst_fld, param[1]);
			add_cmd(Asm::inst_fld, param[0]);
			add_cmd(Asm::inst_fucompp, p_st0, p_st1);
			add_cmd(Asm::inst_fnstsw, p_ax);
			add_cmd(Asm::inst_test, p_ah, param_const(TypeChar, (void*)0x45));
			add_cmd(Asm::inst_setz, ret);
			add_reg_channel(Asm::RegEax, cmd.num - 3, cmd.num - 2);
			break;
		case OperatorFloatGreaterEqual:
			add_cmd(Asm::inst_fld, param[1]);
			add_cmd(Asm::inst_fld, param[0]);
			add_cmd(Asm::inst_fucompp, p_st0, p_st1);
			add_cmd(Asm::inst_fnstsw, p_ax);
			add_cmd(Asm::inst_test, p_ah, param_const(TypeChar, (void*)0x05));
			add_cmd(Asm::inst_setz, ret);
			add_reg_channel(Asm::RegEax, cmd.num - 3, cmd.num - 2);
			break;
		case OperatorFloatSmaller:
			add_cmd(Asm::inst_fld, param[0]);
			add_cmd(Asm::inst_fld, param[1]);
			add_cmd(Asm::inst_fucompp, p_st0, p_st1);
			add_cmd(Asm::inst_fnstsw, p_ax);
			add_cmd(Asm::inst_test, p_ah, param_const(TypeChar, (void*)0x45));
			add_cmd(Asm::inst_setz, ret);
			add_reg_channel(Asm::RegEax, cmd.num - 3, cmd.num - 2);
			break;
		case OperatorFloatSmallerEqual:
			add_cmd(Asm::inst_fld, param[0]);
			add_cmd(Asm::inst_fld, param[1]);
			add_cmd(Asm::inst_fucompp, p_st0, p_st1);
			add_cmd(Asm::inst_fnstsw, p_ax);
			add_cmd(Asm::inst_test, p_ah, param_const(TypeChar, (void*)0x05));
			add_cmd(Asm::inst_setz, ret);
			add_reg_channel(Asm::RegEax, cmd.num - 3, cmd.num - 2);
			break;
		case OperatorFloatNegate:
			add_cmd(Asm::inst_mov, ret, param[0]);
			add_cmd(Asm::inst_xor, ret, param_const(TypeInt, (void*)0x80000000));
			break;
// complex
		case OperatorComplexAddS:
		case OperatorComplexSubtractS:
		//case OperatorComplexMultiplySCF:
		//case OperatorComplexDivideS:
			add_cmd(Asm::inst_fld, param_shift(param[0], 0, TypeFloat));
			if (com->link_nr == OperatorComplexAddS)			add_cmd(Asm::inst_fadd, param_shift(param[1], 0, TypeFloat));
			if (com->link_nr == OperatorComplexSubtractS)	add_cmd(Asm::inst_fsub, param_shift(param[1], 0, TypeFloat));
			add_cmd(Asm::inst_fstp, param_shift(param[0], 0, TypeFloat));
			add_cmd(Asm::inst_fld, param_shift(param[0], 4, TypeFloat));
			if (com->link_nr == OperatorComplexAddS)			add_cmd(Asm::inst_fadd, param_shift(param[1], 4, TypeFloat));
			if (com->link_nr == OperatorComplexSubtractS)	add_cmd(Asm::inst_fsub, param_shift(param[1], 4, TypeFloat));
			add_cmd(Asm::inst_fstp, param_shift(param[0], 4, TypeFloat));
			break;
		case OperatorComplexAdd:
		case OperatorComplexSubtract:
//		case OperatorFloatMultiply:
//		case OperatorFloatDivide:
			add_cmd(Asm::inst_fld, param_shift(param[0], 0, TypeFloat));
			if (com->link_nr == OperatorComplexAdd)		add_cmd(Asm::inst_fadd, param_shift(param[1], 0, TypeFloat));
			if (com->link_nr == OperatorComplexSubtract)	add_cmd(Asm::inst_fsub, param_shift(param[1], 0, TypeFloat));
			add_cmd(Asm::inst_fstp, param_shift(ret, 0, TypeFloat));
			add_cmd(Asm::inst_fld, param_shift(param[0], 4, TypeFloat));
			if (com->link_nr == OperatorComplexAdd)		add_cmd(Asm::inst_fadd, param_shift(param[1], 4, TypeFloat));
			if (com->link_nr == OperatorComplexSubtract)	add_cmd(Asm::inst_fsub, param_shift(param[1], 4, TypeFloat));
			add_cmd(Asm::inst_fstp, param_shift(ret, 4, TypeFloat));
			break;
		case OperatorComplexMultiply:
			// r.x = a.y * b.y
			add_cmd(Asm::inst_fld, param_shift(param[0], 4, TypeFloat));
			add_cmd(Asm::inst_fmul, param_shift(param[1], 4, TypeFloat));
			add_cmd(Asm::inst_fstp, param_shift(ret, 0, TypeFloat));
			// r.x = a.x * b.x - r.x
			add_cmd(Asm::inst_fld, param_shift(param[0], 0, TypeFloat));
			add_cmd(Asm::inst_fmul, param_shift(param[1], 0, TypeFloat));
			add_cmd(Asm::inst_fsub, param_shift(ret, 0, TypeFloat));
			add_cmd(Asm::inst_fstp, param_shift(ret, 0, TypeFloat));
			// r.y = a.y * b.x
			add_cmd(Asm::inst_fld, param_shift(param[0], 4, TypeFloat));
			add_cmd(Asm::inst_fmul, param_shift(param[1], 0, TypeFloat));
			add_cmd(Asm::inst_fstp, param_shift(ret, 4, TypeFloat));
			// r.y += a.x * b.y
			add_cmd(Asm::inst_fld, param_shift(param[0], 0, TypeFloat));
			add_cmd(Asm::inst_fmul, param_shift(param[1], 4, TypeFloat));
			add_cmd(Asm::inst_fadd, param_shift(ret, 4, TypeFloat));
			add_cmd(Asm::inst_fstp, param_shift(ret, 4, TypeFloat));
			break;
		case OperatorComplexMultiplyFC:
			add_cmd(Asm::inst_fld, param[0]);
			add_cmd(Asm::inst_fmul, param_shift(param[1], 0, TypeFloat));
			add_cmd(Asm::inst_fstp, param_shift(ret, 0, TypeFloat));
			add_cmd(Asm::inst_fld, param[0]);
			add_cmd(Asm::inst_fmul, param_shift(param[1], 4, TypeFloat));
			add_cmd(Asm::inst_fstp, param_shift(ret, 4, TypeFloat));
			break;
		case OperatorComplexMultiplyCF:
			add_cmd(Asm::inst_fld, param_shift(param[0], 0, TypeFloat));
			add_cmd(Asm::inst_fmul, param[1]);
			add_cmd(Asm::inst_fstp, param_shift(ret, 0, TypeFloat));
			add_cmd(Asm::inst_fld, param_shift(param[0], 4, TypeFloat));
			add_cmd(Asm::inst_fmul, param[1]);
			add_cmd(Asm::inst_fstp, param_shift(ret, 4, TypeFloat));
			break;
		case OperatorComplexEqual:
			add_cmd(Asm::inst_fld, param_shift(param[0], 0, TypeFloat));
			add_cmd(Asm::inst_fld, param_shift(param[1], 0, TypeFloat));
			add_cmd(Asm::inst_fucompp, p_st0, p_st1);
			add_cmd(Asm::inst_fnstsw, p_ax);
			add_cmd(Asm::inst_and, p_ah, param_const(TypeChar, (void*)0x45));
			add_cmd(Asm::inst_cmp, p_ah, param_const(TypeChar, (void*)0x40));
			add_cmd(Asm::inst_setz, ret);
			add_reg_channel(Asm::RegEax, cmd.num - 4, cmd.num - 2);
			add_cmd(Asm::inst_fld, param_shift(param[0], 4, TypeFloat));
			add_cmd(Asm::inst_fld, param_shift(param[1], 4, TypeFloat));
			add_cmd(Asm::inst_fucompp, p_st0, p_st1);
			add_cmd(Asm::inst_fnstsw, p_ax);
			add_cmd(Asm::inst_and, p_ah, param_const(TypeChar, (void*)0x45));
			add_cmd(Asm::inst_cmp, p_ah, param_const(TypeChar, (void*)0x40));
			add_cmd(Asm::inst_setz, p_ah);
			add_cmd(Asm::inst_and, ret, p_ah);
			add_reg_channel(Asm::RegEax, cmd.num - 5, cmd.num - 1);
			break;
// bool/char
		case OperatorCharEqual:
		case OperatorCharNotEqual:
		case OperatorBoolEqual:
		case OperatorBoolNotEqual:
		case OperatorBoolGreater:
		case OperatorBoolGreaterEqual:
		case OperatorBoolSmaller:
		case OperatorBoolSmallerEqual:
			add_cmd(Asm::inst_cmp, param[0], param[1]);
			if ((com->link_nr == OperatorCharEqual) || (com->link_nr == OperatorBoolEqual))
				add_cmd(Asm::inst_setz, ret);
			else if ((com->link_nr ==OperatorCharNotEqual) || (com->link_nr == OperatorBoolNotEqual))
				add_cmd(Asm::inst_setnz, ret);
			else if (com->link_nr == OperatorBoolGreater)		add_cmd(Asm::inst_setnle, ret);
			else if (com->link_nr == OperatorBoolGreaterEqual)	add_cmd(Asm::inst_setnl, ret);
			else if (com->link_nr == OperatorBoolSmaller)		add_cmd(Asm::inst_setl, ret);
			else if (com->link_nr == OperatorBoolSmallerEqual)	add_cmd(Asm::inst_setle, ret);
			break;
		case OperatorBoolAnd:
			add_cmd(Asm::inst_mov, ret, param[0]);
			add_cmd(Asm::inst_and, ret, param[1]);
			break;
		case OperatorBoolOr:
			add_cmd(Asm::inst_mov, ret, param[0]);
			add_cmd(Asm::inst_or, ret, param[1]);
			break;
		case OperatorCharAddS:
			add_cmd(Asm::inst_add, param[0], param[1]);
			break;
		case OperatorCharSubtractS:
			add_cmd(Asm::inst_sub, param[0], param[1]);
			break;
		case OperatorCharAdd:
			add_cmd(Asm::inst_mov, ret, param[0]);
			add_cmd(Asm::inst_add, ret, param[1]);
			break;
		case OperatorCharSubtract:
			add_cmd(Asm::inst_mov, ret, param[0]);
			add_cmd(Asm::inst_sub, ret, param[1]);
			break;
		case OperatorCharBitAnd:
			add_cmd(Asm::inst_mov, ret, param[0]);
			add_cmd(Asm::inst_and, ret, param[1]);
			break;
		case OperatorCharBitOr:
			add_cmd(Asm::inst_mov, ret, param[0]);
			add_cmd(Asm::inst_or, ret, param[1]);
			break;
		case OperatorBoolNegate:
			add_cmd(Asm::inst_mov, ret, param[0]);
			add_cmd(Asm::inst_xor, ret, param_const(TypeBool, (char*)0x1));
			break;
		case OperatorCharNegate:
			add_cmd(Asm::inst_mov, ret, param_const(TypeChar, (char*)0x0));
			add_cmd(Asm::inst_sub, ret, param[0]);
			break;
// vector
		case OperatorVectorAddS:
			for (int i=0;i<3;i++){
				add_cmd(Asm::inst_fld, param_shift(param[0], i * 4, TypeFloat));
				add_cmd(Asm::inst_fadd, param_shift(param[1], i * 4, TypeFloat));
				add_cmd(Asm::inst_fstp, param_shift(param[0], i * 4, TypeFloat));
			}
			break;
		case OperatorVectorMultiplyS:
			for (int i=0;i<3;i++){
				add_cmd(Asm::inst_fld, param_shift(param[0], i * 4, TypeFloat));
				add_cmd(Asm::inst_fmul, param[1]);
				add_cmd(Asm::inst_fstp, param_shift(param[0], i * 4, TypeFloat));
			}
			break;
		case OperatorVectorDivideS:
			for (int i=0;i<3;i++){
				add_cmd(Asm::inst_fld, param_shift(param[0], i * 4, TypeFloat));
				add_cmd(Asm::inst_fdiv, param[1]);
				add_cmd(Asm::inst_fstp, param_shift(param[0], i * 4, TypeFloat));
			}
			break;
		case OperatorVectorSubtractS:
			for (int i=0;i<3;i++){
				add_cmd(Asm::inst_fld, param_shift(param[0], i * 4, TypeFloat));
				add_cmd(Asm::inst_fsub, param_shift(param[1], i * 4, TypeFloat));
				add_cmd(Asm::inst_fstp, param_shift(param[0], i * 4, TypeFloat));
			}
			break;
		case OperatorVectorAdd:
			for (int i=0;i<3;i++){
				add_cmd(Asm::inst_fld, param_shift(param[0], i * 4, TypeFloat));
				add_cmd(Asm::inst_fadd, param_shift(param[1], i * 4, TypeFloat));
				add_cmd(Asm::inst_fstp, param_shift(ret, i * 4, TypeFloat));
			}
			break;
		case OperatorVectorSubtract:
			for (int i=0;i<3;i++){
				add_cmd(Asm::inst_fld, param_shift(param[0], i * 4, TypeFloat));
				add_cmd(Asm::inst_fsub, param_shift(param[1], i * 4, TypeFloat));
				add_cmd(Asm::inst_fstp, param_shift(ret, i * 4, TypeFloat));
			}
			break;
		case OperatorVectorMultiplyVF:
			for (int i=0;i<3;i++){
				add_cmd(Asm::inst_fld, param_shift(param[0], i * 4, TypeFloat));
				add_cmd(Asm::inst_fmul, param[1]);
				add_cmd(Asm::inst_fstp, param_shift(ret, i * 4, TypeFloat));
			}
			break;
		case OperatorVectorMultiplyFV:
			for (int i=0;i<3;i++){
				add_cmd(Asm::inst_fld, param[0]);
				add_cmd(Asm::inst_fmul, param_shift(param[1], i * 4, TypeFloat));
				add_cmd(Asm::inst_fstp, param_shift(ret, i * 4, TypeFloat));
			}
			break;
		case OperatorVectorDivideVF:
			for (int i=0;i<3;i++){
				add_cmd(Asm::inst_fld, param_shift(param[0], i * 4, TypeFloat));
				add_cmd(Asm::inst_fdiv, param[1]);
				add_cmd(Asm::inst_fstp, param_shift(ret, i * 4, TypeFloat));
			}
			break;
		case OperatorVectorNegate:
			for (int i=0;i<3;i++){
				add_cmd(Asm::inst_mov, param_shift(ret, i * 4, TypeFloat), param_shift(param[0], i * 4, TypeFloat));
				add_cmd(Asm::inst_xor, param_shift(ret, i * 4, TypeFloat), param_const(TypeInt, (void*)0x80000000));
			}
			break;
		default:
			DoError("unimplemented operator: " + PreOperators[com->link_nr].str());
	}
}


SerialCommandParam Serializer::SerializeCommand(Command *com, int level, int index)
{
	msg_db_f("SerializeCommand", 4);

	// for/while need a marker to this point
	int marker_before_params = -1;
	if ((com->kind == KindCompilerFunction) && ((com->link_nr == CommandWhile) || (com->link_nr == CommandFor)))
		marker_before_params = add_marker();

	// return value
	SerialCommandParam ret;
	bool create_constructor_for_return = ((com->kind != KindCompilerFunction) && (com->kind != KindFunction));
	add_temp(com->type, ret, create_constructor_for_return);

	// compile parameters
	SerialCommandParam param[SCRIPT_MAX_PARAMS];
	for (int p=0;p<com->num_params;p++)
		SerializeParameter(com->param[p], level, index, param[p]);

	// class function -> compile instance
	bool is_class_function = false;
	if (com->kind == KindFunction){
		if (com->script->syntax->Functions[com->link_nr]->_class)
			is_class_function = true;
	}
	SerialCommandParam instance = {-1, NULL, NULL};
	if (is_class_function){
		so("member");
		SerializeParameter(com->instance, level, index, instance);
		so(Kind2Str(instance.kind));
		// super_array automatically referenced...
	}

	    
	if (com->kind == KindOperator){
		SerializeOperator(com, param, ret);
		
	}else if (com->kind == KindFunction){

		for (int p=0;p<com->num_params;p++)
			AddFuncParam(param[p]);
			
		AddFuncReturn(ret);
			
		if (is_class_function)
			AddFuncInstance(instance);
			
		AddFunctionCall(com->script, com->link_nr);
	}else if (com->kind == KindCompilerFunction){
			switch(com->link_nr){
				/*case CommandSine:
					break;*/
				case CommandIf:{
					// cmp;  jz m;  -block-  m;
					add_cmd(Asm::inst_cmp, param[0], param_const(TypeBool, (void*)0x0));
					int m_after_true = add_marker_after_command(level, index + 1);
					add_cmd(Asm::inst_jz, param_marker(m_after_true));
					}break;
				case CommandIfElse:{
					// cmp;  jz m1;  -block-  jmp m2;  m1;  -block-  m2;
					add_cmd(Asm::inst_cmp, param[0], param_const(TypeBool, (void*)0x0));
					int m_after_true = add_marker_after_command(level, index + 1);
					int m_after_false = add_marker_after_command(level, index + 2);
					add_cmd(Asm::inst_jz, param_marker(m_after_true)); // jz ...
					add_jump_after_command(level, index + 1, m_after_false); // insert before <m_after_true> is inserted!
					}break;
				case CommandWhile:
				case CommandFor:{
					// m1;  cmp;  jz m2;  -block-             jmp m1;  m2;     (while)
					// m1;  cmp;  jz m2;  -block-  m3;  i++;  jmp m1;  m2;     (for)
					add_cmd(Asm::inst_cmp, param[0], param_const(TypeBool, (void*)0x0));
					int marker_after_while = add_marker_after_command(level, index + 1);
					add_cmd(Asm::inst_jz, param_marker(marker_after_while));
					add_jump_after_command(level, index + 1, marker_before_params); // insert before <marker_after_while> is inserted!

					int marker_continue = marker_before_params;
					if (com->link_nr == CommandFor){
						// NextCommand is a block!
						if (NextCommand->kind != KindBlock)
							DoError("command block in \"for\" loop missing");
						marker_continue = add_marker_after_command(level + 1, syntax_tree->Blocks[NextCommand->link_nr]->command.num - 2);
					}
					LoopData l = {marker_continue, marker_after_while, level, index};
					loop.add(l);
					}break;
				case CommandBreak:
					add_cmd(Asm::inst_jmp, param_marker(loop.back().marker_break));
					break;
				case CommandContinue:
					add_cmd(Asm::inst_jmp, param_marker(loop.back().marker_continue));
					break;
				case CommandReturn:
					if (com->num_params > 0){
						if (cur_func->return_type->UsesReturnByMemory()){ // we already got a return address in [ebp+0x08] (> 4 byte)
							FillInDestructors(false);
							// internally handled...
#if 0
							int s = mem_align(cur_func->return_type->size);

							// slow
							/*SerialCommandParam p, p_deref;
							p.kind = KindVarLocal;
							p.type = TypeReg32;
							p.p = (char*) 0x8;
							p.shift = 0;
							for (int j=0;j<s/4;j++){
								AddDereference(p, p_deref);
								add_cmd(Asm::inst_mov, p_deref, param_shift(param[0], j * 4, TypeInt));
								add_cmd(Asm::inst_add, p, param_const(TypeInt, (void*)0x4));
							}*/

							// test
							SerialCommandParam p_edx = param_reg(TypeReg32, Asm::RegEdx), p_deref_edx;
							SerialCommandParam p_ret_addr;
							p_ret_addr.kind = KindVarLocal;
							p_ret_addr.type = TypeReg32;
							p_ret_addr.p = (char*)0x8;
							p_ret_addr.shift = 0;
							int c_0 = cmd.num;
							add_cmd(Asm::inst_mov, p_edx, p_ret_addr);
							AddDereference(p_edx, p_deref_edx, TypeReg32);
							for (int j=0;j<s/4;j++)
								add_cmd(Asm::inst_mov, param_shift(p_deref_edx, j * 4, TypeInt), param_shift(param[0], j * 4, TypeInt));
							add_reg_channel(Asm::RegEdx, c_0, cmd.num - 1);
#endif

							add_cmd(Asm::inst_ret);
							if (config.instruction_set == Asm::InstructionSetX86)
								add_cmd(Asm::inst_ret, param_const(TypeReg16, (void*)4));
							else
								add_cmd(Asm::inst_ret);
						}else{ // store return directly in eax / fpu stack (4 byte)
							SerialCommandParam t;
							add_temp(cur_func->return_type, t);
							add_cmd(Asm::inst_mov, t, param[0]);
							FillInDestructors(false);
							if (cur_func->return_type == TypeFloat){
								if (config.instruction_set == Asm::InstructionSetAMD64)
									add_cmd(Asm::inst_movss, param_reg(TypeReg128, Asm::RegXmm0), t);
								else
									add_cmd(Asm::inst_fld, t);
							}else if (cur_func->return_type->size == 1)
								add_cmd(Asm::inst_mov, param_reg(cur_func->return_type, Asm::RegAl), t);
							else
								add_cmd(Asm::inst_mov, param_reg(cur_func->return_type, Asm::RegEax), t);
							add_cmd(Asm::inst_leave);
							add_cmd(Asm::inst_ret);
						}
					}else{
						FillInDestructors(false);
						add_cmd(Asm::inst_leave);
						add_cmd(Asm::inst_ret);
					}
					break;
				case CommandWaitOneFrame:
				case CommandWait:
				case CommandWaitRT:{
				// set waiting state
					// GlobalWaitingMode = mode
					// GlobalWaitingTime = time
					SerialCommandParam p_mode = param_global(TypeInt, &GlobalWaitingMode);
					SerialCommandParam p_ttw = param_global(TypeFloat, &GlobalTimeToWait);
					if (com->link_nr == CommandWaitOneFrame){
						add_cmd(Asm::inst_mov, p_mode, param_const(TypeInt, (void*)WaitingModeRT));
						add_cmd(Asm::inst_mov, p_ttw, param_const(TypeFloat, NULL));
					}else if (com->link_nr == CommandWait){
						add_cmd(Asm::inst_mov, p_mode, param_const(TypeInt, (void*)WaitingModeGT));
						add_cmd(Asm::inst_mov, p_ttw, param[0]);
					}else if (com->link_nr == CommandWaitRT){
						add_cmd(Asm::inst_mov, p_mode, param_const(TypeInt, (void*)WaitingModeRT));
						add_cmd(Asm::inst_mov, p_ttw, param[0]);
					}
					
				// save script state
					// stack[ -8] = ebp
					// stack[-12] = esp
					// stack[-16] = eip
					add_cmd(Asm::inst_mov, p_eax, param_const(TypePointer, &script->Stack[config.StackSize-8]));
					add_cmd(Asm::inst_mov, p_deref_eax, param_reg(TypeReg32, Asm::RegEbp));
					add_cmd(Asm::inst_mov, p_eax, param_const(TypePointer, &script->Stack[config.StackSize-12]));
					add_cmd(Asm::inst_mov, p_deref_eax, param_reg(TypeReg32, Asm::RegEsp));
					add_cmd(Asm::inst_mov, param_reg(TypeReg32, Asm::RegEsp), param_const(TypePointer, &script->Stack[config.StackSize-12]));
					add_cmd(Asm::inst_call, param_const(TypePointer, NULL)); // push eip
				// load return
					// mov esp, &stack[-4]
					// pop esp
					// mov ebp, esp
					// leave
					// ret
					add_cmd(Asm::inst_mov, param_reg(TypeReg32, Asm::RegEsp), param_const(TypePointer, &script->Stack[config.StackSize-4])); // start of the script stack
					add_cmd(Asm::inst_pop, param_reg(TypeReg32, Asm::RegEsp)); // old stackpointer (real program)
					add_cmd(Asm::inst_mov, param_reg(TypeReg32, Asm::RegEbp), param_reg(TypeReg32, Asm::RegEsp));
					add_cmd(Asm::inst_leave);
					add_cmd(Asm::inst_ret);
				// here comes the "waiting"...

				// reload script state (eip already loaded)
					// ebp = &stack[-8]
					// esp = &stack[-12]
					// GlobalWaitingMode = WaitingModeNone
					add_cmd(Asm::inst_mov, p_eax, param_const(TypePointer, &script->Stack[config.StackSize-8]));
					add_cmd(Asm::inst_mov, param_reg(TypeReg32, Asm::RegEbp), p_deref_eax);
					add_cmd(Asm::inst_mov, p_eax, param_const(TypePointer, &script->Stack[config.StackSize-12]));
					add_cmd(Asm::inst_mov, param_reg(TypeReg32, Asm::RegEsp), p_deref_eax);
					add_cmd(Asm::inst_mov, p_mode, param_const(TypeInt, (void*)WaitingModeNone));
					}break;
				case CommandIntToFloat:
					add_cmd(Asm::inst_fild, param[0]);
					add_cmd(Asm::inst_fstp, ret);
					break;
				case CommandFloatToInt:
					// round to nearest...
					//add_cmd(Asm::inst_fld, param[0]);
					//add_cmd(Asm::inst_fistp, ret);

					// round to zero...
					SerialCommandParam t1, t2;
					add_temp(TypeReg16, t1);
					add_temp(TypeInt, t2);
					add_cmd(Asm::inst_fld, param[0]);
					add_cmd(Asm::inst_fnstcw, t1);
					add_cmd(Asm::inst_movzx, p_eax, t1);
					add_cmd(Asm::inst_mov, p_ah, param_const(TypeChar, (void*)0x0c));
					add_cmd(Asm::inst_mov, t2, p_eax);
					add_reg_channel(Asm::RegEax, cmd.num - 3, cmd.num - 1);
					add_cmd(Asm::inst_fldcw, param_shift(t2, 0, TypeReg16));
					add_cmd(Asm::inst_fistp, ret);
					add_cmd(Asm::inst_fldcw, t1);
					break;
				case CommandIntToChar:
					add_cmd(Asm::inst_mov, p_eax_int, param[0]);
					add_cmd(Asm::inst_mov, ret, p_al_char);
					add_reg_channel(Asm::RegEax, cmd.num - 2, cmd.num - 1);
					break;
				case CommandCharToInt:
					add_cmd(Asm::inst_mov, p_eax_int, param_const(TypeInt, (void*)0x0));
					add_cmd(Asm::inst_mov, p_al_char, param[0]);
					add_cmd(Asm::inst_mov, ret, p_eax);
					add_reg_channel(Asm::RegEax, cmd.num - 3, cmd.num - 1);
					break;
				case CommandPointerToBool:
					add_cmd(Asm::inst_cmp, param[0], param_const(TypePointer, NULL));
					add_cmd(Asm::inst_setnz, ret);
					break;
				case CommandAsm:
					add_cmd(inst_asm);
					break;
				case CommandRectSet:
					add_cmd(Asm::inst_mov, param_shift(ret, 12, TypeFloat), param[3]);
				case CommandVectorSet:
					add_cmd(Asm::inst_mov, param_shift(ret, 8, TypeFloat), param[2]);
				case CommandComplexSet:
					add_cmd(Asm::inst_mov, param_shift(ret, 4, TypeFloat), param[1]);
					add_cmd(Asm::inst_mov, param_shift(ret, 0, TypeFloat), param[0]);
					break;
				case CommandColorSet:
					add_cmd(Asm::inst_mov, param_shift(ret, 12, TypeFloat), param[0]);
					add_cmd(Asm::inst_mov, param_shift(ret, 0, TypeFloat), param[1]);
					add_cmd(Asm::inst_mov, param_shift(ret, 4, TypeFloat), param[2]);
					add_cmd(Asm::inst_mov, param_shift(ret, 8, TypeFloat), param[3]);
					break;
				default:
					DoError("compiler function unimplemented: " + PreCommands[com->link_nr].name);
			}
	}else if (com->kind == KindBlock){
		SerializeBlock(syntax_tree->Blocks[com->link_nr], level + 1);
	}else{
		msg_write(Kind2Str(com->kind));
		//so("---???");
		//DoError(string("type of command is unimplemented (call Michi!): ",Kind2Str(com->Kind)));
	}
	return ret;
}

void Serializer::SerializeBlock(Block *block, int level)
{
	msg_db_f("SerializeBlock", 4);
	for (int i=0;i<block->command.num;i++){
		//so(string2("%d - %d",i,block->NumCommands));
		StackOffset = cur_func->_var_size;
		NextCommand = NULL;
		if (block->command.num > i + 1)
			NextCommand = block->command[i + 1];

		// serialize
		SerializeCommand(block->command[i], level, i);
		
		// destruct new temp vars
		FillInDestructors(true);

		// any markers / jumps to add?
		for (int j=add_later.num-1;j>=0;j--)
			if ((level == add_later[j].level) && (i == add_later[j].index)){
				if (add_later[j].kind == StuffKindMarker)
					add_marker(add_later[j].marker);
				else if (add_later[j].kind == StuffKindJump)
					add_cmd(Asm::inst_jmp, param_marker(add_later[j].marker));
				add_later.erase(j);
			}

		// end of loop?
		if (loop.num > 0)
			if ((loop.back().level == level) && (loop.back().index == i - 1))
				loop.pop();
	}
}

// modus: KindVarLocal/KindVarTemp
//    -1: -return-   -> don't destruct
void Serializer::add_cmd_constructor(SerialCommandParam &param, int modus)
{
	Type *class_type = param.type;
	if (modus == -1)
		class_type = class_type->parent;
	ClassFunction *f = class_type->GetConstructor();
	if (!f)
		return;
	if (modus == -1){
		AddFuncInstance(param);
	}else{
		SerialCommandParam inst;
		AddReference(param, TypePointer, inst);
		AddFuncInstance(inst);
	}

	AddFunctionCall(class_type->owner->script, f->nr);
	if (modus == KindVarTemp)
		InsertedConstructorTemp.add(param);
	else if (modus == KindVarLocal)
		InsertedConstructorFunc.add(param);
}

void Serializer::add_cmd_destructor(SerialCommandParam &param)
{
	ClassFunction *f = param.type->GetDestructor();
	if (!f)
		return;
	SerialCommandParam inst;
	AddReference(param, TypePointer, inst);
	AddFuncInstance(inst);
	AddFunctionCall(param.type->owner->script, f->nr);
}

void Serializer::FillInConstructorsFunc()
{
	msg_db_f("FillInConstructorsFunc", 4);
	foreach(LocalVariable &v, cur_func->var){
		SerialCommandParam param = param_local(v.type, v._offset);
		add_cmd_constructor(param, (v.name == "-return-") ? -1 : KindVarLocal);
	}
}

void Serializer::FillInDestructors(bool from_temp)
{
	msg_db_f("FillInDestructors", 4);
	if (from_temp){
		for (int i=0;i<InsertedConstructorTemp.num;i++)
			add_cmd_destructor(InsertedConstructorTemp[i]);
		InsertedConstructorTemp.clear();
	}else{
		for (int i=0;i<InsertedConstructorFunc.num;i++)
			add_cmd_destructor(InsertedConstructorFunc[i]);
	}
}

int Serializer::temp_in_cmd(int c, int v)
{
	if (cmd[c].inst >= inst_marker)
		return 0;
	int r = 0;
	if ((cmd[c].p1.kind == KindVarTemp) || (cmd[c].p1.kind == KindDerefVarTemp))
		if ((long)cmd[c].p1.p == v)
			r = 1 + ((cmd[c].p1.kind == KindDerefVarTemp) ? 4 : 0);
	if ((cmd[c].p2.kind == KindVarTemp) || (cmd[c].p2.kind == KindDerefVarTemp))
		if ((long)cmd[c].p2.p == v)
			r += 2 + ((cmd[c].p2.kind == KindDerefVarTemp) ? 8 : 0);
	return r;
}

void Serializer::ScanTempVarUsage()
{
	msg_db_f("ScanTempVarUsage", 4);
	foreachi(TempVar &v, temp_var, i){
		v.first = -1;
		v.last = -1;
		v.count = 0;
		for (int c=0;c<cmd.num;c++){
			if (temp_in_cmd(c, i) > 0){
				v.count ++;
				if (v.first < 0)
					v.first = c;
				v.last = c;
			}
		}
		//so(string2("var %d:   %d - %d", i, v.first, v.last));
	}
	TempVarRangesDefined = true;
}

inline bool param_is_simple(SerialCommandParam &p)
{
	return ((p.kind == KindRegister) || (p.kind == KindVarTemp) || (p.kind < 0));
}

inline bool param_combi_allowed(int inst, SerialCommandParam &p1, SerialCommandParam &p2)
{
//	if (inst >= Asm::inst_marker)
//		return true;
	if ((!param_is_simple(p1)) && (!param_is_simple(p2)))
		return false;
	bool r1, w1, r2, w2;
	Asm::GetInstructionParamFlags(inst, r1, w1, r2, w2);
	if ((w1) && (p1.kind == KindConstant))
		return false;
	if ((w2) && (p2.kind == KindConstant))
		return false;
	if ((p1.kind == KindConstant) || (p2.kind == KindConstant))
		if (!Asm::GetInstructionAllowConst(inst))
			return false;
	return true;
}

// mov [0x..] [0x...]  ->  mov temp, [0x..]   mov [0x..] temp
/*void CorrectUnallowedParamCombis()
{
	msg_db_f("CorrectCombis", 3);
	for (int i=cmd.num-1;i>=0;i--)
		if (!param_combi_allowed(cmd[i].inst, cmd[i].p1, cmd[i].p2)){
			msg_write(string2("correcting param combi  cmd=%d", i));
			bool mov_first_param = (cmd[i].p2.kind < 0) || (cmd[i].p1.kind == KindRefToConst) || (cmd[i].p1.kind == KindConstant);
			SerialCommandParam *pp = mov_first_param ? &cmd[i].p1 : &cmd[i].p2;
			SerialCommandParam temp, p = *pp;
			add_temp(p.type, temp);

			*pp = temp;
			if (p.type->Size == 1)
				add_cmd(Asm::inst_mov, temp, p);
			else
				add_cmd(Asm::inst_mov, temp, p);
			move_last_cmd(i);
			//so("...ok");
		}
	ScanTempVarUsage();
}*/

// mov [0x..] [0x...]  ->  mov eax, [0x..]   mov [0x..] eax    (etc)
void Serializer::CorrectUnallowedParamCombis()
{
	msg_db_f("CorrectCombis", 3);
	for (int i=cmd.num-1;i>=0;i--){
		if (cmd[i].inst >= inst_marker)
			continue;

		// bad?
		if (param_combi_allowed(cmd[i].inst, cmd[i].p1, cmd[i].p2))
			continue;

		// correct
		so(format("correcting param combi  cmd=%d", i));
		bool mov_first_param = (cmd[i].p2.kind < 0) || (cmd[i].p1.kind == KindRefToConst) || (cmd[i].p1.kind == KindConstant);
		SerialCommandParam *pp = mov_first_param ? &cmd[i].p1 : &cmd[i].p2;
		SerialCommandParam p = *pp;

		//msg_error("correct");
		//msg_write(p.type->name);
		*pp = param_reg(p.type, get_reg(0, p.type->size));
		add_cmd(Asm::inst_mov, *pp, p);
		move_last_cmd(i);
		//so("...ok");
	}
	so("scan");
	ScanTempVarUsage();
}

int Serializer::find_unused_reg(int first, int last, int size, bool allow_eax)
{
	for (int r=0;r<MapRegRoot.num;r++)
		if (!is_reg_root_used_in_interval(MapRegRoot[r], first, last))
			return get_reg(MapRegRoot[r], size);
	if (allow_eax)
		if (!is_reg_root_used_in_interval(0, first, last))
			return get_reg(0, size);
	return -1;
}

// inst ... [local] ...
// ->      mov reg, local     inst ...[reg]...
void Serializer::solve_deref_temp_local(int c, int np, bool is_local)
{
	SerialCommandParam *pp = (np == 0) ? &cmd[c].p1 : &cmd[c].p2;
	SerialCommandParam p = *pp;
	int shift = p.shift;

	Type *type_pointer = is_local ? TypePointer : temp_var[(long)p.p].type;
	Type *type_data = p.type;
	
	p.kind = is_local ? KindVarLocal : KindVarTemp;
	p.shift = 0;
	p.type = type_pointer;

	int reg = find_unused_reg(c, c, config.PointerSize, true);
	//so(reg);
	if (reg < 0)
		script->DoErrorInternal("solve_deref_temp_local... no registers available");
	SerialCommandParam p_reg = param_reg(type_pointer, reg);
	SerialCommandParam p_deref_reg;
	p_deref_reg.kind = KindDerefRegister;
	p_deref_reg.p = (char*)(long)reg;
	p_deref_reg.type = type_data;
	p_deref_reg.shift = 0;
	
	*pp = p_deref_reg;
		
	add_cmd(Asm::inst_mov, p_reg, p);
	move_last_cmd(c);
	if (shift > 0){
		msg_write("solve_deref_temp_local");
		add_cmd(Asm::inst_add, p_reg, param_const(TypeInt, (void*)(long)shift));
		move_last_cmd(c + 1);
		add_reg_channel(reg, c, c + 2);
	}else
		add_reg_channel(reg, c, c + 1);
}

#if 0
void ResolveDerefLocal()
{
	msg_db_f("ResolveDerefLocal", 3);
	for (int i=cmd.num-1;i>=0;i--){
		if (cmd[i].inst >= Asm::inst_marker)
			continue;
		bool dl1 = (cmd[i].p1.kind == KindDerefVarLocal);
		bool dl2 = (cmd[i].p2.kind == KindDerefVarLocal);
		if (!(dl1 || dl2))
			continue;
		
		so(string2("deref local... cmd=%d", i));
		if (!dl2){
			solve_deref_temp(i, 0);
			i ++;
		}else if (!dl1){
			solve_deref_temp(i, 1);
			i ++;
		}else{
			// hopefully... p2 is read-only

			int reg = find_unused_reg(i, i, true);
			so(reg);
			if (reg < 0)
				script->DoErrorInternal("deref local... both sides... .no registers available");
			SerialCommandParam p_reg = param_reg(TypeReg32, reg);
			add_reg_channel(reg, i, i); // temp
			
			int reg2 = find_unused_reg(i, i, true);
			so(reg2);
			if (reg2 < 0)
				script->DoErrorInternal("deref local... both sides... .no registers available");
			SerialCommandParam p_reg2 = param_reg(TypeReg32, reg2);
			SerialCommandParam p_deref_reg2;
			p_deref_reg2.kind = KindDerefRegister;
			p_deref_reg2.p = (char*)reg2;
			p_deref_reg2.type = TypePointer;
			p_deref_reg2.shift = 0;
			reg_channel.resize(reg_channel.num - 1); // remove temp reg channel...

			// inst [l1] [l2]
			// ->
			// mov reg2, l1
			// mov reg, [reg2]
			// mov reg2, l2
			// inst [reg2], reg
			SerialCommandParam p1 = cmd[i].p1;
			SerialCommandParam p2 = cmd[i].p2;
			if (p1.shift + p2.shift > 0)
				script->DoErrorInternal("deref local... both sides... shift");
			p1.kind = p2.kind = KindVarLocal;
			cmd[i].p1 = p_deref_reg2;
			cmd[i].p2 = p_reg;
	
			add_cmd(Asm::inst_mov, p_reg2, p2);
			move_last_cmd(i);
	
			add_cmd(Asm::inst_mov, p_reg, p_deref_reg2);
			move_last_cmd(i + 1);
	
			add_cmd(Asm::inst_mov, p_reg2, p1);
			move_last_cmd(i + 2);

			add_reg_channel(reg, i + 1, i + 3);
			add_reg_channel(reg2, i, i + 3);
				
			i += 3;
		}
	}
}
#endif


void Serializer::ResolveDerefTempAndLocal()
{
	msg_db_f("ResolveDerefTempAndLocal", 3);
	for (int i=cmd.num-1;i>=0;i--){
		if (cmd[i].inst >= inst_marker)
			continue;
		bool dl1 = ((cmd[i].p1.kind == KindDerefVarLocal) || (cmd[i].p1.kind == KindDerefVarTemp));
		bool dl2 = ((cmd[i].p2.kind == KindDerefVarLocal) || (cmd[i].p2.kind == KindDerefVarTemp));
		if (!(dl1 || dl2))
			continue;

		bool is_local1 = (cmd[i].p1.kind == KindDerefVarLocal);
		bool is_local2 = (cmd[i].p2.kind == KindDerefVarLocal);
		
		so(format("deref temp/local... cmd=%d", i));
		if (!dl2){
			solve_deref_temp_local(i, 0, is_local1);
			i ++;
		}else if (!dl1){
			solve_deref_temp_local(i, 1, is_local2);
			i ++;
		}else{
			// hopefully... p2 is read-only

			Type *type_pointer = TypePointer;
			Type *type_data = cmd[i].p1.type;

			int reg = find_unused_reg(i, i, type_data->size, true);
			so(reg);
			if (reg < 0)
				DoError("deref local... both sides... .no registers available");
			
			SerialCommandParam p_reg = param_reg(type_data, reg);
			add_reg_channel(reg, i, i); // temp
			
			int reg2 = find_unused_reg(i, i, config.PointerSize, true);
			so(reg2);
			if (reg2 < 0)
				DoError("deref temp/local... both sides... .no registers available");
			SerialCommandParam p_reg2 = param_reg(type_pointer, reg2);
			SerialCommandParam p_deref_reg2;
			p_deref_reg2.kind = KindDerefRegister;
			p_deref_reg2.p = (char*)(long)reg2;
			p_deref_reg2.type = type_data;
			p_deref_reg2.shift = 0;
			reg_channel.pop(); // remove temp reg channel...

			// inst [l1] [l2]
			// ->
			// mov reg2, l2
			//   (add reg2, shift2)
			// mov reg, [reg2]
			// mov reg2, l1
			//   (add reg2, shift1)
			// inst [reg2], reg
			SerialCommandParam p1 = cmd[i].p1;
			SerialCommandParam p2 = cmd[i].p2;
			long shift1 = p1.shift;
			long shift2 = p2.shift;
			p1.shift = p2.shift = 0;
			
			p1.kind = is_local1 ? KindVarLocal : KindVarTemp;
			p2.kind = is_local2 ? KindVarLocal : KindVarTemp;
			p1.type = type_pointer;
			p2.type = type_pointer;
			cmd[i].p1 = p_deref_reg2;
			cmd[i].p2 = p_reg;
			int cmd_pos = i;

			int r2_first = cmd_pos;
			add_cmd(Asm::inst_mov, p_reg2, p2);
			move_last_cmd(cmd_pos ++);

			if (shift2 > 0){
				msg_write("resolve deref temp&loc 2");
				add_cmd(Asm::inst_add, p_reg2, param_const(type_pointer, (void*)shift2));
				move_last_cmd(cmd_pos ++);
			}

			int r1_first = cmd_pos;
			add_cmd(Asm::inst_mov, p_reg, p_deref_reg2);
			move_last_cmd(cmd_pos ++);
	
			add_cmd(Asm::inst_mov, p_reg2, p1);
			move_last_cmd(cmd_pos ++);

			if (shift1 > 0){
				msg_write("resolve deref temp&loc 1");
				add_cmd(Asm::inst_add, p_reg2, param_const(type_pointer, (void*)shift1));
				move_last_cmd(cmd_pos ++);
			}

			add_reg_channel(reg, r1_first, cmd_pos);
			add_reg_channel(reg2, r2_first, cmd_pos);
				
			i = cmd_pos;
		}
	}
}

bool Serializer::ParamUntouchedInInterval(SerialCommandParam &p, int first, int last)
{
	// direct usage?
	for (int i=first;i<=last;i++)
		if ((cmd[i].p1 == p) || (cmd[i].p2 == p))
			return false;
	
	// registers may be more subtle..
	if ((p.kind == KindRegister) || (p.kind == KindDerefRegister)){
		for (int i=first;i<=last;i++){
			
			// call violates all!
			if (cmd[i].inst == Asm::inst_call)
				return false;

			// div violates eax and edx
			if (cmd[i].inst == Asm::inst_div)
				if (((long)p.p == Asm::RegEdx) || ((long)p.p == Asm::RegEax))
					return false;

			// registers used? (may be part of the same meta-register)
			if ((cmd[i].p1.kind == KindRegister) || (cmd[i].p1.kind == KindDerefRegister))
				if (Asm::RegRoot[(long)cmd[i].p1.p] == Asm::RegRoot[(long)p.p])
					return false;
			if ((cmd[i].p2.kind == KindRegister) || (cmd[i].p2.kind == KindDerefRegister))
				if (Asm::RegRoot[(long)cmd[i].p2.p] == Asm::RegRoot[(long)p.p])
					return false;
		}
	}
	return true;
}

void Serializer::SimplifyFPUStack()
{
	msg_db_f("SimplifyFPUStack", 3);

// fstp temp
// fld temp
	for (int vi=temp_var.num-1;vi>=0;vi--){
		TempVar &v = temp_var[vi];
		if (v.first < 0)
			continue;

		// may only appear two times
		if (v.count > 2)
			continue;

		// stored then loaded...?
		if ((cmd[v.first].inst != Asm::inst_fstp) || (cmd[v.last].inst != Asm::inst_fld))
			continue;

		// value still on the stack?
		int d_stack = 0, min_d_stack = 0, max_d_stack = 0;
		for (int i=v.first + 1;i<v.last;i++){
			if (cmd[i].inst == Asm::inst_fld)
				d_stack ++;
			else if (cmd[i].inst == Asm::inst_fstp)
				d_stack --;
			min_d_stack = min(min_d_stack, d_stack);
			max_d_stack = max(max_d_stack, d_stack);
		}
		if ((d_stack != 0) || (min_d_stack < 0) || (max_d_stack > 5))
			continue;

		// reuse value on the stack
		so(format("fpu (a)  var=%d first=%d last=%d stack: d=%d min=%d max=%d", vi, v.first, v.last, d_stack, min_d_stack, max_d_stack));
		remove_cmd(v.last);
		remove_cmd(v.first);
		remove_temp_var(vi);
	}

// fstp temp
// mov xxx, temp
	for (int vi=temp_var.num-1;vi>=0;vi--){
		TempVar &v = temp_var[vi];
		if (v.first < 0)
			continue;

		// may only appear two times
		if (v.count > 2)
			continue;

		// stored then moved...?
		if ((cmd[v.first].inst != Asm::inst_fstp) || (cmd[v.last].inst != Asm::inst_mov))
			continue;
		if (temp_in_cmd(v.last, vi) != 2)
			continue;
		// moved into fstore'able?
		int kind = cmd[v.last].p1.kind;
		if ((kind != KindVarLocal) && (kind != KindVarGlobal) && (kind != KindVarTemp) && (kind != KindDerefVarTemp) && (kind != KindDerefRegister))
		    continue;

		// check, if mov target is used in between
		SerialCommandParam target = cmd[v.last].p1;
		if (!ParamUntouchedInInterval(target, v.first + 1 ,v.last - 1))
			continue;
		// ...we are lazy...
		//if (v.last - v.first != 1)
		//	continue;

		// store directly into target
		so(format("fpu (b)  var=%d first=%d last=%d", v, v.first, v.last));
		cmd[v.first].p1 = target;
		move_param(target, v.last, v.first);
		remove_cmd(v.last);
		remove_temp_var(vi);
	}
}

// remove  mov x,...    mov ...,x   (and similar)
//    (does not affect reg channels)
void Serializer::SimplifyMovs()
{
	// TODO: count > 2 .... first == input && all_other == output?  (only if first == mov (!=eax?)... else count == 2)
	// should take care of fpu simplification (b)...

	msg_db_f("SimplifyMovs", 3);
	for (int vi=temp_var.num-1;vi>=0;vi--){
		TempVar &v = temp_var[vi];
		if (v.first < 0)
			continue;
		
		// may only appear two times
		if (v.count > 2)
			continue;

		// both times in a mov command (or fld as second)
		if (cmd[v.first].inst != Asm::inst_mov)
			continue;
		int n = cmd[v.last].inst;
		bool fld = (n == Asm::inst_fld) || (n == Asm::inst_fadd) || (n == Asm::inst_fadd) || (n == Asm::inst_fsub) || (n == Asm::inst_fmul) || (n == Asm::inst_fdiv);
		if ((cmd[v.last].inst != Asm::inst_mov) && (!fld))
			continue;
		
		// used as source/target?   no deref?
		if ((temp_in_cmd(v.first, vi) != 1) || (temp_in_cmd(v.last, vi) != (fld ? 1 : 2)))
			continue;

		// new construction allowed?
		SerialCommandParam target = cmd[v.last].p1;
		SerialCommandParam source = cmd[v.first].p2;
		if (fld){
			if (!param_combi_allowed(cmd[v.last].inst, source, cmd[v.last].p2))
				continue;
		}else{
			if (!param_combi_allowed(cmd[v.last].inst, cmd[v.last].p1, source))
				continue;
		}

		// check, if mov source or target are used in between
		if (!ParamUntouchedInInterval(target, v.first + 1 ,v.last - 1))
			continue;
		if (!ParamUntouchedInInterval(source, v.first + 1 ,v.last - 1))
			continue;
		
		so(format("mov simplification allowed  v=%d first=%d last=%d", vi, v.first, v.last));
		if (fld)
			cmd[v.last].p1 = source;
		else
			cmd[v.last].p2 = source;
		move_param(source, v.first, v.last);
		remove_cmd(v.first);
		remove_temp_var(vi);
	}

	// TODO: should happen automatically...
	//ScanTempVarUsage();
	//cmd_list_out();
}

void Serializer::RemoveUnusedTempVars()
{
	// unused temp vars...
	for (int v=temp_var.num-1;v>=0;v--)
		if (temp_var[v].first < 0){
			remove_temp_var(v);
		}
}

/*inline void test_reg_usage(int c)
{
	// call -> violates all...
	if (cmd[c].inst == Asm::inst_call){
		for (int i=0;i<max_reg;i++)
			RegUsed[i] = true;
		return;
	}
	if ((cmd[c].p1.kind == KindRegister) || (cmd[c].p1.kind == KindDerefRegister))
		set_reg_used((long)cmd[c].p1.p);
	if ((cmd[c].p2.kind == KindRegister) || (cmd[c].p2.kind == KindDerefRegister))
		set_reg_used((long)cmd[c].p2.p);
}*/

void Serializer::MapTempVarToReg(int vi, int reg)
{
	msg_db_f("reg", 4);
	TempVar &v = temp_var[vi];
	so(format("temp=reg:  %d - %d:   tv %d := reg %d", v.first, v.last, vi, reg));
	
	SerialCommandParam p = param_reg(v.type, reg);
	
	// map register
	for (int i=v.first;i<=v.last;i++){
		int r = temp_in_cmd(i, vi);
		if (r & 1){
			p.shift = cmd[i].p1.shift;
			cmd[i].p1 = p;
			if (r & 4)
				cmd[i].p1.kind = KindDerefRegister;
		}
		if (r & 2){
			p.shift = cmd[i].p2.shift;
			cmd[i].p2 = p;
			if (r & 8)
				cmd[i].p2.kind = KindDerefRegister;
		}
	}
	add_reg_channel(reg, v.first, v.last);
}

void Serializer::add_stack_var(Type *type, int first, int last, SerialCommandParam &p)
{
	so("add_stack_var");
	so(type->size);
	// find free stack space for the life span of the variable....
	// don't mind... use old algorithm: ALWAYS grow stack... remove ALL on each command in a block

	int s = mem_align(type->size, 4);
	StackOffset += s;
	int offset = - StackOffset;
	if (StackOffset > StackMaxSize)
		StackMaxSize = StackOffset;

	p.kind = KindVarLocal;
	p.p = (char*)(long)offset;
	p.type = type;
	p.shift = 0;
}

void Serializer::MapTempVarToStack(int vi)
{
	msg_db_f("stack", 4);
	TempVar &v = temp_var[vi];
	so(format("temp=stack: %d   (%d - %d)", vi, v.first, v.last));

	SerialCommandParam p;
	add_stack_var(v.type, v.first, v.last, p);
	
	// map
	for (int i=v.first;i<=v.last;i++){
		int r = temp_in_cmd(i, vi);
		if (r == 0)
			continue;

		if ((r & 3) == 3)
			script->DoErrorInternal("asm error: (MapTempVar) temp var on both sides of command");
		
		SerialCommandParam *p_own;
		if ((r & 1) > 0){
			p_own = &cmd[i].p1;
		}else{
			p_own = &cmd[i].p2;
		}
		bool deref = (r > 3);

		p_own->kind = deref ? KindDerefVarLocal : KindVarLocal;
		p_own->p = p.p;
		
#if 0
		SerialCommandParam *p_own, *p_other;
		if ((r & 1) > 0){
			p_own = &cmd[i].p1;
			p_other = &cmd[i].p2;
		}else{
			p_own = &cmd[i].p2;
			p_other = &cmd[i].p1;
		}
		bool deref = (r > 3);

		// allowed directly?
		if (!deref){
			if (param_is_simple(*p_other)){
				*p_own = p;
				continue;
			}
		}

		// TODO: map literally... solve unallowed combis later...?

		// is our variable used for writing... or reading?
		bool var_read = false;
		bool var_write = false;
		int c = cmd[i].inst;
		bool dummy1, dummy2;
		if ((r & 1) > 0)
			GetInstructionParamFlags(cmd[i].inst, var_read, var_write, dummy1, dummy2);
		else if ((r & 2) > 0)
			GetInstructionParamFlags(cmd[i].inst, dummy1, dummy2, var_read, var_write);

		if ((var_write) && (var_read)){ // rw
			if (deref)
				script->DoErrorInternal("map_stack_var (read, write, deref)");
			*p_own = p_eax;
			add_cmd(Asm::inst_mov, p_eax, p);
			move_last_cmd(i);
			add_cmd(Asm::inst_mov, p, p_eax);
			move_last_cmd(i+2);
			add_reg_channel(RegEax, i, i + 2);
			i += 2;
			
		}else if (var_write){ // write only
			if (deref){
				//script->DoErrorInternal("map_stack_var (write, deref)");
				int shift = p_own->shift;
				*p_own = p_deref_eax;
				add_cmd(Asm::inst_mov, p_eax, p);
				move_last_cmd(i);
				if (shift > 0){
					add_cmd(Asm::inst_add, p_eax, param_const(TypeInt, (void*)shift));
					move_last_cmd(i + 1);
					add_reg_channel(RegEax, i, i + 2);
				}else
					add_reg_channel(RegEax, i, i + 1);
			}else{
				*p_own = p_eax;
				add_cmd(Asm::inst_mov, p, p_eax);
				move_last_cmd(i+1);
				add_reg_channel(RegEax, i, i + 1);
			}
			i ++;
		}else{ // read only
			int shift = p_own->shift;
			*p_own = deref ? p_deref_eax : p_eax;
			add_cmd(Asm::inst_mov, p_eax, p);
			move_last_cmd(i);
			if ((deref) && (shift > 0)){
				add_cmd(Asm::inst_add, p_eax, param_const(TypeInt, (void*)shift));
				move_last_cmd(i + 1);
				add_reg_channel(RegEax, i, i + 2);
			}else
				add_reg_channel(RegEax, i, i + 1);
			i ++;
		}
#endif
	}
}

bool Serializer::is_reg_root_used_in_interval(int reg_root, int first, int last)
{
	//so(string2("used?   %d: %d - %d", reg, first, last));
	for (int i=0;i<reg_channel.num;i++)
		if (reg_channel[i].reg_root == reg_root){
			//so(string2("rc   %d: %d - %d", RegChannel[i].reg, RegChannel[i].first, RegChannel[i].last));
			if ((reg_channel[i].first <= last) && (reg_channel[i].last >= first)){
				so(i);
				return true;
			}
		}
	//so("false");
	return false;
}

void Serializer::MapTempVar(int vi)
{
	msg_db_f("MapTempVar", 4);
	TempVar &v = temp_var[vi];
	int first = v.first;
	int last = v.last;
	if (first < 0)
		return;

	bool reg_allowed = true;
	for (int i=first;i<=last;i++)
		if (temp_in_cmd(i, vi))
			if (!Asm::GetInstructionAllowGenReg(cmd[i].inst)){
				reg_allowed = false;
				break;
			}

	int reg = -1;
	if (reg_allowed){

		// any register not used in this interval?
		for (int i=0;i<max_reg;i++)
			RegRootUsed[i] = false;
		for (int i=0;i<reg_channel.num;i++)
			if ((reg_channel[i].first <= last) && (reg_channel[i].last >= first))
				RegRootUsed[reg_channel[i].reg_root] = true;
		for (int i=0;i<MapRegRoot.num;i++)
			if (!RegRootUsed[MapRegRoot[i]]){
				reg = get_reg(MapRegRoot[i], v.type->size);
				break;
			}
	}

	if (reg >= 0)
		MapTempVarToReg(vi, reg);
	else
		MapTempVarToStack(vi);
}

void Serializer::MapTempVars()
{
	msg_db_f("MapTempVars", 3);

	for (int i=0;i<temp_var.num;i++)
		MapTempVar(i);
	
	//cmd_list_out();
}

inline void try_map_param_to_stack(SerialCommandParam &p, int v, SerialCommandParam &stackvar)
{
	if ((p.kind == KindVarTemp) && ((long)p.p == v)){
		p.kind = KindVarLocal;//stackvar.kind;
		p.p = stackvar.p;
	}else if ((p.kind == KindDerefVarTemp) && ((long)p.p == v)){
		p.kind = KindDerefVarLocal;
		p.p = stackvar.p;
	}
}

void Serializer::MapReferencedTempVars()
{
	msg_db_f("MapReferencedTempVars", 3);
	for (int i=0;i<cmd.num;i++)
		if (cmd[i].inst == Asm::inst_lea)
			if (cmd[i].p2.kind == KindVarTemp){
				temp_var[(long)cmd[i].p2.p].force_stack = true;
			}

	for (int i=temp_var.num-1;i>=0;i--)
		if (temp_var[i].force_stack){
			SerialCommandParam stackvar;
			add_stack_var(temp_var[i].type, temp_var[i].first, temp_var[i].last, stackvar);
			for (int j=0;j<cmd.num;j++){
				try_map_param_to_stack(cmd[j].p1, i, stackvar);
				try_map_param_to_stack(cmd[j].p2, i, stackvar);
			}
			remove_temp_var(i);
		}
}

void Serializer::DisentangleShiftedTempVars()
{
	msg_db_f("DisentangleShiftedTempVars", 3);
	for (int i=0;i<cmd.num;i++){
		if ((cmd[i].p1.kind == KindVarTemp) && (cmd[i].p1.shift > 0)){
			temp_var[(long)cmd[i].p1.p].entangled = max(temp_var[(long)cmd[i].p1.p].entangled, cmd[i].p1.shift);
		}
		if ((cmd[i].p2.kind == KindVarTemp) && (cmd[i].p2.shift > 0)){
			temp_var[(long)cmd[i].p2.p].entangled = max(temp_var[(long)cmd[i].p2.p].entangled, cmd[i].p2.shift);
		}
	}
	for (int i=temp_var.num-1;i>=0;i--)
		if (temp_var[i].entangled > 0){
			int n = temp_var[i].entangled / 4 + 1;
			Type *t = temp_var[i].type;
			so("entangled");
			so(n);
			SerialCommandParam *p = new SerialCommandParam[n];

			// create small temp vars
			for (int j=0;j<n;j++){
				Type *tt = TypeReg32;
				// corresponding to element in a class?
				for (int k=0;k<t->element.num;k++)
					if (t->element[k].offset == j * 4)
						if (t->element[k].type->size == 4)
							tt = t->element[k].type;
				add_temp(tt, p[j]);
			}
			
			for (int j=0;j<cmd.num;j++){
				if ((cmd[j].p1.kind == KindVarTemp) && ((long)cmd[j].p1.p == i))
					cmd[j].p1 = p[cmd[j].p1.shift / 4];
				if ((cmd[j].p2.kind == KindVarTemp) && ((long)cmd[j].p2.p == i))
					cmd[j].p2 = p[cmd[j].p2.shift / 4];
			}
			delete[]p;
			remove_temp_var(i);
		}

	ScanTempVarUsage();
}

inline void _resolve_deref_reg_shift_(Serializer *_s, SerialCommandParam &p, int i)
{
	long s = p.shift;
	p.shift = 0;
	msg_write("_resolve_deref_reg_shift_");
	int reg = reg_resize((long)p.p, 4);
	_s->add_cmd(Asm::inst_add, param_reg(TypeReg32, reg), param_const(TypeInt, (void*)s));
	_s->move_last_cmd(i);
	_s->add_cmd(Asm::inst_sub, param_reg(TypeReg32, reg), param_const(TypeInt, (void*)s));
	_s->move_last_cmd(i + 2);
}

// TODO....
void Serializer::ResolveDerefRegShift()
{
	msg_db_f("ResolveDerefRegShift", 3);
	for (int i=cmd.num-1;i>=0;i--){
		if ((cmd[i].p1.kind == KindDerefRegister) && (cmd[i].p1.shift > 0)){
			_resolve_deref_reg_shift_(this, cmd[i].p1, i);
			continue;
		}
		if ((cmd[i].p2.kind == KindDerefRegister) && (cmd[i].p2.shift > 0)){
			_resolve_deref_reg_shift_(this, cmd[i].p2, i);
			continue;
		}
	}
}

void Serializer::AddFunctionIntro(Function *f)
{
	if (config.instruction_set == Asm::InstructionSetAMD64){
		// return, instance, params
		Array<LocalVariable> param;
		if (f->return_type->UsesReturnByMemory()){
			foreach(LocalVariable &v, f->var)
				if (v.name == "-return-"){
					param.add(v);
					break;
				}
		}
		if (f->_class){
			foreach(LocalVariable &v, f->var)
				if (v.name == "self"){
					param.add(v);
					break;
				}
		}
		for (int i=0;i<f->num_params;i++)
			param.add(f->var[i]);

		// map params...
		Array<LocalVariable> reg_param;
		Array<LocalVariable> stack_param;
		Array<LocalVariable> xmm_param;
		foreach(LocalVariable &p, param){
			if ((p.type == TypeInt) || (p.type == TypeChar) || (p.type == TypeBool) || (p.type->is_pointer)){
				if (reg_param.num < 6){
					reg_param.add(p);
				}else{
					stack_param.add(p);
				}
			}else if (p.type == TypeFloat){
				if (xmm_param.num < 8){
					xmm_param.add(p);
				}else{
					stack_param.add(p);
				}
			}else
				DoError("parameter type currently not supported: " + p.type->name);
		}
	
		// xmm0-7
		foreachib(LocalVariable &p, xmm_param, i){
			int reg = Asm::RegXmm0 + i;
			add_cmd(Asm::inst_movss, param_local(p.type, p._offset), param_reg(p.type, reg));
		}
	
		// rdi, rsi,rdx, rcx, r8, r9 
		int param_regs_root[6] = {7, 6, 2, 1, 8, 9};
		foreachib(LocalVariable &p, reg_param, i){
			int root = param_regs_root[i];
			int reg = get_reg(root, p.type->size);
			if (reg >= 0){
				add_cmd(Asm::inst_mov, param_local(p.type, p._offset), param_reg(p.type, reg));
				add_reg_channel(reg, cmd.num - 1, cmd.num - 1);
			}else{
				// some registers are not 8bit'able
				add_cmd(Asm::inst_mov, p_eax, param_reg(TypeReg32, get_reg(root, 4)));
				add_cmd(Asm::inst_mov, param_local(p.type, p._offset), param_reg(p.type, get_reg(0, p.type->size)));
				add_reg_channel(reg, cmd.num - 2, cmd.num - 2);
				add_reg_channel(Asm::RegEax, cmd.num - 2, cmd.num - 1);
			}
		}
		
		// get parameters from stack
		foreachb(LocalVariable &p, stack_param){
			DoError("func with stack...");
			/*int s = 8;
			add_cmd(Asm::inst_push, p);
			push_size += s;*/
		}
	}

}

void init_serializing()
{
	p_eax = param_reg(TypeReg32, Asm::RegEax);
	p_eax_int = param_reg(TypeInt, Asm::RegEax);
	p_rax = param_reg(TypeReg64, Asm::RegRax);

	p_deref_eax.kind = KindDerefRegister;
	p_deref_eax.p = (char*)Asm::RegEax;
	p_deref_eax.type = TypePointer;
	p_deref_eax.shift = 0;
	
	p_ax = param_reg(TypeReg16, Asm::RegAx);
	p_al = param_reg(TypeReg8, Asm::RegAl);
	p_al_bool = param_reg(TypeBool, Asm::RegAl);
	p_al_char = param_reg(TypeChar, Asm::RegAl);
	p_ah = param_reg(TypeReg8, Asm::RegAh);
	p_st0 = param_reg(TypeFloat, Asm::RegSt0);
	p_st1 = param_reg(TypeFloat, Asm::RegSt1);
}

void Serializer::SerializeFunction(Function *f)
{
	msg_db_f("SerializeFunction", 2);

	init_serializing();

	syntax_tree->CreateAsmMetaInfo();
	syntax_tree->AsmMetaInfo->CurrentOpcodePos = script->OpcodeSize;
	syntax_tree->AsmMetaInfo->PreInsertionLength = script->OpcodeSize;
	syntax_tree->AsmMetaInfo->LineOffset = 0;
	Asm::CurrentMetaInfo = syntax_tree->AsmMetaInfo;

	cur_func = f;
	NumMarkers = 0;
	call_used = false;
	StackOffset = f->_var_size;
	StackMaxSize = f->_var_size;
	TempVarRangesDefined = false;
	
	if (config.allow_registers){
	//	MapRegRoot.add(0); // eax
		MapRegRoot.add(1); // ecx
		MapRegRoot.add(2); // edx
	//	MapRegRoot.add(3); // ebx
	//	MapRegRoot.add(6); // esi
	//	MapRegRoot.add(7); // edi
	}

// serialize

	AddFunctionIntro(f);

	FillInConstructorsFunc();

	// function
	SerializeBlock(f->block, 0);
	if (script->syntax->FlagShow)
		cmd_list_out();
	


	// outro (if last command != return)
	bool need_outro = true;
	if (f->block->command.num > 0)
		if ((f->block->command.back()->kind == KindCompilerFunction) && (f->block->command.back()->link_nr == CommandReturn))
			need_outro = false;
	if (need_outro){
		FillInDestructors(false);
		add_cmd(Asm::inst_leave);
		if ((f->return_type->UsesReturnByMemory()) && (config.instruction_set == Asm::InstructionSetX86))
			add_cmd(Asm::inst_ret, param_const(TypeReg16, (void*)4));
		else
			add_cmd(Asm::inst_ret);
	}


	if (add_later.num > 0){
		msg_write(f->name);
		msg_write(add_later.num);
		for (int i=0;i<add_later.num;i++){
			msg_write(add_later[i].kind);
			msg_write(add_later[i].marker);
			msg_write(add_later[i].index);
			msg_write(add_later[i].level);
		}
		DoError("StuffToAdd");
	}

	//cmd_list_out();
}


void Serializer::FindReferencedTempVars()
{
	msg_db_f("MapRemainingTempVarsToStack", 3);
	for (int i=0;i<cmd.num;i++)
		if (cmd[i].inst == Asm::inst_lea)
			if (cmd[i].p2.kind == KindVarTemp){
				temp_var[(long)cmd[i].p2.p].force_stack = true;
			}
}
void Serializer::TryMapTempVarsRegisters()
{
	msg_db_f("TryMapTempVarsRegisters", 3);
	for (int i=temp_var.num-1;i>=0;i--){
		if (temp_var[i].force_stack)
			continue;
	}
}
void Serializer::MapRemainingTempVarsToStack()
{
	msg_db_f("MapRemainingTempVarsToStack", 3);
	for (int i=temp_var.num-1;i>=0;i--){
		SerialCommandParam stackvar;
		add_stack_var(temp_var[i].type, temp_var[i].first, temp_var[i].last, stackvar);
		for (int j=0;j<cmd.num;j++){
			try_map_param_to_stack(cmd[j].p1, i, stackvar);
			try_map_param_to_stack(cmd[j].p2, i, stackvar);
		}
		remove_temp_var(i);
	}
}

void Serializer::DoMapping()
{
	FindReferencedTempVars();

	TryMapTempVarsRegisters();

	MapRemainingTempVarsToStack();

	ResolveDerefTempAndLocal();

	CorrectUnallowedParamCombis();

	/*MapReferencedTempVars();

	//HandleDerefTemp();

	DisentangleShiftedTempVars();

	ResolveDerefTempAndLocal();

	RemoveUnusedTempVars();

	if (config.allow_simplification){
	SimplifyMovs();

	SimplifyFPUStack();
	}

	MapTempVars();

	ResolveDerefRegShift();

	//ResolveDerefLocal();

	CorrectUnallowedParamCombis();*/


	if (script->syntax->FlagShow)
		cmd_list_out();
}

inline void get_param(int inst, SerialCommandParam &p, int &param_type, int &param_size, void *&param, Asm::InstructionWithParamsList *list, Script *s)
{
	param_size = -1;
	if (p.kind < 0){
		param_type = Asm::PKNone;
		param = NULL;
	}else if (p.kind == KindMarker){
		param_type = Asm::PKLabel;
		param_size = 4;
		param = (void*)(long)list->add_label("_kaba_" + i2s((int)(long)p.p), false);
	}else if (p.kind == KindRegister){
		param_type = Asm::PKRegister;
		param = p.p;
		param_size = p.type->size;
		if (p.shift > 0)
			s->DoErrorInternal("get_param: reg + shift");
	}else if (p.kind == KindDerefRegister){
		param_size = p.type->size;
		param_type = Asm::PKDerefRegister;
		//if ((param_size != 1) && (param_size != 2) && (param_size != 4) && (param_size != 8))
		//	s->DoErrorInternal("get_param: evil deref reg of type " + p.type->name);
		param = p.p;
		if (p.shift > 0){
			if ((long)p.p == Asm::RegEdx){
				param_type = Asm::PKEdxRel;
				param = (void*)(long)p.shift;
			}else
				s->DoErrorInternal("get_param: [reg] + shift");
		}
	}else if (p.kind == KindVarGlobal){
		param_type = Asm::PKDerefConstant;
		param_size = p.type->size;
		if ((param_size != 1) && (param_size != 2) && (param_size != 4) && (param_size != 8))
			s->DoErrorInternal("get_param: evil global of type " + p.type->name);
		param = p.p + p.shift;
	}else if (p.kind == KindVarLocal){
		param_size = p.type->size;
		param_type = Asm::PKLocal;
		if ((param_size != 1) && (param_size != 2) && (param_size != 4) && (param_size != 8))
			param_size = -1; // lea doesn't need size...
			//s->DoErrorInternal("get_param: evil local of type " + p.type->name);
		param = p.p + p.shift;
	}else if (p.kind == KindRefToConst){
		bool imm_allowed = Asm::GetInstructionAllowConst(inst);
		if ((imm_allowed) && (p.type->is_pointer)){
			param_type = Asm::PKConstant;
			param_size = 4;
			param = (char*)(long)*(int*)(p.p + p.shift);
		}else if ((p.type->size <= 4) && (imm_allowed)){
			param_type = Asm::PKConstant;
			param_size = p.type->size;
			param = (char*)(long)*(int*)(p.p + p.shift);
		}else{
			param_type = Asm::PKDerefConstant;
			param_size = 4;
			param = p.p + p.shift;
		}
	}else if (p.kind == KindConstant){
		param_type = Asm::PKConstant;
		param_size = p.type->size;
		param = p.p;
		if (p.shift > 0)
			s->DoErrorInternal("get_param: const + shift");
	}else
		s->DoErrorInternal("get_param: unexpected param..." + Kind2Str(p.kind));
}


void assemble_cmd(Asm::InstructionWithParamsList *list, SerialCommand &c, Script *s)
{
	// translate parameters
	int param1_type, param2_type;
	int param1_size, param2_size;
	void *param1, *param2;
	get_param(c.inst, c.p1, param1_type, param1_size, param1, list, s);
	get_param(c.inst, c.p2, param2_type, param2_size, param2, list, s);

	// assemble instruction
	//list->current_line = c.
	list->add_easy(c.inst, param1_type, param1_size, param1, param2_type, param2_size, param2);
}

void AddAsmBlock(Asm::InstructionWithParamsList *list, Script *s)
{
	msg_db_f("AddAsmBlock", 4);
	//msg_write(".------------------------------- asm");
	SyntaxTree *ps = s->syntax;
	ps->AsmMetaInfo->LineOffset = ps->AsmBlocks[0].line;
	list->AppendFromSource(ps->AsmBlocks[0].block);
	ps->AsmBlocks.erase(0);
}

void Serializer::Assemble(char *Opcode, int &OpcodeSize)
{
	msg_db_f("Serializer.Assemble", 2);

	// intro + allocate stack memory
	StackMaxSize += MaxPushSize;
	StackMaxSize = mem_align(StackMaxSize, config.StackFrameAlign);
	list->add_func_intro(StackMaxSize);

	for (int i=0;i<cmd.num;i++){

		if (cmd[i].inst == inst_marker){
			//msg_write("marker _kaba_" + i2s(cmd[i].p1.kind));
			list->add_label("_kaba_" + i2s((long)cmd[i].p1.p), true);
		}else if (cmd[i].inst == inst_asm){
			AddAsmBlock(list, script);
		}else{

			// push 8 bit -> push 32 bit
			if (cmd[i].inst == Asm::inst_push)
				if (cmd[i].p1.kind == KindRegister)
					cmd[i].p1.p = (char*)(long)reg_resize((long)cmd[i].p1.p, config.PointerSize);

			// FIXME
			// evil hack to allow inconsistent param types (in address shifts)
			if (config.instruction_set == Asm::InstructionSetAMD64){
				if ((cmd[i].inst == Asm::inst_add) || (cmd[i].inst == Asm::inst_mov)){
					if ((cmd[i].p1.kind == KindRegister) && (cmd[i].p2.kind == KindRefToConst)){
						if (cmd[i].p1.type->is_pointer){
#ifdef debug_evil_corrections
							msg_write("----evil resize a");
							msg_write(cmd2str(cmd[i]));
#endif
							cmd[i].p1.type = TypeReg32;
							cmd[i].p1.p = (char*)(long)reg_resize((long)cmd[i].p1.p, 4);
#ifdef debug_evil_corrections
							msg_write(cmd2str(cmd[i]));
#endif
						}
					}
					if ((cmd[i].p1.type->size == 8) && (cmd[i].p2.type->size == 4)){
						if ((cmd[i].p1.kind == KindRegister) && ((cmd[i].p2.kind == KindRegister) || (cmd[i].p2.kind == KindConstant) || (cmd[i].p2.kind == KindRefToConst))){
#ifdef debug_evil_corrections
							msg_write("----evil resize b");
							msg_write(cmd2str(cmd[i]));
#endif
							cmd[i].p1.type = cmd[i].p2.type;
							cmd[i].p1.p = (char*)(long)reg_resize((long)cmd[i].p1.p, cmd[i].p2.type->size);
#ifdef debug_evil_corrections
							msg_write(cmd2str(cmd[i]));
#endif
						}else if (cmd[i].p2.kind == KindRegister){
#ifdef debug_evil_corrections
							msg_write("----evil resize c");
							msg_write(cmd2str(cmd[i]));
#endif
							cmd[i].p2.type = cmd[i].p1.type;
							cmd[i].p2.p = (char*)(long)reg_resize((long)cmd[i].p2.p, cmd[i].p1.type->size);
#ifdef debug_evil_corrections
							msg_write(cmd2str(cmd[i]));
#endif
						}
					}
					if ((cmd[i].p1.type->size < 8) && (cmd[i].p2.type->size == 8)){
						if ((cmd[i].p1.kind == KindRegister) && ((cmd[i].p2.kind == KindRegister) || (cmd[i].p2.kind == KindDerefRegister))){
#ifdef debug_evil_corrections
							msg_write("----evil resize d");
							msg_write(cmd2str(cmd[i]));
#endif
							cmd[i].p1.type = cmd[i].p2.type;
							cmd[i].p1.p = (char*)(long)reg_resize((long)cmd[i].p1.p, cmd[i].p2.type->size);
#ifdef debug_evil_corrections
							msg_write(cmd2str(cmd[i]));
#endif
						}
					}
					/*if (cmd[i].p1.type->size > cmd[i].p2.type->size){
						msg_write("size ok");
						if ((cmd[i].p1.kind == KindRegister) && ((cmd[i].p2.kind == KindRegister) || (cmd[i].p2.kind == KindConstant) || (cmd[i].p2.kind == KindRefToConst))){
							msg_error("----evil resize");
							cmd[i].p1.type = cmd[i].p2.type;
							cmd[i].p1.p = (char*)(long)Asm::RegResize[Asm::RegRoot[(long)cmd[i].p1.p]][cmd[i].p2.type->size];
						}
					}*/
				}
			}

			assemble_cmd(list, cmd[i], script);
		}
	}

	//msg_write(Opcode2Asm(Opcode, OpcodeSize));

	list->Optimize(Opcode, OpcodeSize);
	list->Compile(Opcode, OpcodeSize);
}

void Serializer::DoError(const string &msg)
{
	script->DoErrorInternal(msg);
}

void Serializer::DoErrorLink(const string &msg)
{
	script->DoErrorLink(msg);
}

void do_func_align(char *Opcode, int &OpcodeSize)
{
	int ocs_new = mem_align(OpcodeSize, config.FunctionAlign);
	for (int i=OpcodeSize;i<ocs_new;i++)
		Opcode[i] = 0x90;
	OpcodeSize = ocs_new;
}

Serializer::Serializer(Script *s)
{
	script = s;
	syntax_tree = s->syntax;
	list = new Asm::InstructionWithParamsList(0);
	MaxPushSize = 0;
}

Serializer::~Serializer()
{
	delete(list);
}

void Script::CompileFunction(Function *f, char *Opcode, int &OpcodeSize)
{
	msg_db_f("Compile Function", 2);

	if (syntax->FlagShow)
		msg_write("serializing " + f->name + " -------------------");

	do_func_align(Opcode, OpcodeSize);

	cur_func = f;
	Serializer d = Serializer(this);

	try{
		d.SerializeFunction(f);
		d.DoMapping();
		d.Assemble(Opcode, OpcodeSize);
	}catch(Exception &e){
		throw e;
	}catch(Asm::Exception &e){
		throw Exception(e, this);
	}
}

};
