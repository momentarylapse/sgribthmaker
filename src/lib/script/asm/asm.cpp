#include "../../base/base.h"
#include "../../file/file.h"
#include "asm.h"
#include <stdio.h>

namespace Asm
{


int OCParam;

struct ParserState
{
	bool EndOfLine;
	bool EndOfCode;
	int LineNo;
	int ColumnNo;
	int DefaultSize;
	int ParamSize, AddrSize;
	void reset()
	{
		ParamSize = DefaultSize;
		AddrSize = DefaultSize;
	}
};
static ParserState state;
static int InstructionSet;

const char *code_buffer;
MetaInfo *CurrentMetaInfo = NULL;

Exception::Exception(const string &_message, const string &_expression, int _line, int _column)
{
	if (_expression.num > 0)
		message += "\"" + _expression + "\": ";
	message += _message;
	if (line >= 0)
		message += "\nline " + i2s(_line);

	line = _line;
	column = _column;
}

Exception::~Exception(){}

void Exception::print() const
{
	msg_error(message);
}

void SetError(const string &str)
{
	//msg_error(str + format("\nline %d", LineNo + 1));
	throw Exception(str, "", state.LineNo, state.ColumnNo);
}


bool DebugAsm = false;

static void so(const char *str)
{
	if (DebugAsm)
		printf("%s\n",str);
}

static void so(const string &str)
{
	if (DebugAsm)
		printf("%s\n",str.c_str());
}

static void so(int i)
{
	if (DebugAsm)
		printf("%d\n",i);
}

// penalty:  0 -> max output
#define ASM_DB_LEVEL	10




/*string bufstr;
char *buffer;
inline void resize_buffer(int size)
{
	bufstr.resize(size);
	buffer = &bufstr[0];
}*/

enum{
	Size8 = 1,
	Size16 = 2,
	Size32 = 4,
//	Size48 = 6,
	Size64 = 8,
	SizeVariable = -5,
	Size32or48 = -6,
	SizeUnknown = -7
};

inline int absolute_size(int s)
{
	if (s == Size8)		return 1;
	if (s == Size16)	return 2;
	if (s == Size32)	return 4;
	if (s == Size64)	return 8;
	return 0;
}

struct Register{
	string name;
	int id, group, size;
};
Array<Register> Registers;
Array<Register*> RegisterByID;
int RegRoot[NUM_REGISTERS];

void add_reg(const string &name, int id, int group, int size, int root = -1)
{
	Register r;
	r.name = name;
	r.id = id;
	r.group = group;
	r.size = size;
	Registers.add(r);
	RegRoot[id] = (root < 0) ? id : root;
}

struct InstructionName{
	int inst;
	string name;
	int rw1, rw2; // parameter is read(1), modified(2) or both (3)
};

// rw1/2: 
InstructionName InstructionNames[NUM_INSTRUCTION_NAMES + 1] = {
	{inst_add,		"add",		3, 1},
	{inst_add_b,	"add.b",	3, 1},
	{inst_adc,		"adc",		3, 1},
	{inst_adc_b,	"adc.b",	3, 1},
	{inst_sub,		"sub",		3, 1},
	{inst_sub_b,	"sub.b",	3, 1},
	{inst_sbb,		"sbb",		3, 1},
	{inst_sbb_b,	"sbb.b",	3, 1},
	{inst_inc,		"inc",		3},
	{inst_inc_b,	"inc.b",	3},
	{inst_dec,		"dec",		3},
	{inst_dec_b,	"dec.b",	3},
	{inst_mul,		"mul",		3, 1},
	{inst_mul_b,	"mul.b",	3, 1},
	{inst_imul,		"imul",		3, 1},
	{inst_imul_b,	"imul.b",	3, 1},
	{inst_div,		"div",		3, 1},
	{inst_div_b,	"div.b",	3, 1},
	{inst_idiv,		"idiv",		3, 1},
	{inst_idiv_b,	"idiv.b",	3, 1},
	{inst_mov,		"mov",		2, 1},
	{inst_mov_b,	"mov.b",	2, 1},
	{inst_movzx,	"movzx",	2, 1},
	{inst_movsx,	"movsx",	2, 1},
	{inst_and,		"and",		3, 1},
	{inst_and_b,	"and.b",	3, 1},
	{inst_or,		"or",		3, 1},
	{inst_or_b,		"or.b",		3, 1},
	{inst_xor,		"xor",		3, 1},
	{inst_xor_b,	"xor.b",	3, 1},
	{inst_not,		"not",		3},
	{inst_not_b,	"not.b",	3},
	{inst_neg,		"neg",		3},
	{inst_neg_b,	"neg.b",	3},
	{inst_pop,		"pop",		2},
	{inst_popa,		"popa",		2},
	{inst_push,		"push",		1},
	{inst_pusha,	"pusha",	1},
	
	{inst_jo,		"jo",		1},
	{inst_jo_b,		"jo.b",		1},
	{inst_jno,		"jno",		1},
	{inst_jno_b,	"jno.b",	1},
	{inst_jb,		"jb",		1},
	{inst_jb_b,		"jb.b",		1},
	{inst_jnb,		"jnb",		1},
	{inst_jnb_b,	"jnb.b",	1},
	{inst_jz,		"jz",		1},
	{inst_jz_b,		"jz.b",		1},
	{inst_jnz,		"jnz",		1},
	{inst_jnz_b,	"jnz.b",	1},
	{inst_jbe,		"jbe",		1},
	{inst_jbe_b,	"jbe.b",	1},
	{inst_jnbe,		"jnbe",		1},
	{inst_jnbe_b,	"jnbe.b",	1},
	{inst_js,		"js",		1},
	{inst_js_b,		"js.b",		1},
	{inst_jns,		"jns",		1},
	{inst_jns_b,	"jns.b",	1},
	{inst_jp,		"jp",		1},
	{inst_jp_b,		"jp.b",		1},
	{inst_jnp,		"jnp",		1},
	{inst_jnp_b,	"jnp.b",	1},
	{inst_jl,		"jl",		1},
	{inst_jl_b,		"jl.b",		1},
	{inst_jnl,		"jnl",		1},
	{inst_jnl_b,	"jnl.b",	1},
	{inst_jle,		"jle",		1},
	{inst_jle_b,	"jle.b",	1},
	{inst_jnle,		"jnle",		1},
	{inst_jnle_b,	"jnle.b",	1},
	
	{inst_cmp,		"cmp",		1, 1},
	{inst_cmp_b,	"cmp.b",	1, 1},
	
	{inst_seto_b,	"seto.b",	2},
	{inst_setno_b,	"setno.b",	2},
	{inst_setb_b,	"setb.b",	2},
	{inst_setnb_b,	"setnb.b",	2},
	{inst_setz_b,	"setz.b",	2},
	{inst_setnz_b,	"setnz.b",	2},
	{inst_setbe_b,	"setbe.b",	2},
	{inst_setnbe_b,	"setnbe.b",	2},
	{inst_sets_b,	"sets.b",	2},
	{inst_setns_b,	"setns.b",	2},
	{inst_setp_b,	"setp.b",	2},
	{inst_setnp_b,	"setnp.b",	2},
	{inst_setl_b,	"setl.b",	2},
	{inst_setnl_b,	"setn.bl",	2},
	{inst_setle_b,	"setle.b",	2},
	{inst_setnle_b,	"setnle.b",	2},
	
	{inst_sldt,		"sldt"},
	{inst_str,		"str"},
	{inst_lldt,		"lldt"},
	{inst_ltr,		"ltr"},
	{inst_verr,		"verr"},
	{inst_verw,		"verw"},
	{inst_sgdt,		"sgdt"},
	{inst_sidt,		"sidt"},
	{inst_lgdt,		"lgdt"},
	{inst_lidt,		"lidt"},
	{inst_smsw,		"smsw"},
	{inst_lmsw,		"lmsw"},
	
	{inst_test,		"test",		1, 1},
	{inst_test_b,	"test.b",	1, 1},
	{inst_xchg,		"xchg",		3, 3},
	{inst_xchg_b,	"xchg.b",	3, 3},
	{inst_lea,		"lea", 		2, 1},
	{inst_nop,		"nop"},
	{inst_cbw_cwde,	"cbw/cwde"},
	{inst_cgq_cwd,	"cgq/cwd"},
	{inst_movs_ds_esi_es_edi,	"movs_ds:esi,es:edi"},
	{inst_movs_b_ds_esi_es_edi,	"movs.b_ds:esi,es:edi"},
	{inst_cmps_ds_esi_es_edi,	"cmps_ds:esi,es:edi"},
	{inst_cmps_b_ds_esi_es_edi,	"cmps.b_ds:esi,es:edi"},
	{inst_rol,		"rol",		3, 1},
	{inst_rol_b,	"rol.b",	3, 1},
	{inst_ror,		"ror",		3, 1},
	{inst_ror_b,	"ror.b",	3, 1},
	{inst_rcl,		"rcl",		3, 1},
	{inst_rcl_b,	"rcl.b",	3, 1},
	{inst_rcr,		"rcr",		3, 1},
	{inst_rcr_b,	"rcr.b",	3, 1},
	{inst_shl,		"shl",		3, 1},
	{inst_shl_b,	"shl.b",	3, 1},
	{inst_shr,		"shr",		3, 1},
	{inst_shr_b,	"shr.b",	3, 1},
	{inst_sar,		"sar",		3, 1},
	{inst_sar_b,	"sar.b",	3, 1},
	{inst_ret,		"ret",		1},
	{inst_leave,	"leave",	1},
	{inst_ret_far,	"ret_far",	1},
	{inst_int,		"int",		1},
	{inst_iret,		"iret",		1},
	
	{inst_fadd,		"fadd",		1},
	{inst_fmul,		"fmul",		1},
	{inst_fsub,		"fsub",		1},
	{inst_fdiv,		"fdiv",		1},
	{inst_fld,		"fld",		1},
	{inst_fst,		"fst",		2},
	{inst_fstp,		"fstp",		2},
	{inst_fild,		"fild",		1},
	{inst_faddp,	"faddp",	1},
	{inst_fmulp,	"fmulp",	1},
	{inst_fsubp,	"fsubp",	1},
	{inst_fdivp,	"fdivp",	1},
	{inst_fldcw,	"fldcw",	1},
	{inst_fnstcw,	"fnstcw",	2},
	{inst_fnstsw,	"fnstsw",	2},
	{inst_fistp,	"fistp",	2},
	{inst_fxch,		"fxch",		3, 3},
	{inst_fsin,		"fsin",		3},
	{inst_fcos,		"fcos",		3},
	{inst_fchs,		"fchs",		3},
	{inst_fabs,		"fabs",		3},
	{inst_fucompp,	"fucompp",	1, 1},
	
	{inst_loop,		"loop"},
	{inst_loope,	"loope"},
	{inst_loopne,	"loopne"},
	{inst_in,		"in",		2, 1},
	{inst_in_b,		"in.b",		2, 1},
	{inst_out,		"out",		1, 1},
	{inst_out_b,	"out.b",	1, 1},
	
	{inst_call,		"call",		1},
	{inst_call_far,	"call_far", 1},
	{inst_jmp,		"jmp",		1},
	{inst_jmp_b,	"jmp.b",	1},
	{inst_lock,		"lock"},
	{inst_rep,		"rep"},
	{inst_repne,	"repne"},
	{inst_hlt,		"hlt"},
	{inst_cmc,		"cmc"},
	{inst_clc,		"clc"},
	{inst_stc,		"stc"},
	{inst_cli,		"cli"},
	{inst_sti,		"sti"},
	{inst_cld,		"cld"},
	{inst_std,		"std"},
	
	{-1,			"???"}
};



// groups of registers
enum{
	RegGroupNone,
	RegGroupGeneral,
	RegGroupSegment,
	RegGroupFlags,
	RegGroupControl
};

// parameter types
enum{
	ParamTImmediate,
	ParamTImmediateDouble,
	ParamTRegister,
	ParamTRegisterOrMem, // ...
	ParamTMemory,
	//ParamTSIB,
	ParamTNone,
	ParamTInvalid
};

// short parameter type
enum{
	__Dummy__ = 10000,
	Eb,Ev,Ew,Ed,
	Gb,Gv,Gw,Gd,
	Ib,Iv,Iw,Id,
	Ob,Ov,Ow,Od,
	Rb,Rv,Rw,Rd,Rq,
	Cb,Cv,Cw,Cd,
	Mb,Mv,Mw,Md,
	Jb,Jv,Jw,Jd,
	Sw,Ap,
};

// displacement for registers
enum{
	DispModeNone,	// reg
	DispMode8,		// reg + 8bit
	DispMode16,		// reg + 16bit
	DispMode32,		// reg + 32bit
	DispModeSIB,	// SIB-byte
	DispMode8SIB,	// SIB-byte + 8bit
	DispModeReg2,	// reg + reg2
	DispMode8Reg2,	// reg + reg2 + 8bit
	DispMode16Reg2	// reg + reg2 + 16bit
};

// a real parameter (usable)
struct InstructionParam{
	int type;
	int disp;
	Register *reg, *reg2;
	bool deref;
	int size;
	long long value; // disp or immediate
	bool is_label;
	bool immediate_is_relative;	// for jump
	string str();
};

struct InstructionWithParams{
	int inst;
	InstructionParam p1, p2;
	int line, col;
	int size;
	int addr_size;
	int param_size;
};


InstructionParam _make_param_(int type, long long param);

InstructionWithParamsList::InstructionWithParamsList(int line_no)
{
	current_line = line_no;
	current_col = 0;
}

InstructionWithParamsList::~InstructionWithParamsList()
{}

void InstructionWithParamsList::add_easy(int inst, int param1_type, void *param1, int param2_type, void *param2)
{
	InstructionWithParams i;
	i.inst = inst;
	i.p1 = _make_param_(param1_type, (long)param1);
	i.p2 = _make_param_(param2_type, (long)param2);
	i.line = current_line;
	i.col = current_col;
	add(i);
};

int InstructionWithParamsList::add_label(const string &name, bool declaring)
{
	so("add_label: " + name);
	// label already in use? (used before declared)
	if (declaring){
		foreachi(Label &l, label, i)
			if (l.InstNo < 0)
				if (l.Name == name){
					l.InstNo = num;
					so("----redecl");
					return i;
				}
	}else{
		foreachi(Label &l, label, i)
			if (l.Name == name){
				so("----reuse");
				return i;
			}
	}
	Label l;
	l.Name = name;
	l.InstNo = declaring ? num : -1;
	l.Value = -1;
	label.add(l);
	return label.num - 1;
}

void InstructionWithParamsList::add_func_intro(int stack_alloc_size)
{
	int reg_bp = (InstructionSet == InstructionSetAMD64) ? RegRbp : RegEbp;
	int reg_sp = (InstructionSet == InstructionSetAMD64) ? RegRsp : RegEsp;
	add_easy(inst_push, PKRegister, (void*)reg_bp);
	add_easy(inst_mov, PKRegister, (void*)reg_bp, PKRegister, (void*)reg_sp);
	if (stack_alloc_size > 127){
		add_easy(inst_sub, PKRegister, (void*)reg_sp, PKConstant32, (void*)stack_alloc_size);
	}else if (stack_alloc_size > 0){
		add_easy(inst_sub_b, PKRegister, (void*)reg_sp, PKConstant8, (void*)stack_alloc_size);
	}
}

void InstructionWithParamsList::add_func_return(int return_size)
{
	add_easy(inst_leave);
	if (return_size > 4)
		add_easy(inst_ret, PKConstant16, (void*)4);
	else
		add_easy(inst_ret);
}

// which part of the modr/m byte is used
enum{
	MRMNone,
	MRMReg,
	MRMModRM
};

string SizeOut(int size)
{
	if (size == Size8)		return "8";
	if (size == Size16)		return "16";
	if (size == Size32)		return "32";
	if (size == SizeVariable)	return "16/32/64";
	if (size == Size32or48)	return "32/48";
	if (size == Size64)		return "64";
	return "???";
}

// parameter definition (filter for real parameters)
struct InstructionParamFuzzy{
	bool used;
	bool allow_memory_address;	// [0x12.34...]
	bool allow_memory_indirect;	// [eax]    [eax + ...]
	bool allow_immediate;		// 0x12.34...
	bool allow_register;		// eax
	int _type_;					// approximate type.... (UnFuzzy without mod/rm)
	Register *reg;				// if != NULL  -> force a single register
	int reg_group;
	int mrm_mode;				// which part of the modr/m byte is used?
	int size;
	bool immediate_is_relative;	// for jump


	void print() const
	{
		string t;
		if (used){
			if (allow_register)
				t += "	Reg";
			if (allow_immediate)
				t += "	Im";
			if (allow_memory_address)
				t += "	[Mem]";
			if (allow_memory_indirect)
				t += "	[Mem + ind]";

			if (reg)
				t += "  " + reg->name;
			if (size != SizeUnknown)
				t += "  " + SizeOut(size);
			if (mrm_mode == MRMReg)
				t += "   /r";
			else if (mrm_mode == MRMModRM)
				t += "   /m";
		}else{
			t += "	None";
		}
		printf("%s\n", t.c_str());
	}
};

// an instruction/opcode the cpu offers
struct sInstruction{
	int inst;
	int code, code_size, cap;
	bool has_modrm;
	InstructionParamFuzzy param1, param2;
	string name;

	void print() const
	{
		printf("inst: %s   %.4x (%d) %d  %s\n", name.c_str(), code, code_size, cap, has_modrm ? "modr/m" : "");
		param1.print();
		param2.print();
	}
};

Array<sInstruction> Instruction;

// expands the short instruction parameters
//   returns true if mod/rm byte needed
bool _get_inst_param_(int param, InstructionParamFuzzy &ip)
{
	ip.reg = NULL;
	ip.reg_group = RegGroupNone;
	ip.mrm_mode = MRMNone;
	ip.reg_group = -1;
	ip._type_ = ParamTInvalid;
	ip.allow_register = false;
	ip.allow_immediate = false;
	ip.allow_memory_address = false;
	ip.allow_memory_indirect = false;
	ip.immediate_is_relative = false;
	if (param < 0){	ip.used = false;	ip._type_ = ParamTNone;	return false;	}
	ip.used = true;

	// is it a register?
	for (int i=0;i<Registers.num;i++)
		if (Registers[i].id == param){
			ip._type_ = ParamTRegister;
			ip.reg = &Registers[i];
			ip.allow_register = true;
			ip.reg_group = Registers[i].group;
			ip.size = Registers[i].size;
			return false;
		}
	// general reg / mem
	if ((param == Eb) || (param == Ev) || (param == Ew) || (param == Ed)){
		ip._type_ = ParamTInvalid;//ParamTRegisterOrMem;
		ip.allow_register = true;
		ip.allow_memory_address = true;
		ip.allow_memory_indirect = true;
		ip.reg_group = RegGroupGeneral;
		ip.mrm_mode = MRMModRM;
		if (param == Eb)	ip.size = Size8;
		if (param == Ev)	ip.size = SizeVariable;
		if (param == Ew)	ip.size = Size16;
		if (param == Ed)	ip.size = Size32;
		return true;
	}
	// general reg (reg)
	if ((param == Gb) || (param == Gv) || (param == Gw) || (param == Gd)){
		ip._type_ = ParamTRegister;
		ip.allow_register = true;
		ip.reg_group = RegGroupGeneral;
		ip.mrm_mode = MRMReg;
		if (param == Gb)	ip.size = Size8;
		if (param == Gv)	ip.size = SizeVariable;
		if (param == Gw)	ip.size = Size16;
		if (param == Gd)	ip.size = Size32;
		return true;
	}
	// general reg (mod)
	if ((param == Rb) || (param == Rv) || (param == Rw) || (param == Rd)){
		ip._type_ = ParamTRegister;
		ip.allow_register = true;
		ip.reg_group = RegGroupGeneral;
		ip.mrm_mode = MRMModRM;
		if (param == Rb)	ip.size = Size8;
		if (param == Rv)	ip.size = SizeVariable;
		if (param == Rw)	ip.size = Size16;
		if (param == Rd)	ip.size = Size32;
		if (param == Rq)	ip.size = Size64;
		return true;
	}
	// immediate
	if ((param == Ib) || (param == Iv) || (param == Iw) || (param == Id) || (param == Ap)){
		ip._type_ = ParamTImmediate;
		ip.allow_immediate = true;
		if (param == Ib)	ip.size = Size8;
		if (param == Iv)	ip.size = SizeVariable;
		if (param == Iw)	ip.size = Size16;
		if (param == Id)	ip.size = Size32;
		if (param == Ap){	ip.size = Size32or48;	ip._type_ = ParamTImmediateDouble;	} // TODO allowed??? (instead of ParamTImmediate)
		return false;
	}
	// immediate (relative)
	if ((param == Jb) || (param == Jv) || (param == Jw) || (param == Jd)){
		ip._type_ = ParamTImmediate;
		ip.allow_immediate = true;
		ip.immediate_is_relative = true;
		if (param == Jb)	ip.size = Size8;
		if (param == Jv)	ip.size = SizeVariable;
		if (param == Jw)	ip.size = Size16;
		if (param == Jd)	ip.size = Size32;
		return false;
	}
	// mem
	if ((param == Ob) || (param == Ov) || (param == Ow) || (param == Od)){
		ip._type_ = ParamTMemory;
		ip.allow_memory_address = true;
		if (param == Ob)	ip.size = Size8;
		if (param == Ov)	ip.size = SizeVariable;
		if (param == Ow)	ip.size = Size16;
		if (param == Od)	ip.size = Size32;
		return false;
	}
	// mem
	if ((param == Mb) || (param == Mv) || (param == Mw) || (param == Md)){
		ip._type_ = ParamTInvalid; // ...
		ip.allow_memory_address = true;
		ip.allow_memory_indirect = true;
		ip.reg_group = RegGroupGeneral;
		ip.mrm_mode = MRMModRM;
		if (param == Mb)	ip.size = Size8;
		if (param == Mv)	ip.size = SizeVariable;
		if (param == Mw)	ip.size = Size16;
		if (param == Md)	ip.size = Size32;
		return true;
	}
	// control reg
	if ((param == Cb) || (param == Cv) || (param == Cw) || (param == Cd)){
		ip._type_ = ParamTRegister;
		ip.allow_register = true;
		ip.reg_group = RegGroupControl;
		ip.mrm_mode = MRMReg;
		if (param == Cb)	ip.size = Size8;
		if (param == Cv)	ip.size = SizeVariable;
		if (param == Cw)	ip.size = Size16;
		if (param == Cd)	ip.size = Size32;
		return true;
	}
	// segment reg
	if (param == Sw){
		ip._type_ = ParamTRegister;
		ip.allow_register = true;
		ip.reg_group = RegGroupSegment;
		ip.mrm_mode = MRMReg;
		ip.size = Size16;
		return true;
	}
	msg_error("asm: unknown instparam (call Michi!)");
	msg_write(param);
	exit(0);
	return false;
	
}

void add_inst(int inst, int code, int code_size, int cap, int param1, int param2)
{
	sInstruction i;
	memset(&i.param1, 0, sizeof(i.param1));
	memset(&i.param2, 0, sizeof(i.param2));
	i.inst = inst;
	i.code = code;
	i.code_size = code_size;
	i.cap = cap;
	bool m1 = _get_inst_param_(param1, i.param1);
	bool m2 = _get_inst_param_(param2, i.param2);
	i.has_modrm  = m1 || m2 || (cap >= 0);
	
	i.name = InstructionNames[NUM_INSTRUCTION_NAMES].name;
	for (int j=0;j<NUM_INSTRUCTION_NAMES;j++)
		if (inst == InstructionNames[j].inst)
			i.name = InstructionNames[j].name;
	Instruction.add(i);
}

string GetInstructionName(int inst)
{
	for (int i=0;i<Asm::NUM_INSTRUCTION_NAMES;i++)
		if (inst == InstructionNames[i].inst)
			return Asm::InstructionNames[i].name;
	return "???";
}

void GetInstructionParamFlags(int inst, bool &p1_read, bool &p1_write, bool &p2_read, bool &p2_write)
{
	for (int i=0;i<NUM_INSTRUCTION_NAMES;i++)
		if (InstructionNames[i].inst == inst){
			p1_read = ((InstructionNames[i].rw1 & 1) > 0);
			p1_write = ((InstructionNames[i].rw1 & 2) > 0);
			p2_read = ((InstructionNames[i].rw2 & 1) > 0);
			p2_write = ((InstructionNames[i].rw2 & 2) > 0);
		}
}

bool GetInstructionAllowConst(int inst)
{
	if ((inst == inst_div) || (inst == inst_idiv))
		return false;
	for (int i=0;i<NUM_INSTRUCTION_NAMES;i++)
		if (InstructionNames[i].inst == inst)
			return (InstructionNames[i].name[0] != 'f');
	return true;
}

bool GetInstructionAllowGenReg(int inst)
{
	if (inst == inst_lea)
		return false;
	for (int i=0;i<NUM_INSTRUCTION_NAMES;i++)
		if (InstructionNames[i].inst == inst)
			return (InstructionNames[i].name[0] != 'f');
	return true;
}

/*
#define NumInstructionsX86		319
sInstruction InstructionX86[NumInstructionsX86]={
};

int NumInstructions=0;
sInstruction *Instruction=NULL;

void SetInstructionSet(int set)
{
	msg_db_f("SetInstructionSet", 1+ASM_DB_LEVEL);
	if (set==InstructionSetDefault){
		if (sizeof(long)==8)
			set=InstructionSetAMD64;
		else
			set=InstructionSetX86;
	}
	
	set=InstructionSetX86;
	
	if (set==InstructionSetX86){
		NumInstructions=NumInstructionsX86;
		Instruction=InstructionX86;
		so("--------------------------------x86");
	}
	
	// build name table
	for (int i=0;i<NumInstructions;i++){
		Instruction[i].name==InstructionName[NUM_INSTRUCTION_NAMES].name;
		for (int j=0;j<NUM_INSTRUCTION_NAMES;j++)
			if (Instruction[i].inst==InstructionName[j].inst)
				Instruction[i].name=InstructionName[j].name;
	}
}*/



void Init(int set)
{
	if (set == InstructionSetDefault){
		if (sizeof(void*) == 8)
			set = InstructionSetAMD64;
		else if (sizeof(void*) == 4)
			set = InstructionSetX86;
		else{
			msg_error("Asm: unknown instruction set");
			set = InstructionSetX86;
		}
	}
	InstructionSet = set;


	Registers.clear();
	add_reg("rax",	RegRax,	RegGroupGeneral,	Size64,	RegEax);
	add_reg("eax",	RegEax,	RegGroupGeneral,	Size32,	RegEax);
	add_reg("ax",	RegAx,	RegGroupGeneral,	Size16,	RegEax);
	add_reg("ah",	RegAh,	RegGroupGeneral,	Size8,	RegEax);
	add_reg("al",	RegAl,	RegGroupGeneral,	Size8,	RegEax);
	add_reg("rcx",	RegRcx,	RegGroupGeneral,	Size64,	RegEcx);
	add_reg("ecx",	RegEcx,	RegGroupGeneral,	Size32,	RegEcx);
	add_reg("cx",	RegCx,	RegGroupGeneral,	Size16,	RegEcx);
	add_reg("ch",	RegCh,	RegGroupGeneral,	Size8,	RegEcx);
	add_reg("cl",	RegCl,	RegGroupGeneral,	Size8,	RegEcx);
	add_reg("rdx",	RegRdx,	RegGroupGeneral,	Size64,	RegEdx);
	add_reg("edx",	RegEdx,	RegGroupGeneral,	Size32,	RegEdx);
	add_reg("dx",	RegDx,	RegGroupGeneral,	Size16,	RegEdx);
	add_reg("dh",	RegDh,	RegGroupGeneral,	Size8,	RegEdx);
	add_reg("dl",	RegDl,	RegGroupGeneral,	Size8,	RegEdx);
	add_reg("rbx",	RegRbx,	RegGroupGeneral,	Size64,	RegEbx);
	add_reg("ebx",	RegEbx,	RegGroupGeneral,	Size32,	RegEbx);
	add_reg("bx",	RegBx,	RegGroupGeneral,	Size16,	RegEbx);
	add_reg("bh",	RegBh,	RegGroupGeneral,	Size8,	RegEbx);
	add_reg("bl",	RegBl,	RegGroupGeneral,	Size8,	RegEbx);

	add_reg("rsp",	RegRsp,	RegGroupGeneral,	Size64);
	add_reg("esp",	RegEsp,	RegGroupGeneral,	Size32);
	add_reg("sp",	RegSp,	RegGroupGeneral,	Size16);
	add_reg("rsi",	RegRsi,	RegGroupGeneral,	Size64);
	add_reg("esi",	RegEsi,	RegGroupGeneral,	Size32);
	add_reg("si",	RegSi,	RegGroupGeneral,	Size16);
	add_reg("rdi",	RegRdi,	RegGroupGeneral,	Size64);
	add_reg("edi",	RegEdi,	RegGroupGeneral,	Size32);
	add_reg("di",	RegDi,	RegGroupGeneral,	Size16);
	add_reg("rbp",	RegRbp,	RegGroupGeneral,	Size64);
	add_reg("ebp",	RegEbp,	RegGroupGeneral,	Size32);
	add_reg("bp",	RegBp,	RegGroupGeneral,	Size16);

	add_reg("cs",	RegCs,	RegGroupSegment,	Size16);
	add_reg("ss",	RegSs,	RegGroupSegment,	Size16);
	add_reg("ds",	RegDs,	RegGroupSegment,	Size16);
	add_reg("es",	RegEs,	RegGroupSegment,	Size16);
	add_reg("fs",	RegFs,	RegGroupSegment,	Size16);
	add_reg("gs",	RegGs,	RegGroupSegment,	Size16);

	add_reg("cr0",	RegCr0,	RegGroupControl,	Size32);
	add_reg("cr1",	RegCr1,	RegGroupControl,	Size32);
	add_reg("cr2",	RegCr2,	RegGroupControl,	Size32);
	add_reg("cr3",	RegCr3,	RegGroupControl,	Size32);

	add_reg("st0",	RegSt0,	-1,	Size32); // ??? 32
	add_reg("st1",	RegSt1,	-1,	Size32);
	add_reg("st2",	RegSt2,	-1,	Size32);
	add_reg("st3",	RegSt3,	-1,	Size32);
	add_reg("st4",	RegSt4,	-1,	Size32);
	add_reg("st5",	RegSt5,	-1,	Size32);
	add_reg("st6",	RegSt6,	-1,	Size32);
	add_reg("st7",	RegSt7,	-1,	Size32);

	// create easy to access array
	RegisterByID.clear();
	for (int i=0;i<Registers.num;i++){
		if (RegisterByID.num <= Registers[i].id)
			RegisterByID.resize(Registers[i].id + 1);
		RegisterByID[Registers[i].id] = &Registers[i];
	}

	Instruction.clear();
	add_inst(inst_add_b		,0x00	,1	,-1	,Eb	,Gb	);
	add_inst(inst_add		,0x01	,1	,-1	,Ev	,Gv	);
	add_inst(inst_add_b		,0x02	,1	,-1	,Gb	,Eb	);
	add_inst(inst_add		,0x03	,1	,-1	,Gv	,Ev	);
	add_inst(inst_add_b		,0x04	,1	,-1	,RegAl	,Ib	);
	add_inst(inst_add		,0x05	,1	,-1	,RegEax,Iv	);
	add_inst(inst_push		,0x06	,1	,-1	,RegEs	,-1	);
	add_inst(inst_pop		,0x07	,1	,-1	,RegEs	,-1	);
	add_inst(inst_or_b		,0x08	,1	,-1	,Eb	,Gb	);
	add_inst(inst_or		,0x09	,1	,-1	,Ev	,Gv	);
	add_inst(inst_or_b		,0x0a	,1	,-1	,Gb	,Eb	);
	add_inst(inst_or		,0x0b	,1	,-1	,Gv	,Ev	);
	add_inst(inst_or_b		,0x0c	,1	,-1	,RegAl	,Ib	);
	add_inst(inst_or		,0x0d	,1	,-1	,RegEax,Iv	);
	add_inst(inst_push		,0x0e	,1	,-1	,RegCs	,-1	);
	add_inst(inst_sldt		,0x000f	,2	,0	,Ew	,-1	);
	add_inst(inst_str		,0x000f	,2	,1	,Ew	,-1	);
	add_inst(inst_lldt		,0x000f	,2	,2	,Ew	,-1	);
	add_inst(inst_ltr		,0x000f	,2	,3	,Ew	,-1	);
	add_inst(inst_verr		,0x000f	,2	,4	,Ew	,-1	);
	add_inst(inst_verw		,0x000f	,2	,5	,Ew	,-1	);
	add_inst(inst_sgdt		,0x010f	,2	,0	,Ev	,-1	);
	add_inst(inst_sidt		,0x010f	,2	,1	,Ev	,-1	);
	add_inst(inst_lgdt		,0x010f	,2	,2	,Ev	,-1	);
	add_inst(inst_lidt		,0x010f	,2	,3	,Ev	,-1	);
	add_inst(inst_smsw		,0x010f	,2	,4	,Ew	,-1	);
	add_inst(inst_lmsw		,0x010f	,2	,6	,Ew	,-1	);
	add_inst(inst_mov		,0x200f	,2	,-1	,Rd	,Cd	); // Fehler im Algorhytmus!!!!  (wirklich ???) -> Fehler in Tabelle?!?
	add_inst(inst_mov		,0x220f	,2	,-1	,Cd	,Rd	);
	add_inst(inst_jo		,0x800f	,2	,-1	,Iv	,-1	);
	add_inst(inst_jno		,0x810f	,2	,-1	,Iv	,-1	);
	add_inst(inst_jb		,0x820f	,2	,-1	,Iv	,-1	);
	add_inst(inst_jnb		,0x830f	,2	,-1	,Iv	,-1	);
	add_inst(inst_jz		,0x840f	,2	,-1	,Iv	,-1	);
	add_inst(inst_jnz		,0x850f	,2	,-1	,Iv	,-1	);
	add_inst(inst_jbe		,0x860f	,2	,-1	,Iv	,-1	);
	add_inst(inst_jnbe		,0x870f	,2	,-1	,Iv	,-1	);
	add_inst(inst_js		,0x880f	,2	,-1	,Iv	,-1	);
	add_inst(inst_jns		,0x890f	,2	,-1	,Iv	,-1	);
	add_inst(inst_jp		,0x8a0f	,2	,-1	,Iv	,-1	);
	add_inst(inst_jnp		,0x8b0f	,2	,-1	,Iv	,-1	);
	add_inst(inst_jl		,0x8c0f	,2	,-1	,Iv	,-1	);
	add_inst(inst_jnl		,0x8d0f	,2	,-1	,Iv	,-1	);
	add_inst(inst_jle		,0x8e0f	,2	,-1	,Iv	,-1	);
	add_inst(inst_jnle		,0x8f0f	,2	,-1	,Iv	,-1	);
	add_inst(inst_seto_b	,0x900f	,2	,-1	,Eb	,-1	);
	add_inst(inst_setno_b	,0x910f	,2	,-1	,Eb	,-1	);
	add_inst(inst_setb_b	,0x920f	,2	,-1	,Eb	,-1	);
	add_inst(inst_setnb_b	,0x930f	,2	,-1	,Eb	,-1	);
	add_inst(inst_setz_b	,0x940f	,2	,-1	,Eb	,-1	);
	add_inst(inst_setnz_b	,0x950f	,2	,-1	,Eb	,-1	);
	add_inst(inst_setbe_b	,0x960f	,2	,-1	,Eb	,-1	);
	add_inst(inst_setnbe_b	,0x970f	,2	,-1	,Eb	,-1	);
	add_inst(inst_sets_b	,0x980f	,2	,-1	,Eb	,-1	); // error in table... "Ev"
	add_inst(inst_setns_b	,0x990f	,2	,-1	,Eb	,-1	);
	add_inst(inst_setp_b	,0x9a0f	,2	,-1	,Eb	,-1	);
	add_inst(inst_setnp_b	,0x9b0f	,2	,-1	,Eb	,-1	);
	add_inst(inst_setl_b	,0x9c0f	,2	,-1	,Eb	,-1	);
	add_inst(inst_setnl_b	,0x9d0f	,2	,-1	,Eb	,-1	);
	add_inst(inst_setle_b	,0x9e0f	,2	,-1	,Eb	,-1	);
	add_inst(inst_setnle_b	,0x9f0f	,2	,-1	,Eb	,-1	);
	add_inst(inst_imul		,0xaf0f	,2	,-1	,Gv	,Ev	);
	add_inst(inst_movzx		,0xb60f	,2	,-1	,Gv	,Eb	);
	add_inst(inst_movzx		,0xb70f	,2	,-1	,Gv	,Ew	);
	add_inst(inst_movsx		,0xbe0f	,2	,-1	,Gv	,Eb	);
	add_inst(inst_movsx		,0xbf0f	,2	,-1	,Gv	,Ew	);
	add_inst(inst_adc_b		,0x10	,1	,-1	,Eb	,Gb	);
	add_inst(inst_adc		,0x11	,1	,-1	,Ev	,Gv	);
	add_inst(inst_adc_b		,0x12	,1	,-1	,Gb	,Eb	);
	add_inst(inst_adc		,0x13	,1	,-1	,Gv	,Ev	);
	add_inst(inst_adc_b		,0x14	,1	,-1	,RegAl	,Ib	);
	add_inst(inst_adc		,0x15	,1	,-1	,RegEax,Iv	);
	add_inst(inst_push		,0x16	,1	,-1	,RegSs	,-1	);
	add_inst(inst_pop		,0x17	,1	,-1	,RegSs	,-1	);
	add_inst(inst_sbb_b		,0x18	,1	,-1	,Eb	,Gb	);
	add_inst(inst_sbb		,0x19	,1	,-1	,Ev	,Gv	);
	add_inst(inst_sbb_b		,0x1a	,1	,-1	,Gb	,Eb	);
	add_inst(inst_sbb		,0x1b	,1	,-1	,Gv	,Ev	);
	add_inst(inst_sbb_b		,0x1c	,1	,-1	,RegAl	,Ib	);
	add_inst(inst_sbb		,0x1d	,1	,-1	,RegEax	,Iv	);
	add_inst(inst_push		,0x1e	,1	,-1	,RegDs	,-1	);
	add_inst(inst_pop		,0x1f	,1	,-1	,RegDs	,-1	);
	add_inst(inst_and_b		,0x20	,1	,-1	,Eb	,Gb	);
	add_inst(inst_and		,0x21	,1	,-1	,Ev	,Gv	);
	add_inst(inst_and_b		,0x22	,1	,-1	,Gb	,Eb	);
	add_inst(inst_and		,0x23	,1	,-1	,Gv	,Ev	);
	add_inst(inst_and_b		,0x24	,1	,-1	,RegAl	,Ib	);
	add_inst(inst_and		,0x25	,1	,-1	,RegEax	,Iv	);
	add_inst(inst_sub_b		,0x28	,1	,-1	,Eb	,Gb	);
	add_inst(inst_sub		,0x29	,1	,-1	,Ev	,Gv	);
	add_inst(inst_sub_b		,0x2a	,1	,-1	,Gb	,Eb	);
	add_inst(inst_sub		,0x2b	,1	,-1	,Gv	,Ev	);
	add_inst(inst_sub_b		,0x2c	,1	,-1	,RegAl	,Ib	);
	add_inst(inst_sub		,0x2d	,1	,-1	,RegEax	,Iv	);
	add_inst(inst_xor_b		,0x30	,1	,-1	,Eb	,Gb	);
	add_inst(inst_xor		,0x31	,1	,-1	,Ev	,Gv	);
	add_inst(inst_xor_b		,0x32	,1	,-1	,Gb	,Eb	);
	add_inst(inst_xor		,0x33	,1	,-1	,Gv	,Ev	);
	add_inst(inst_xor_b		,0x34	,1	,-1	,RegAl	,Ib	);
	add_inst(inst_xor		,0x35	,1	,-1	,RegEax	,Iv	);
	add_inst(inst_cmp_b		,0x38	,1	,-1	,Eb	,Gb	);
	add_inst(inst_cmp		,0x39	,1	,-1	,Ev	,Gv	);
	add_inst(inst_cmp_b		,0x3a	,1	,-1	,Gb	,Eb	);
	add_inst(inst_cmp		,0x3b	,1	,-1	,Gv	,Ev	);
	add_inst(inst_cmp_b		,0x3c	,1	,-1	,RegAl	,Ib	);
	add_inst(inst_cmp		,0x3d	,1	,-1	,RegEax	,Iv	);
	if (set == InstructionSetX86){
		add_inst(inst_inc		,0x40	,1	,-1	,RegEax	,-1	);
		add_inst(inst_inc		,0x41	,1	,-1	,RegEcx	,-1	);
		add_inst(inst_inc		,0x42	,1	,-1	,RegEdx	,-1	);
		add_inst(inst_inc		,0x43	,1	,-1	,RegEbx	,-1	);
		add_inst(inst_inc		,0x44	,1	,-1	,RegEsp	,-1	);
		add_inst(inst_inc		,0x45	,1	,-1	,RegEbp	,-1	);
		add_inst(inst_inc		,0x46	,1	,-1	,RegEsi	,-1	);
		add_inst(inst_inc		,0x47	,1	,-1	,RegEdi	,-1	);
		add_inst(inst_dec		,0x48	,1	,-1	,RegEax	,-1	);
		add_inst(inst_dec		,0x49	,1	,-1	,RegEcx	,-1	);
		add_inst(inst_dec		,0x4a	,1	,-1	,RegEdx	,-1	);
		add_inst(inst_dec		,0x4b	,1	,-1	,RegEbx	,-1	);
		add_inst(inst_dec		,0x4c	,1	,-1	,RegEsp	,-1	);
		add_inst(inst_dec		,0x4d	,1	,-1	,RegEbp	,-1	);
		add_inst(inst_dec		,0x4e	,1	,-1	,RegEsi	,-1	);
		add_inst(inst_dec		,0x4f	,1	,-1	,RegEdi	,-1	);
	}
	if (set == InstructionSetX86){
		add_inst(inst_push		,0x50	,1	,-1	,RegEax	,-1	);
		add_inst(inst_push		,0x51	,1	,-1	,RegEcx	,-1	);
		add_inst(inst_push		,0x52	,1	,-1	,RegEdx	,-1	);
		add_inst(inst_push		,0x53	,1	,-1	,RegEbx	,-1	);
		add_inst(inst_push		,0x54	,1	,-1	,RegEsp	,-1	);
		add_inst(inst_push		,0x55	,1	,-1	,RegEbp	,-1	);
		add_inst(inst_push		,0x56	,1	,-1	,RegEsi	,-1	);
		add_inst(inst_push		,0x57	,1	,-1	,RegEdi	,-1	);
		add_inst(inst_pop		,0x58	,1	,-1	,RegEax	,-1	);
		add_inst(inst_pop		,0x59	,1	,-1	,RegEcx	,-1	);
		add_inst(inst_pop		,0x5a	,1	,-1	,RegEdx	,-1	);
		add_inst(inst_pop		,0x5b	,1	,-1	,RegEbx	,-1	);
		add_inst(inst_pop		,0x5c	,1	,-1	,RegEsp	,-1	);
		add_inst(inst_pop		,0x5d	,1	,-1	,RegEbp	,-1	);
		add_inst(inst_pop		,0x5e	,1	,-1	,RegEsi	,-1	);
		add_inst(inst_pop		,0x5f	,1	,-1	,RegEdi	,-1	);
	}else if (set == InstructionSetAMD64){
		add_inst(inst_push		,0x50	,1	,-1	,RegRax	,-1	);
		add_inst(inst_push		,0x51	,1	,-1	,RegRcx	,-1	);
		add_inst(inst_push		,0x52	,1	,-1	,RegRdx	,-1	);
		add_inst(inst_push		,0x53	,1	,-1	,RegRbx	,-1	);
		add_inst(inst_push		,0x54	,1	,-1	,RegRsp	,-1	);
		add_inst(inst_push		,0x55	,1	,-1	,RegRbp	,-1	);
		add_inst(inst_push		,0x56	,1	,-1	,RegRsi	,-1	);
		add_inst(inst_push		,0x57	,1	,-1	,RegRdi	,-1	);
		add_inst(inst_pop		,0x58	,1	,-1	,RegRax	,-1	);
		add_inst(inst_pop		,0x59	,1	,-1	,RegRcx	,-1	);
		add_inst(inst_pop		,0x5a	,1	,-1	,RegRdx	,-1	);
		add_inst(inst_pop		,0x5b	,1	,-1	,RegRbx	,-1	);
		add_inst(inst_pop		,0x5c	,1	,-1	,RegRsp	,-1	);
		add_inst(inst_pop		,0x5d	,1	,-1	,RegRbp	,-1	);
		add_inst(inst_pop		,0x5e	,1	,-1	,RegRsi	,-1	);
		add_inst(inst_pop		,0x5f	,1	,-1	,RegRdi	,-1	);
	}
	add_inst(inst_pusha		,0x60	,1	,-1	,-1	,-1	);
	add_inst(inst_popa		,0x61	,1	,-1	,-1	,-1	);
	add_inst(inst_push		,0x68	,1	,-1	,Iv	,-1	);
	add_inst(inst_imul		,0x69	,1	,-1	,Ev	,Iv	);
	add_inst(inst_push		,0x6a	,1	,-1	,Ib	,-1	);
	add_inst(inst_jo_b		,0x70	,1	,-1	,Jb	,-1	);
	add_inst(inst_jno_b		,0x71	,1	,-1	,Jb	,-1	);
	add_inst(inst_jb_b		,0x72	,1	,-1	,Jb	,-1	);
	add_inst(inst_jnb_b		,0x73	,1	,-1	,Jb	,-1	);
	add_inst(inst_jz_b		,0x74	,1	,-1	,Jb	,-1	);
	add_inst(inst_jnz_b		,0x75	,1	,-1	,Jb	,-1	);
	add_inst(inst_jbe_b		,0x76	,1	,-1	,Jb	,-1	);
	add_inst(inst_jnbe_b	,0x77	,1	,-1	,Jb	,-1	);
	add_inst(inst_js_b		,0x78	,1	,-1	,Jb	,-1	);
	add_inst(inst_jns_b		,0x79	,1	,-1	,Jb	,-1	);
	add_inst(inst_jp_b		,0x7a	,1	,-1	,Jb	,-1	);
	add_inst(inst_jnp_b		,0x7b	,1	,-1	,Jb	,-1	);
	add_inst(inst_jl_b		,0x7c	,1	,-1	,Jb	,-1	);
	add_inst(inst_jnl_b		,0x7d	,1	,-1	,Jb	,-1	);
	add_inst(inst_jle_b		,0x7e	,1	,-1	,Jb	,-1	);
	add_inst(inst_jnle_b	,0x7f	,1	,-1	,Jb	,-1	);
	// Immediate Group 1
	add_inst(inst_add_b		,0x80	,1	,0	,Eb	,Ib	);
	add_inst(inst_or_b		,0x80	,1	,1	,Eb	,Ib	);
	add_inst(inst_adc_b		,0x80	,1	,2	,Eb	,Ib	);
	add_inst(inst_sbb_b		,0x80	,1	,3	,Eb	,Ib	);
	add_inst(inst_and_b		,0x80	,1	,4	,Eb	,Ib	);
	add_inst(inst_sub_b		,0x80	,1	,5	,Eb	,Ib	);
	add_inst(inst_xor_b		,0x80	,1	,6	,Eb	,Ib	);
	add_inst(inst_cmp_b		,0x80	,1	,7	,Eb	,Ib	);
	add_inst(inst_add		,0x81	,1	,0	,Ev	,Iv	);
	add_inst(inst_or		,0x81	,1	,1	,Ev	,Iv	);
	add_inst(inst_adc		,0x81	,1	,2	,Ev	,Iv	);
	add_inst(inst_sbb		,0x81	,1	,3	,Ev	,Iv	);
	add_inst(inst_and		,0x81	,1	,4	,Ev	,Iv	);
	add_inst(inst_sub		,0x81	,1	,5	,Ev	,Iv	);
	add_inst(inst_xor		,0x81	,1	,6	,Ev	,Iv	);
	add_inst(inst_cmp		,0x81	,1	,7	,Ev	,Iv	);
	add_inst(inst_add_b		,0x83	,1	,0	,Ev	,Ib	);
	add_inst(inst_or_b		,0x83	,1	,1	,Ev	,Ib	);
	add_inst(inst_adc_b		,0x83	,1	,2	,Ev	,Ib	);
	add_inst(inst_sbb_b		,0x83	,1	,3	,Ev	,Ib	);
	add_inst(inst_and_b		,0x83	,1	,4	,Ev	,Ib	);
	add_inst(inst_sub_b		,0x83	,1	,5	,Ev	,Ib	);
	add_inst(inst_xor_b		,0x83	,1	,6	,Ev	,Ib	);
	add_inst(inst_cmp_b		,0x83	,1	,7	,Ev	,Ib	);
	add_inst(inst_test_b	,0x84	,1	,-1	,Eb	,Gb	);
	add_inst(inst_test		,0x85	,1	,-1	,Ev	,Gv	);
	add_inst(inst_xchg_b	,0x86	,1	,-1	,Eb	,Gb	);
	add_inst(inst_xchg		,0x87	,1	,-1	,Ev	,Gv	);
	add_inst(inst_mov_b		,0x88	,1	,-1	,Eb	,Gb	);
	add_inst(inst_mov		,0x89	,1	,-1	,Ev	,Gv	);
	add_inst(inst_mov_b		,0x8a	,1	,-1	,Gb	,Eb	);
	add_inst(inst_mov		,0x8b	,1	,-1	,Gv	,Ev	);
	add_inst(inst_mov		,0x8c	,1	,-1	,Ew	,Sw	);
	//add_inst(inst_lea		,0x8d	,1	,-1	,Gv	,Ev	);
	add_inst(inst_lea		,0x8d	,1	,-1	,Gv	,Mv	);
	add_inst(inst_mov		,0x8e	,1	,-1	,Sw	,Ew	);
	add_inst(inst_pop		,0x8f	,1	,-1	,Ev	,-1	);
	add_inst(inst_nop		,0x90	,1	,-1	,-1	,-1	);
	add_inst(inst_xchg		,0x91	,1	,-1	,RegEax	,RegEcx	);
	add_inst(inst_xchg		,0x92	,1	,-1	,RegEax	,RegEdx	);
	add_inst(inst_xchg		,0x93	,1	,-1	,RegEax	,RegEbx	);
	add_inst(inst_xchg		,0x94	,1	,-1	,RegEax	,RegEsp	);
	add_inst(inst_xchg		,0x95	,1	,-1	,RegEax	,RegEbp	);
	add_inst(inst_xchg		,0x96	,1	,-1	,RegEax	,RegEsi	);
	add_inst(inst_xchg		,0x97	,1	,-1	,RegEax	,RegEdi	);
	add_inst(inst_cbw_cwde	,0x98	,1	,-1	,-1 ,-1	);
	add_inst(inst_cgq_cwd	,0x99	,1	,-1	,-1 ,-1	);
	add_inst(inst_mov		,0xa0	,1	,-1	,RegAl	,Ob	);
	add_inst(inst_mov		,0xa1	,1	,-1	,RegEax	,Ov	);
	add_inst(inst_mov_b		,0xa2	,1	,-1	,Ob	,RegAl	);
	add_inst(inst_mov		,0xa3	,1	,-1	,Ov	,RegEax	);
	add_inst(inst_movs_b_ds_esi_es_edi	,0xa4	,1	,-1	,-1,-1	);
	add_inst(inst_movs_ds_esi_es_edi	,0xa5	,1	,-1	,-1,-1	);
	add_inst(inst_cmps_b_ds_esi_es_edi	,0xa6	,1	,-1	,-1,-1	);
	add_inst(inst_cmps_ds_esi_es_edi	,0xa7	,1	,-1	,-1,-1	);
	add_inst(inst_mov_b		,0xb0	,1	,-1	,RegAl	,Ib	);
	add_inst(inst_mov_b		,0xb1	,1	,-1	,RegCl	,Ib	);
	add_inst(inst_mov_b		,0xb2	,1	,-1	,RegDl	,Ib	);
	add_inst(inst_mov_b		,0xb3	,1	,-1	,RegBl	,Ib	);
	add_inst(inst_mov_b		,0xb4	,1	,-1	,RegAh	,Ib	);
	add_inst(inst_mov_b		,0xb5	,1	,-1	,RegCh	,Ib	);
	add_inst(inst_mov_b		,0xb6	,1	,-1	,RegDh	,Ib	);
	add_inst(inst_mov_b		,0xb7	,1	,-1	,RegBh	,Ib	);
	add_inst(inst_mov		,0xb8	,1	,-1	,RegEax	,Iv	);
	add_inst(inst_mov		,0xb9	,1	,-1	,RegEcx	,Iv	);
	add_inst(inst_mov		,0xba	,1	,-1	,RegEdx	,Iv	);
	add_inst(inst_mov		,0xbb	,1	,-1	,RegEbx	,Iv	);
	add_inst(inst_mov		,0xbc	,1	,-1	,RegEsp	,Iv	);
	add_inst(inst_mov		,0xbd	,1	,-1	,RegEbp	,Iv	);
	add_inst(inst_mov		,0xbe	,1	,-1	,RegEsi	,Iv	);
	add_inst(inst_mov		,0xbf	,1	,-1	,RegEdi	,Iv	);
	// Shift Group 2
	add_inst(inst_rol_b		,0xc0	,1	,0	,Eb	,Ib	);
	add_inst(inst_ror_b		,0xc0	,1	,1	,Eb	,Ib	);
	add_inst(inst_rcl_b		,0xc0	,1	,2	,Eb	,Ib	);
	add_inst(inst_rcr_b		,0xc0	,1	,3	,Eb	,Ib	);
	add_inst(inst_shl_b		,0xc0	,1	,4	,Eb	,Ib	);
	add_inst(inst_shr_b		,0xc0	,1	,5	,Eb	,Ib	);
	add_inst(inst_sar_b		,0xc0	,1	,7	,Eb	,Ib	);
	add_inst(inst_rol		,0xc1	,1	,0	,Ev	,Ib	); // Ib, auch wenn die Tabelle Iv sagt!!!!
	add_inst(inst_ror		,0xc1	,1	,1	,Ev	,Ib	);
	add_inst(inst_rcl		,0xc1	,1	,2	,Ev	,Ib	);
	add_inst(inst_rcr		,0xc1	,1	,3	,Ev	,Ib	);
	add_inst(inst_shl		,0xc1	,1	,4	,Ev	,Ib	);
	add_inst(inst_shr		,0xc1	,1	,5	,Ev	,Ib	);
	add_inst(inst_sar		,0xc1	,1	,7	,Ev	,Ib	);
	add_inst(inst_ret		,0xc2	,1	,-1	,Iw	,-1	);
	add_inst(inst_ret		,0xc3	,1	,-1	,-1	,-1	);
	add_inst(inst_mov_b		,0xc6	,1	,-1	,Eb	,Ib	);
	add_inst(inst_mov		,0xc7	,1	,-1	,Ev	,Iv	);
	add_inst(inst_leave		,0xc9	,1	,-1	,-1	,-1	);
	add_inst(inst_ret_far	,0xca	,1	,-1	,Iw	,-1	);
	add_inst(inst_ret_far	,0xcb	,1	,-1	,-1	,-1	);
	add_inst(inst_int		,0xcd	,1	,-1	,Ib	,-1	);
	add_inst(inst_iret		,0xcf	,1	,-1	,-1	,-1	);
	add_inst(inst_rol		,0xd3	,1	,0	,Ev	,RegCl	);
	add_inst(inst_ror		,0xd3	,1	,1	,Ev	,RegCl	);
	add_inst(inst_rcl		,0xd3	,1	,2	,Ev	,RegCl	);
	add_inst(inst_rcr		,0xd3	,1	,3	,Ev	,RegCl	);
	add_inst(inst_shl		,0xd3	,1	,4	,Ev	,RegCl	);
	add_inst(inst_shr		,0xd3	,1	,5	,Ev	,RegCl	);
	add_inst(inst_sar		,0xd3	,1	,7	,Ev	,RegCl	);
	add_inst(inst_fadd		,0xd8	,1	,0	,Ev	,-1	);
	add_inst(inst_fmul		,0xd8	,1	,1	,Ev	,-1	);
	add_inst(inst_fsub		,0xd8	,1	,4	,Ev	,-1	);
	add_inst(inst_fdiv		,0xd8	,1	,6	,Ev	,-1	);
	add_inst(inst_fld		,0xd9	,1	,0	,Mv	,-1	);
	add_inst(inst_fst		,0xd9	,1	,2	,Mv	,-1	);
	add_inst(inst_fstp		,0xd9	,1	,3	,Mv	,-1	);
	add_inst(inst_fldcw		,0xd9	,1	,5	,Mw	,-1	);
	add_inst(inst_fnstcw	,0xd9	,1	,7	,Mw	,-1	);
	add_inst(inst_fxch		,0xc9d9	,2	,-1	,RegSt0	,RegSt1	);
	add_inst(inst_fucompp	,0xe9da	,2	,-1	,RegSt0	,RegSt1	);
	add_inst(inst_fistp		,0xdb	,1	,3	,Md	,-1	);

//	FNSTCW
	add_inst(inst_fild		,0xdb	,1	,0	,Ev	,-1	);
	add_inst(inst_faddp		,0xde	,1	,0	,Ev	,-1	);
	add_inst(inst_fmulp		,0xde	,1	,1	,Ev	,-1	);
	add_inst(inst_fsubp		,0xde	,1	,5	,Ev	,-1	);
	add_inst(inst_fdivp		,0xde	,1	,7	,Ev	,-1	); // de.f9 ohne Parameter...?
	add_inst(inst_fnstsw	,0xe0df	,2	,-1	,RegAx	,-1	);
	add_inst(inst_loopne	,0xe0	,1	,-1	,Jb	,-1	);
	add_inst(inst_loope		,0xe1	,1	,-1	,Jb	,-1	);
	add_inst(inst_loop		,0xe2	,1	,-1	,Jb	,-1	);
	add_inst(inst_in_b		,0xe4	,1	,-1	,RegAl	,Ib	);
	add_inst(inst_in_b		,0xe5	,1	,-1	,RegEax,Ib	);
	add_inst(inst_out_b		,0xe6	,1	,-1	,Ib	,RegAl	);
	add_inst(inst_out_b		,0xe7	,1	,-1	,Ib	,RegEax);
	add_inst(inst_call		,0xe8	,1	,-1	,Jv	,-1	); // well... "Av" in tyble
	add_inst(inst_jmp		,0xe9	,1	,-1	,Jv	,-1	); // miswritten in the table
	add_inst(inst_jmp		,0xea	,1	,-1	,Ap	,-1	);
	add_inst(inst_jmp_b		,0xeb	,1	,-1	,Jb	,-1	);
	add_inst(inst_in		,0xec	,1	,-1	,RegAl	,RegDx	);
	add_inst(inst_in		,0xed	,1	,-1	,RegEax,RegDx	);
	add_inst(inst_out		,0xee	,1	,-1	,RegDx	,RegAl	);
	add_inst(inst_out		,0xef	,1	,-1	,RegDx	,RegEax);
	add_inst(inst_lock		,0xf0	,1	,-1	,-1	,-1	);
	add_inst(inst_repne		,0xf2	,1	,-1	,-1	,-1	);
	add_inst(inst_rep		,0xf3	,1	,-1	,-1	,-1	);
	add_inst(inst_hlt		,0xf4	,1	,-1	,-1	,-1	);
	add_inst(inst_cmc		,0xf5	,1	,-1	,-1	,-1	);
	// Unary Group 3
	add_inst(inst_test_b	,0xf6	,1	,0	,Eb	,Ib	);
	add_inst(inst_not_b		,0xf6	,1	,2	,Eb	,-1	);
	add_inst(inst_neg_b		,0xf6	,1	,3	,Eb	,-1	);
	add_inst(inst_mul_b		,0xf6	,1	,4	,RegAl	,Eb	);
	add_inst(inst_imul_b	,0xf6	,1	,5	,RegAl	,Eb	);
	add_inst(inst_div_b		,0xf6	,1	,6	,RegAl	,Eb	);
	add_inst(inst_idiv_b	,0xf6	,1	,7	,Eb	,-1	);
	add_inst(inst_test		,0xf7	,1	,0	,Ev	,Iv	);
	add_inst(inst_not		,0xf7	,1	,2	,Ev	,-1	);
	add_inst(inst_neg		,0xf7	,1	,3	,Ev	,-1	);
	add_inst(inst_mul		,0xf7	,1	,4	,RegEax	,Ev	);
	add_inst(inst_imul		,0xf7	,1	,5	,RegEax	,Ev	);
	add_inst(inst_div		,0xf7	,1	,6	,RegEax	,Ev	);
	add_inst(inst_idiv		,0xf7	,1	,7	,RegEax	,Ev	);
	add_inst(inst_clc		,0xf8	,1	,-1	,-1	,-1	);
	add_inst(inst_stc		,0xf9	,1	,-1	,-1	,-1	);
	add_inst(inst_cli		,0xfa	,1	,-1	,-1	,-1	);
	add_inst(inst_sti		,0xfb	,1	,-1	,-1	,-1	);
	add_inst(inst_cld		,0xfc	,1	,-1	,-1	,-1	);
	add_inst(inst_std		,0xfd	,1	,-1	,-1	,-1	);
	add_inst(inst_inc_b		,0xfe	,1	,0	,Eb	,-1	);
	add_inst(inst_dec_b		,0xfe	,1	,1	,Eb	,-1	);
	add_inst(inst_inc		,0xff	,1	,0	,Ev	,-1	);
	add_inst(inst_dec		,0xff	,1	,1	,Ev	,-1	);
	add_inst(inst_call		,0xff	,1	,2	,Ev	,-1	);
	add_inst(inst_call_far	,0xff	,1	,3	,Ev	,-1	); // Ep statt Ev...
	add_inst(inst_jmp		,0xff	,1	,4	,Ev	,-1	);
//	add_inst(inst_jmp		,0xff	,1	,5	,Ep	,-1	);
	add_inst(inst_push		,0xff	,1	,6	,Ev	,-1	);
}

// convert an asm parameter into a human readable expression
string InstructionParam::str()
{
	msg_db_f("AddParam", 1+ASM_DB_LEVEL);
	string str = "\?\?\?";
	//msg_write("----");
	//msg_write(p.type);
	if (type == ParamTInvalid){
		str = "-\?\?\?-";
	}else if (type == ParamTNone){
		str = "";
	}else if (type == ParamTRegister){
			//msg_write((long)reg);
			//msg_write((long)disp);
		if (deref){
			//msg_write("deref");
			if (disp == DispModeNone)
				str = "[" + reg->name + "]";
			else if (disp == DispMode8)
				str += format("[%s + %2x]", reg->name.c_str(), value);
			else if (disp == DispMode16)
				str += format("[%s + %4x]", reg->name.c_str(), value);
			else if (disp == DispMode32)
				str += format("[%s + %8x]", reg->name.c_str(), value);
			else if (disp == DispModeSIB)
				str = "SIB[...][...]";
			else if (disp == DispMode8SIB)
				str = format("[SIB... + %2x]", value);
			else if (disp == DispMode8Reg2)
				str = format("[%s + %s + %2x]", reg->name.c_str(), reg2->name.c_str(), value);
			else if (disp == DispModeReg2)
				str = "[" + reg->name + " + " + reg2->name + "]";
		}else
			str = reg->name;
	}else if (type == ParamTImmediate){
		//msg_write("im");
		if (deref)
			str = format("[%s]", d2h(&value, state.AddrSize).c_str());
		else
			str = d2h(&value, absolute_size(size));
	}else if (type == ParamTImmediateDouble){
		//msg_write("im");
		str = format("%s:%s", d2h(&((char*)&value)[4], 2).c_str(), d2h(&value, state.ParamSize).c_str());
	}
#if 0
	for (int i=0;i<Registers.num;i++)
		if (param==Registers[i].reg){
			strcat(str,Registers[i].name);
			return;
		}
	switch(param){
		case peAX:	strcat(str,"[eax]");	break;
		case peCX:	strcat(str,"[ecx]");	break;
		case peDX:	strcat(str,"[edx]");	break;
		case peBX:	strcat(str,"[ebx]");	break;
		case pp:	strcat(str,"[--][--]");	break;
		case disp8:	strcat(str,string("[",d2h((char*)&disp,1),"]"));	break;
		case disp16:	strcat(str,string("[",d2h((char*)&disp,2),"]"));	break;
		case disp32:	strcat(str,string("[",d2h((char*)&disp,4),"]"));	break;
		case peSI:	strcat(str,"[esi]");	break;
		case peDI:	strcat(str,"[edi]");	break;
		case pBX_pSI:	strcat(str,"[bx + si]");		break;
		case pBX_pDI:	strcat(str,"[bx + di]");		break;
		case pBP_pSI:	strcat(str,"[bp + si]");		break;
		case pBP_pDI:	strcat(str,"[bp + di]");		break;
		case pBX:	strcat(str,"[bx]");		break;
		case pSI:	strcat(str,"[si]");		break;
		case pDI:	strcat(str,"[di]");		break;

		case d8_peAX:	strcat(str,string("[eax + ",d2h((char*)&disp,1),"]"));	break;
		case d8_peCX:	strcat(str,string("[ecx + ",d2h((char*)&disp,1),"]"));	break;
		case d8_peDX:	strcat(str,string("[edx + ",d2h((char*)&disp,1),"]"));	break;
		case d8_peBX:	strcat(str,string("[ebx + ",d2h((char*)&disp,1),"]"));	break;
		case d8_pp:	strcat(str,string("[--][-- + ",d2h((char*)&disp,1),"]"));	break;
		case d8_peBP:	strcat(str,string("[ebp + ",d2h((char*)&disp,1),"]"));	break;
		case d8_peSI:	strcat(str,string("[esi + ",d2h((char*)&disp,1),"]"));	break;
		case d8_peDI:	strcat(str,string("[edi + ",d2h((char*)&disp,1),"]"));	break;

		case d8_pBX:	strcat(str,string("[bx + ",d2h((char*)&disp,1),"]"));	break;
		case d8_pBP:	strcat(str,string("[bp + ",d2h((char*)&disp,1),"]"));	break;
		case d8_pSI:	strcat(str,string("[si + ",d2h((char*)&disp,1),"]"));	break;
		case d8_pDI:	strcat(str,string("[di + ",d2h((char*)&disp,1),"]"));	break;
	
		case d16_pBX_pSI:	strcat(str,string("[bx + si + ",d2h((char*)&disp,2),"]"));	break;
		case d16_pBX_pDI:	strcat(str,string("[bx + di + ",d2h((char*)&disp,2),"]"));	break;
		case d16_pBP_pSI:	strcat(str,string("[bp + si + ",d2h((char*)&disp,2),"]"));	break;
		case d16_pBP_pDI:	strcat(str,string("[bp + di + ",d2h((char*)&disp,2),"]"));	break;
		case d16_pBX:	strcat(str,string("[bx + ",d2h((char*)&disp,2),"]"));	break;
		case d16_pBP:	strcat(str,string("[bp + ",d2h((char*)&disp,2),"]"));	break;
		case d16_pSI:	strcat(str,string("[si + ",d2h((char*)&disp,2),"]"));	break;
		case d16_pDI:	strcat(str,string("[di + ",d2h((char*)&disp,2),"]"));	break;

		case d32_peAX:	strcat(str,string("[eax + ",d2h((char*)&disp,4),"]"));	break;
		case d32_peCX:	strcat(str,string("[ecx + ",d2h((char*)&disp,4),"]"));	break;
		case d32_peDX:	strcat(str,string("[edx + ",d2h((char*)&disp,4),"]"));	break;
		case d32_peBX:	strcat(str,string("[ebx + ",d2h((char*)&disp,4),"]"));	break;
		case d32_pp:	strcat(str,string("[--][-- + ",d2h((char*)&disp,4),"]"));	break;
		case d32_peBP:	strcat(str,string("[ebp + ",d2h((char*)&disp,4),"]"));	break;
		case d32_peSI:	strcat(str,string("[esi + ",d2h((char*)&disp,4),"]"));	break;
		case d32_peDI:	strcat(str,string("[edi + ",d2h((char*)&disp,4),"]"));	break;

		case Ib:	strcat(str,d2h((char*)&disp,1));	break;
		case Iw:	strcat(str,d2h((char*)&disp,2));	break;
		case Id:	strcat(str,d2h((char*)&disp,4));	break;
		case Ip:
			if ((small_param)&&(!mode16))	strcat(str,"word ");
			if ((!small_param)&&(mode16))	strcat(str,"dword ");
			strcat(str,d2h((char*)&ParamConstantDouble,2));
			strcat(str,":");
			if (small_param)	strcat(str,d2h((char*)&disp,2));
			else				strcat(str,d2h((char*)&disp,4));
			break;

		default:	strcat(str,string(i2s(param),": -\?\?- "));	break;
	};
#endif
	return str;
}

// adjust parameter type to ... 32bit / 16bit
inline void ApplyParamSize(InstructionParamFuzzy &p)
{
	msg_db_f("CorrectParam", 1+ASM_DB_LEVEL);
	if (state.ParamSize == Size16){
		if (p.size == SizeVariable){
			//if ((p.allow_register) || (p.allow_memory_address) || (p.allow_memory_indirect) || (p.allow_immediate))
				p.size = Size16;
		}
		if (p.reg){
			if (p.reg->id == RegEax){	p.reg = RegisterByID[RegAx];	p.size = Size16;	}
			if (p.reg->id == RegEcx){	p.reg = RegisterByID[RegCx];	p.size = Size16;	}
			if (p.reg->id == RegEdx){	p.reg = RegisterByID[RegDx];	p.size = Size16;	}
			if (p.reg->id == RegEbx){	p.reg = RegisterByID[RegBx];	p.size = Size16;	}
			if (p.reg->id == RegEsp){	p.reg = RegisterByID[RegSp];	p.size = Size16;	}
			if (p.reg->id == RegEbp){	p.reg = RegisterByID[RegBp];	p.size = Size16;	}
			if (p.reg->id == RegEsi){	p.reg = RegisterByID[RegSi];	p.size = Size16;	}
			if (p.reg->id == RegEdi){	p.reg = RegisterByID[RegDi];	p.size = Size16;	}
		}
	}else if (state.ParamSize == Size64){
			if (p.size == SizeVariable){
				//if ((p.allow_register) || (p.allow_memory_address) || (p.allow_memory_indirect) || (p.allow_immediate))
					p.size = Size64;
			}
			if (p.reg){
				if (p.reg->id == RegEax){	p.reg = RegisterByID[RegRax];	p.size = Size64;	}
				if (p.reg->id == RegEcx){	p.reg = RegisterByID[RegRcx];	p.size = Size64;	}
				if (p.reg->id == RegEdx){	p.reg = RegisterByID[RegRdx];	p.size = Size64;	}
				if (p.reg->id == RegEbx){	p.reg = RegisterByID[RegRbx];	p.size = Size64;	}
				if (p.reg->id == RegEsp){	p.reg = RegisterByID[RegRsp];	p.size = Size64;	}
				if (p.reg->id == RegEbp){	p.reg = RegisterByID[RegRbp];	p.size = Size64;	}
				if (p.reg->id == RegEsi){	p.reg = RegisterByID[RegRsi];	p.size = Size64;	}
				if (p.reg->id == RegEdi){	p.reg = RegisterByID[RegRdi];	p.size = Size64;	}
			}
	}else{
		if (p.size == SizeVariable){
			//if ((p.type == ParamTRegister) || (p.type == ParamTRegisterOrMem) || (p.type == ParamTImmediate))
			if ((p.allow_register) || (p.allow_memory_address) || (p.allow_memory_indirect) || (p.allow_immediate))
				p.size = Size32;
		}
	}
}

inline void UnfuzzyParam(InstructionParam &p, InstructionParamFuzzy &pf)
{
	msg_db_f("UnfuzzyParam", 2+ASM_DB_LEVEL);
	p.type = pf._type_;
	p.reg2 = NULL;
	p.disp = DispModeNone;
	p.reg = pf.reg;
	p.size = pf.size;
	p.deref = false; // well... FIXME
	p.value = 0;
	p.is_label = false;
	if (pf._type_ == ParamTMemory){
		p.type = ParamTImmediate;
		p.deref = true;
	}
}

Register *GetModRMRegister(int reg, int size)
{
	if (size == Size8){
		if (reg == 0x00)	return RegisterByID[RegAl];
		if (reg == 0x01)	return RegisterByID[RegCl];
		if (reg == 0x02)	return RegisterByID[RegDl];
		if (reg == 0x03)	return RegisterByID[RegBl];
		if (reg == 0x04)	return RegisterByID[RegAh];
		if (reg == 0x05)	return RegisterByID[RegCh];
		if (reg == 0x06)	return RegisterByID[RegDh];
		if (reg == 0x07)	return RegisterByID[RegBh];
	}else if (size == Size16){
		if (reg == 0x00)	return RegisterByID[RegAx];
		if (reg == 0x01)	return RegisterByID[RegCx];
		if (reg == 0x02)	return RegisterByID[RegDx];
		if (reg == 0x03)	return RegisterByID[RegBx];
		if (reg == 0x04)	return RegisterByID[RegSp];
		if (reg == 0x05)	return RegisterByID[RegBp];
		if (reg == 0x06)	return RegisterByID[RegSi];
		if (reg == 0x07)	return RegisterByID[RegDi];
	}else if (size == Size32){
		if (reg == 0x00)	return RegisterByID[RegEax];
		if (reg == 0x01)	return RegisterByID[RegEcx];
		if (reg == 0x02)	return RegisterByID[RegEdx];
		if (reg == 0x03)	return RegisterByID[RegEbx];
		if (reg == 0x04)	return RegisterByID[RegEsp];
		if (reg == 0x05)	return RegisterByID[RegEbp];
		if (reg == 0x06)	return RegisterByID[RegEsi];
		if (reg == 0x07)	return RegisterByID[RegEdi];
	}else if (size == Size64){
		if (reg == 0x00)	return RegisterByID[RegRax];
		if (reg == 0x01)	return RegisterByID[RegRcx];
		if (reg == 0x02)	return RegisterByID[RegRdx];
		if (reg == 0x03)	return RegisterByID[RegRbx];
		if (reg == 0x04)	return RegisterByID[RegRsp];
		if (reg == 0x05)	return RegisterByID[RegRbp];
		if (reg == 0x06)	return RegisterByID[RegRsi];
		if (reg == 0x07)	return RegisterByID[RegRdi];
	}
}

inline void GetFromModRM(InstructionParam &p, InstructionParamFuzzy &pf, unsigned char modrm)
{
	msg_db_f("GetFromModRM", 2+ASM_DB_LEVEL);
	if (pf.mrm_mode == MRMReg){
		unsigned char reg = modrm & 0x38; // bits 5, 4, 3
		p.type = ParamTRegister;
		p.deref = false;
		if (pf.reg_group == RegGroupSegment){
			if (reg == 0x00)	p.reg = RegisterByID[RegEs];
			if (reg == 0x08)	p.reg = RegisterByID[RegCs];
			if (reg == 0x10)	p.reg = RegisterByID[RegSs];
			if (reg == 0x18)	p.reg = RegisterByID[RegDs];
			if (reg == 0x20)	p.reg = RegisterByID[RegFs];
			if (reg == 0x28)	p.reg = RegisterByID[RegGs];
		}else if (pf.reg_group == RegGroupControl){
			if (reg == 0x00)	p.reg = RegisterByID[RegCr0];
			if (reg == 0x08)	p.reg = RegisterByID[RegCr1];
			if (reg == 0x10)	p.reg = RegisterByID[RegCr2];
			if (reg == 0x18)	p.reg = RegisterByID[RegCr3];
		}else{
			p.reg = GetModRMRegister(reg >> 3, p.size);
		}
	}else if (pf.mrm_mode == MRMModRM){
		unsigned char mod = modrm & 0xc0; // bits 7, 6
		unsigned char rm = modrm & 0x07; // bits 2, 1, 0
		if (mod == 0x00){
			if (state.AddrSize == Size16){
				p.type = ParamTRegister;
				p.deref = true;
				if (rm == 0x00){p.reg = RegisterByID[RegBx];	p.reg2 = RegisterByID[RegSi];	p.disp = DispModeReg2;	}
				if (rm == 0x01){p.reg = RegisterByID[RegBx];	p.reg2 = RegisterByID[RegDi];	p.disp = DispModeReg2;	}
				if (rm == 0x02){p.reg = RegisterByID[RegBp];	p.reg2 = RegisterByID[RegSi];	p.disp = DispModeReg2;	}
				if (rm == 0x03){p.reg = RegisterByID[RegBp];	p.reg2 = RegisterByID[RegDi];	p.disp = DispModeReg2;	}
				if (rm == 0x04)	p.reg = RegisterByID[RegSi];
				if (rm == 0x05)	p.reg = RegisterByID[RegDi];
				if (rm == 0x06){p.reg = NULL;	p.type = ParamTImmediate;	}
				if (rm == 0x07)	p.reg = RegisterByID[RegBx];
			}else{
				p.type = ParamTRegister;
				p.deref = true;
				if (rm == 0x00)	p.reg = RegisterByID[RegEax];
				if (rm == 0x01)	p.reg = RegisterByID[RegEcx];
				if (rm == 0x02)	p.reg = RegisterByID[RegEdx];
				if (rm == 0x03)	p.reg = RegisterByID[RegEbx];
				//if (rm == 0x04){p.reg = NULL;	p.disp = DispModeSIB;	p.type = ParamTImmediate;}//p.type = ParamTInvalid;	Error("kein SIB byte...");}
				if (rm == 0x04){p.reg = RegisterByID[RegEax];	p.disp = DispModeSIB;	} // eax = provisoric
				if (rm == 0x05){p.reg = NULL;	p.type = ParamTImmediate;	}
				if (rm == 0x06)	p.reg = RegisterByID[RegEsi];
				if (rm == 0x07)	p.reg = RegisterByID[RegEdi];
			}
		}else if ((mod == 0x40) || (mod == 0x80)){
			if (state.AddrSize == Size16){
				p.type = ParamTRegister;
				p.deref = true;
				if (rm == 0x00){p.reg = RegisterByID[RegBx];	p.reg2 = RegisterByID[RegSi];	p.disp = (mod == 0x40) ? DispMode8Reg2 : DispMode16Reg2;	}
				if (rm == 0x01){p.reg = RegisterByID[RegBx];	p.reg2 = RegisterByID[RegDi];	p.disp = (mod == 0x40) ? DispMode8Reg2 : DispMode16Reg2;	}
				if (rm == 0x02){p.reg = RegisterByID[RegBp];	p.reg2 = RegisterByID[RegSi];	p.disp = (mod == 0x40) ? DispMode8Reg2 : DispMode16Reg2;	}
				if (rm == 0x03){p.reg = RegisterByID[RegBp];	p.reg2 = RegisterByID[RegDi];	p.disp = (mod == 0x40) ? DispMode8Reg2 : DispMode16Reg2;	}
				if (rm == 0x04){p.reg = RegisterByID[RegSi];	p.disp = (mod == 0x40) ? DispMode8 : DispMode16;	}
				if (rm == 0x05){p.reg = RegisterByID[RegDi];	p.disp = (mod == 0x40) ? DispMode8 : DispMode16;	}
				if (rm == 0x06){p.reg = RegisterByID[RegBp];	p.disp = (mod == 0x40) ? DispMode8 : DispMode16;	}
				if (rm == 0x07){p.reg = RegisterByID[RegBx];	p.disp = (mod == 0x40) ? DispMode8 : DispMode16;	}
			}else{
				p.type = ParamTRegister;
				p.deref = true;
				p.disp = (mod == 0x40) ? DispMode8 : DispMode32;
				if (rm == 0x00)	p.reg = RegisterByID[RegEax];
				if (rm == 0x01)	p.reg = RegisterByID[RegEcx];
				if (rm == 0x02)	p.reg = RegisterByID[RegEdx];
				if (rm == 0x03)	p.reg = RegisterByID[RegEbx];
				//if (rm == 0x04){p.reg = NULL;	p.type = ParamTInvalid;	}
				if (rm == 0x04){p.reg = RegisterByID[RegEax];	p.disp = DispMode8SIB;	} // eax = provisoric
				if (rm == 0x05)	p.reg = RegisterByID[RegEbp];
				if (rm == 0x06)	p.reg = RegisterByID[RegEsi];
				if (rm == 0x07)	p.reg = RegisterByID[RegEdi];
			}
		}else if (mod == 0xc0){
			p.type = ParamTRegister;
			p.deref = false;
			p.reg = GetModRMRegister(rm, p.size);
		}
	}
}

inline void TryGetSIB(InstructionParam &p, char *&cur)
{
	if ((p.disp == DispModeSIB) || (p.disp == DispMode8SIB)){
		bool disp8 = (p.disp == DispMode8SIB);
		char sib = *cur;
		cur++;
		unsigned char ss = (sib & 0xc0); // bits 7, 6
		unsigned char index = (sib & 0x38); // bits 5, 4, 3
		unsigned char base = (sib & 0x07); // bits 2, 1, 0
		/*msg_error("SIB");
		msg_write(ss);
		msg_write(index);
		msg_write(base);*/

		// direct?
		//if (p.disp == DispModeSIB){
			if (ss == 0x00){ // scale factor 1
				p.deref = true;
				p.disp = disp8 ? DispMode8Reg2 : DispModeReg2;
				if (base == 0x00)		p.reg = RegisterByID[RegEax];
				else if (base == 0x01)	p.reg = RegisterByID[RegEcx];
				else if (base == 0x02)	p.reg = RegisterByID[RegEdx];
				else if (base == 0x03)	p.reg = RegisterByID[RegEbx];
				else if (base == 0x04)	p.reg = RegisterByID[RegEsp];
				else p.disp = DispModeSIB; // ...
				if (index == 0x00)		p.reg2 = RegisterByID[RegEax];
				else if (index == 0x08)	p.reg2 = RegisterByID[RegEcx];
				else if (index == 0x10)	p.reg2 = RegisterByID[RegEdx];
				else if (index == 0x18)	p.reg2 = RegisterByID[RegEbx];
				else if (index == 0x28)	p.reg2 = RegisterByID[RegEbp];
				else if (index == 0x30)	p.reg2 = RegisterByID[RegEsi];
				else if (index == 0x38)	p.reg2 = RegisterByID[RegEdi];
				else p.disp = disp8 ? DispMode8 : DispModeNone;
			}
		//}
	}
}

inline void ReadParamData(char *&cur, InstructionParam &p)
{
	msg_db_f("ReadParamData", 2+ASM_DB_LEVEL);
	//char *o = cur;
	p.value = 0;
	if (p.type == ParamTImmediate){
		if (p.deref){
			memcpy(&p.value, cur, state.AddrSize);
			cur += state.AddrSize;
		}else{
			memcpy(&p.value, cur, absolute_size(p.size));
			cur += absolute_size(p.size);
		}
	}else if (p.type == ParamTImmediateDouble){
		if (state.ParamSize == Size16){ // addr?
			*(short*)&p.value = *(short*)cur;	cur += 2;	((short*)&p.value)[2] = *(short*)cur;	cur += 2;
		}else{
			memcpy(&p.value, cur, 6);		cur += 6;
		}
	}else if (p.type == ParamTRegister){
		if ((p.disp == DispMode8) || (p.disp == DispMode8Reg2) || (p.disp == DispMode8SIB)){
			*(char*)&p.value = *cur;		cur ++;
		}else if (p.disp == DispMode16){
			*(short*)&p.value = *(short*)cur;		cur += 2;
		}else if (p.disp == DispMode32){
			*(int*)&p.value = *(int*)cur;		cur += 4;
		}
	}
	//msg_write((long)cur - (long)o);
}

// convert some opcode into (human readable) assembler language
string Disassemble(void *_code_,int length,bool allow_comments)
{
	msg_db_f("Disassemble", 1+ASM_DB_LEVEL);
	/*if (!Instruction)
		SetInstructionSet(InstructionSetDefault);*/

	char *code = (char*)_code_;

	string param;
	char *opcode;
	string bufstr;
	char *end=code+length;
	char *orig=code;
	if (length<0)	end=code+65536;

	// code points to the start of the (current) complete command (dword cs: mov ax, ...)
	// cur points to the currently processed byte
	// opcode points to the start of the instruction (mov)
	char *cur = code;
	state.DefaultSize = Size32;
	if (CurrentMetaInfo)
		if (CurrentMetaInfo->Mode16)
			state.DefaultSize = Size16;


	while(code < end){
		state.reset();
		opcode = cur;
		code = cur;

		// done?
		if (code >= end)
			break;

		// special info
		if (CurrentMetaInfo){

			// labels
#if 0
			// TODO
			for (int i=0;i<CurrentMetaInfo->label.num;i++)
				if ((long)code - (long)orig == CurrentMetaInfo->label[i].Pos)
					bufstr += "    " + CurrentMetaInfo->label[i].Name + ":\n";
#endif

			// data blocks
			bool inserted = false;
			for (int i=0;i<CurrentMetaInfo->data.num;i++){
				//printf("%d  %d  %d  %d\n", CurrentMetaInfo->data[i].Pos, (long)code, (long)orig, (long)code - (long)orig);
				if ((long)code - (long)orig == CurrentMetaInfo->data[i].Pos){
					//msg_write("data");
					if (CurrentMetaInfo->data[i].Size==1){
						bufstr += "  db\t";
						bufstr += d2h(cur,1);
					}else if (CurrentMetaInfo->data[i].Size==2){
						bufstr += "  dw\t";
						bufstr += d2h(cur,2);
					}else if (CurrentMetaInfo->data[i].Size==4){
						bufstr += "  dd\t";
						bufstr += d2h(cur,4);
					}else{
						bufstr += "  ds \t...";
					}
					cur += CurrentMetaInfo->data[i].Size;
					bufstr += "\n";
					inserted = true;
				}
			}
			if (inserted)
				continue;

			// change of bits (processor mode)
			for (int i=0;i<CurrentMetaInfo->bit_change.num;i++)
				if ((long)code-(long)orig == CurrentMetaInfo->bit_change[i].Pos){
					state.DefaultSize = (CurrentMetaInfo->bit_change[i].Bits == 16) ? Size16 : Size32;
					state.reset();
					if (state.DefaultSize == Size16)
						bufstr += "   bits_16\n";
					else
						bufstr += "   bits_32\n";
				}
		}

		// code

		// prefix (size/segment register)
		Register *seg = NULL;
		if (cur[0]==0x67){
			state.AddrSize = (state.DefaultSize == Size32) ? Size16 : Size32;
			cur++;
		}
		if (cur[0]==0x66){
			state.ParamSize = (state.DefaultSize == Size32) ? Size16 : Size32;
			cur++;
		}
		if ((cur[0]==0x48) && (InstructionSet == InstructionSetAMD64)){
			state.ParamSize = Size64;
			cur++;
		}
		if (cur[0]==0x2e){	seg = RegisterByID[RegCs];	cur++;	}
		else if (cur[0]==0x36){	seg = RegisterByID[RegSs];	cur++;	}
		else if (cur[0]==0x3e){	seg = RegisterByID[RegDs];	cur++;	}
		else if (cur[0]==0x26){	seg = RegisterByID[RegEs];	cur++;	}
		else if (cur[0]==0x64){	seg = RegisterByID[RegFs];	cur++;	}
		else if (cur[0]==0x65){	seg = RegisterByID[RegGs];	cur++;	}
		opcode=cur;

		// instruction
		sInstruction *inst = NULL;
		for (int i=0;i<Instruction.num;i++){
			// opcode correct?
			bool ok = true;
			for (int j=0;j<Instruction[i].code_size;j++)
				if (cur[j] != ((char*)&Instruction[i].code)[j])
					ok = false;
			// cap correct?
			if (Instruction[i].cap >= 0)
				ok &= ((unsigned char)Instruction[i].cap == ((unsigned)cur[1] / 8) % 8);
			if (ok){
				inst = &Instruction[i];
				cur += inst->code_size;
				break;
			}
		}
		if (inst){
			InstructionParamFuzzy ip1 = inst->param1;
			InstructionParamFuzzy ip2 = inst->param2;
			ApplyParamSize(ip1);
			ApplyParamSize(ip2);

			
			InstructionParam p1, p2;
			UnfuzzyParam(p1, ip1);
			UnfuzzyParam(p2, ip2);

			// modr/m byte
			if (inst->has_modrm){
				//msg_write("modrm");
				char modrm = *cur;
				cur ++;
				GetFromModRM(p1, ip1, modrm);
				GetFromModRM(p2, ip2, modrm);
				TryGetSIB(p1, cur);
				TryGetSIB(p2, cur);
			}

			// immediate...
			ReadParamData(cur, p1);
			ReadParamData(cur, p2);



		// create asm code
			string str;

			// segment register?
			if (seg)
				str += seg->name + ": ";

			// command
			str += inst->name;

			// parameters
			if (state.ParamSize != state.DefaultSize){
				if (state.ParamSize == Size16)
					str += " word";
				else if (state.ParamSize == Size32)
					str += " dword";
				else if (state.ParamSize == Size64)
					str += " qword";
			}
			if (p1.type != ParamTNone)
				str += " " + p1.str();
			if (p2.type != ParamTNone)
				str += ", " + p2.str();
			
			
			if (allow_comments){
				int l = str.num;
				str += " ";
				for (int ii=0;ii<48-l;ii++)
					str += " ";
				str += "// ";
				str += d2h(code,long(cur) - long(code), false);
			}
			//msg_write(str);
			bufstr += str;
			bufstr += "\n";

#if 0
			
			bool e_first=false;
			char modRM;
			int mod,Reg,rm;
			if ((p1>=0)||(p2>=0)){
				if ((p1==Ed)||(p1==Ew)||(p1==Eb)||(p1==Sw)||(p2==Ed)||(p2==Ew)||(p2==Eb)||(p2==Sw)){
					e_first=((p1==Ed)||(p1==Ew)||(p1==Eb)) && (p1!=Sw);
					if (!e_first){	int t=p1;		p1=p2;			p2=t;	}
					if (!e_first){	int t=pp1;		pp1=pp2;		pp2=t;	}
					if (!e_first){	int t=disp1;	disp1=disp2;	disp2=t;	}
					cur++;
					modRM=cur[0];
					mod=(unsigned char)modRM/64;
					Reg=((unsigned char)modRM/8)%8;
					rm=(unsigned char)modRM%8;
					if (mod==0){
						if (mode16){
							if (rm==0)	pp1=pBX_pSI;
							if (rm==1)	pp1=pBX_pDI;
							if (rm==2)	pp1=pBP_pSI;
							if (rm==3)	pp1=pBP_pDI;
							if (rm==4)	pp1=pSI;
							if (rm==5)	pp1=pDI;
							if (rm==6){	pp1=disp16;	disp1=*(short*)&cur[1];	cur+=2;	}
							if (rm==7)	pp1=pBX;
						}else{
							if (rm==0)	pp1=peAX;
							if (rm==1)	pp1=peCX;
							if (rm==2)	pp1=peDX;
							if (rm==3)	pp1=peBX;
							if (rm==4){	pp1=pp;		disp1=cur[1];		cur++;	}
							if (rm==5){	pp1=disp32;	disp1=*(int*)&cur[1];	cur+=4; }
							if (rm==6)	pp1=peSI;
							if (rm==7)	pp1=peDI;
						}
					}else if (mod==1){
						if (mode16){
							if (rm==0)	pp1=d8_pBX_pSI;
							if (rm==1)	pp1=d8_pBX_pDI;
							if (rm==2)	pp1=d8_pBP_pSI;
							if (rm==3)	pp1=d8_pBP_pDI;
							if (rm==4)	pp1=d8_pSI;
							if (rm==5)	pp1=d8_pDI;
							if (rm==6)	pp1=d8_pBP;
							if (rm==7)	pp1=d8_pBX;
							disp1=cur[1];	cur++;
						}else{
							if (rm==0)	pp1=d8_peAX;
							if (rm==1)	pp1=d8_peCX;
							if (rm==2)	pp1=d8_peDX;
							if (rm==3)	pp1=d8_peBX;
							if (rm==4){	pp1=d8_pp;		disp1=cur[1];		cur++;	}
							if (rm==5)	pp1=d8_peBP;
							if (rm==6)	pp1=d8_peSI;
							if (rm==7)	pp1=d8_peDI;
							disp1=cur[1];	cur++;
						}
					}else if (mod==2){
						if (mode16){
							if (rm==0)	pp1=d16_pBX_pSI;
							if (rm==1)	pp1=d16_pBX_pDI;
							if (rm==2)	pp1=d16_pBP_pSI;
							if (rm==3)	pp1=d16_pBP_pDI;
							if (rm==4)	pp1=d16_pSI;
							if (rm==5)	pp1=d16_pDI;
							if (rm==6)	pp1=d16_pBP;
							if (rm==7)	pp1=d16_pBX;
							disp1=*(short*)&cur[1];	cur+=2;
						}else{
							if (rm==0)	pp1=d32_peAX;
							if (rm==1)	pp1=d32_peCX;
							if (rm==2)	pp1=d32_peDX;
							if (rm==3)	pp1=d32_peBX;
							if (rm==4){	pp1=d32_pp;		disp1=cur[1];		cur++;	}
							if (rm==5)	pp1=d32_peBP;
							if (rm==6)	pp1=d32_peSI;
							if (rm==7)	pp1=d32_peDI;
							disp1=*(int*)&cur[1];	cur+=4;
						}
					}else if (mod==3){
						if (p1==Eb){
							if (rm==0)	pp1=AL;
							if (rm==1)	pp1=CL;
							if (rm==2)	pp1=DL;
							if (rm==3)	pp1=BL;
							if (rm==4)	pp1=AH;
							if (rm==5)	pp1=CH;
							if (rm==6)	pp1=DH;
							if (rm==7)	pp1=BH;
						}else if (p1==Ew){
							if (rm==0)	pp1=AX;
							if (rm==1)	pp1=CX;
							if (rm==2)	pp1=DX;
							if (rm==3)	pp1=BX;
							if (rm==4)	pp1=SP;
							if (rm==5)	pp1=BP;
							if (rm==6)	pp1=SI;
							if (rm==7)	pp1=DI;
						}else if (p1==Ed){
							if (rm==0)	pp1=eAX;
							if (rm==1)	pp1=eCX;
							if (rm==2)	pp1=eDX;
							if (rm==3)	pp1=eBX;
							if (rm==4)	pp1=eSP;
							if (rm==5)	pp1=eBP;
							if (rm==6)	pp1=eSI;
							if (rm==7)	pp1=eDI;
						}
					}
					if (p2==Gb){
						if (Reg==0)	pp2=AL;
						if (Reg==1)	pp2=CL;
						if (Reg==2)	pp2=DL;
						if (Reg==3)	pp2=BL;
						if (Reg==4)	pp2=AH;
						if (Reg==5)	pp2=CH;
						if (Reg==6)	pp2=DH;
						if (Reg==7)	pp2=BH;
					}else if (p2==Gw){
						if (Reg==0)	pp2=AX;
						if (Reg==1)	pp2=CX;
						if (Reg==2)	pp2=DX;
						if (Reg==3)	pp2=BX;
						if (Reg==4)	pp2=SP;
						if (Reg==5)	pp2=BP;
						if (Reg==6)	pp2=SI;
						if (Reg==7)	pp2=DI;
					}else if (p2==Gd){
						if (Reg==0)	pp2=eAX;
						if (Reg==1)	pp2=eCX;
						if (Reg==2)	pp2=eDX;
						if (Reg==3)	pp2=eBX;
						if (Reg==4)	pp2=eSP;
						if (Reg==5)	pp2=eBP;
						if (Reg==6)	pp2=eSI;
						if (Reg==7)	pp2=eDI;
					}else if (p2==Sw){
						if (Reg==0)	pp2=ES;
						if (Reg==1)	pp2=CS;
						if (Reg==2)	pp2=SS;
						if (Reg==3)	pp2=DS;
						if (Reg==4)	pp2=FS;
						if (Reg==5)	pp2=GS;
					}else if (p2==Cd){
						if (Reg==0)	pp2=CR0;
						if (Reg==1)	pp2=CR1;
						if (Reg==2)	pp2=CR2;
						if (Reg==3)	pp2=CR3;
					}else if ((p2==Ob)||(p2==Ow)||(p2==Od)){
						if (small_param){
							pp2=disp16;
							disp2=*(short*)&cur[1];	cur+=2;
						}else{
							pp2=disp32;
							disp2=*(int*)&cur[1];	cur+=4;
						}
					}

					if (!e_first){	int t=pp1;	pp1=pp2;	pp2=t;	}
					if (!e_first){	int t=p1;	p1=p2;		p2=t;	}
					if (!e_first){	int t=disp1;	disp1=disp2;	disp2=t;	}
				}else if ((p1==Ob)||(p1==Ow)||(p1==Od)){
					if (small_param){
						pp1=disp16;
						disp1=*(short*)&cur[1];	cur+=2;
					}else{
						pp1=disp32;
						disp1=*(int*)&cur[1];	cur+=4;
					}
				}else if (p1==Ip){
					if (small_param){	disp1=*(short*)&cur[1];	cur+=2;	}
					else{				disp1=*(int*)&cur[1];	cur+=4;	}
					ParamConstantDouble=*(short*)&cur[1];	cur+=2;
				}else if (p1==Id){
					disp1=*(int*)&cur[1];	cur+=4;
				}else if (p1==Iw){
					disp1=*(short*)&cur[1];	cur+=2;
				}else if (p1==Ib){
					disp1=cur[1];	cur++;
				}
				// Param2
				if ((p2==Ob)||(p2==Ow)||(p2==Od)){
					if (small_param){
						pp2=disp16;
						disp2=*(short*)&cur[1];	cur+=2;
					}else{
						pp2=disp32;
						disp2=*(int*)&cur[1];	cur+=4;
					}
				}else if (p2==Id){
					disp2=*(int*)&cur[1];	cur+=4;
				}else if (p2==Iw){
					disp2=*(short*)&cur[1];	cur+=2;
				}else if (p2==Ib){
					disp2=cur[1];	cur++;
				}
				//for (int i=0;i<32-l;i++)
				//	strcat(str," ");
				strcat(param," ");
				AddParam(param,pp1,disp1);
				if (p2>=0){
					strcat(param,", ");
					AddParam(param,pp2,disp2);
				}
			}
			char str[128];		strcpy(str,"");
			if (seg==CS)	strcat(str,"CS: ");
			if (seg==SS)	strcat(str,"SS: ");
			if (seg==DS)	strcat(str,"DS: ");
			if (seg==ES)	strcat(str,"ES: ");
			if (seg==FS)	strcat(str,"FS: ");
			if (seg==GS)	strcat(str,"GS: ");
			strcat(str,string(Instruction[ae].name,param));
			strcat(buffer,str);
			if (allow_comments){
				int l=strlen(str);
				strcat(buffer," ");
				for (int ii=0;ii<48-l;ii++)
					strcat(buffer," ");
				strcat(buffer,"// ");
				strcat(buffer,d2h(code,1+long(cur)-long(code),false));
			}
#endif
		}else{
			//msg_write(string2("????? -                          unknown         // %s\n",d2h(code,1+long(cur)-long(code),false)));
			bufstr += format("????? -                          unknown         // %s\n",d2h(code,1+long(cur)-long(code),false).c_str());
			cur ++;
		}

		// done?
		if ((length < 0) && (((unsigned char)opcode[0] == 0xc3) || ((unsigned char)opcode[0] == 0xc2)))
			break;
	}
	return bufstr;
}

// skip unimportant code (whitespace/comments)
//    returns true if end of code
bool IgnoreUnimportant(int &pos)
{
	msg_db_f("IgnoreUnimportant", 4+ASM_DB_LEVEL);
	bool CommentLine = false;
	
	// ignore comments and "white space"
	for (int i=0;i<1048576;i++){
		if (code_buffer[pos] == 0){
			state.EndOfCode = true;
			state.EndOfLine = true;
			return true;
		}
		if (code_buffer[pos] == '\n'){
			state.LineNo ++;
			state.ColumnNo = 0;
			CommentLine = false;
		}
		// "white space"
		if ((code_buffer[pos] == '\n') || (code_buffer[pos] == ' ') || (code_buffer[pos] == '\t')){
			pos ++;
			state.ColumnNo ++;
			continue;
		}
		// comments
		if ((code_buffer[pos] == ';') || ((code_buffer[pos] == '/') && (code_buffer[pos] == '/'))){
			CommentLine = true;
			pos ++;
			state.ColumnNo ++;
			continue;
		}
		if (!CommentLine)
			break;
		pos ++;
		state.ColumnNo ++;
	}
	return false;
}

// returns one "word" in the source code
string FindMnemonic(int &pos)
{
	msg_db_f("GetMne", 1+ASM_DB_LEVEL);
	state.EndOfLine = false;
	char mne[128];
	strcpy(mne, "");

	if (IgnoreUnimportant(pos))
		return mne;
	
	bool in_string = false;
	for (int i=0;i<128;i++){
		mne[i] = code_buffer[pos];
		mne[i + 1] = 0;
		
		// string like stuff
		if ((mne[i] == '\'') || (mne[i] == '\"'))
			in_string =! in_string;
		// end of code
		if (code_buffer[pos] == 0){
			mne[i] = 0;
			state.EndOfCode = true;
			state.EndOfLine = true;
			break;
		}
		// end of line
		if (code_buffer[pos] == '\n'){
			mne[i] = 0;
			state.EndOfLine = true;
			break;
		}
		if (!in_string){
			// "white space" -> complete
			if ((code_buffer[pos] == ' ') || (code_buffer[pos] == '\t') || (code_buffer[pos] == ',')){
				mne[i] = 0;
				// end of line?
				for (int j=0;j<128;j++){
					if ((code_buffer[pos+j] != ' ') && (code_buffer[pos+j] != '\t') && (code_buffer[pos+j] != ',')){
						if ((code_buffer[pos + j] == 0) || (code_buffer[pos + j] == '\n'))
							state.EndOfLine = true;
						// comment ending the line
						if ((code_buffer[pos + j] == ';') || ((code_buffer[pos + j] == '/') && (code_buffer[pos + j + 1] == '/')))
							state.EndOfLine = true;
						pos += j;
						state.ColumnNo += j;
						if (code_buffer[pos] == '\n')
							state.ColumnNo = 0;
						break;
					}
				}
				break;
			}
		}
		pos ++;
		state.ColumnNo ++;
	}
	/*msg_write>Write(mne);
	if (EndOfLine)
		msg_write>Write("    eol");*/
	return mne;
}

// interpret an expression from source code as an assembler parameter
void GetParam(InstructionParam &p, const string &param, InstructionWithParamsList &list, int pn)
{
	msg_db_f("GetParam", 1+ASM_DB_LEVEL);
	p.type = ParamTInvalid;
	p.reg = NULL;
	p.deref = false;
	p.size = SizeUnknown;
	p.disp = DispModeNone;
	p.is_label = false;
	//msg_write(param);

	// none
	if (param.num == 0){
		p.type = ParamTNone;

	// deref
	}else if ((param[0] == '[') && (param[param.num-1] == ']')){
		if (DebugAsm)
			printf("deref:   ");
		so("Deref:");
		//bool u16 = use_mode16;
		GetParam(p, param.substr(1, -2), list, pn);
		p.deref = true;
		//use_mode16 = u16;

	// string
	}else if ((param[0] == '\"') && (param[param.num-1] == '\"')){
		if (DebugAsm)
			printf("String:   ");
		char *ps = new char[param.num - 1];
		strcpy(ps, param.substr(1, -2).c_str());
		p.value = (long)ps;
		p.type = ParamTImmediate;

	// complex...
	}else if (param.find("+") >= 0){
		if (DebugAsm)
			printf("complex:   ");
		InstructionParam sub;
		
		// first part (must be a register)
		string part;
		for (int i=0;i<param.num;i++)
			if ((param[i] == ' ') || (param[i] == '+'))
				break;
			else
				part.add(param[i]);
		int offset = part.num;
		GetParam(sub, part, list, pn);
		if (sub.type == ParamTRegister){
			//msg_write("reg");
			p.type = ParamTRegister;
			p.size = Size32;
			p.reg = sub.reg;
		}else
			p.type = ParamTInvalid;

		// second part (...up till now only hex)
		for (int i=offset;i<param.num;i++)
			if ((param[i] != ' ') && (param[i] != '+')){
				offset = i;
				break;
			}
		part = param.substr(offset, -1);
		GetParam(sub, part, list, pn);
		if (sub.type == ParamTImmediate){
			//msg_write("c2 = im");
			if (((long)sub.value & 0xffffff00) == 0)
				p.disp = DispMode8;
			else
				p.disp = DispMode32;
			p.value = sub.value;
		}else
			p.type = ParamTInvalid;

		

	// hex const
	}else if ((param[0] == '0') && (param[1] == 'x')){
		p.type = ParamTImmediate;
		int v = 0;
		for (int i=2;i<param.num;i++){
			if (param[i] == '.'){
			}else if ((param[i] >= 'a') && (param[i] <= 'f')){
				v *= 16;
				v += param[i] - 'a' + 10;
			}else if ((param[i] >= 'A') && (param[i] <= 'F')){
				v *= 16;
				v += param[i]-'A'+10;
			}else if ((param[i]>='0')&&(param[i]<='9')){
				v*=16;
				v+=param[i]-'0';
			}else if (param[i]==':'){
				InstructionParam sub;
				GetParam(sub, param.tail(param.num - i), list, pn);
				if (sub.type != ParamTImmediate){
					SetError("error in hex parameter:  " + string(param));
					p.type = PKInvalid;
					return;						
				}
				p.value = (long)v;
				p.value <<= 32;
				p.value += sub.value;
				p.type = ParamTImmediateDouble;
				break;
			}else{
				SetError("evil character in hex parameter:  \"" + param + "\"");
				p.type = ParamTInvalid;
				return;
			}
			p.value = (long)v;
		}
		if (DebugAsm){
			if (p.type == ParamTImmediateDouble)
				printf("hex const:  %s:%s\n",d2h((char*)&p.value+4,2).c_str(),d2h((char*)&p.value,4).c_str());
			else
				printf("hex const:  %s\n",d2h((char*)&p.value,4).c_str());
		}

	// char const
	}else if ((param[0] == '\'') && (param[param.num - 1] == '\'')){
		p.value = (long)param[1];
		p.type = ParamTImmediate;
		if (DebugAsm)
			printf("hex const:  %s\n",d2h((char*)&p.value,1).c_str());

	// label substitude
	}else if (param == "$"){
		p.value = list.add_label(param, true);
		p.type = ParamTImmediate;
		p.is_label = true;
		so("label:  " + param + "\n");
		
	}else{
		// register
		for (int i=0;i<Registers.num;i++)
			if (Registers[i].name == param){
				p.type = ParamTRegister;
				p.reg = &Registers[i];
				p.size = Registers[i].size;
				so("Register:  " + Registers[i].name + "\n");
				return;
			}
		// existing label
		for (int i=0;i<list.label.num;i++)
			if (list.label[i].Name == param){
				p.value = i;
				p.type = ParamTImmediate;
				p.is_label = true;
				so("label:  " + param + "\n");
				return;
			}
		// script variable (global)
		for (int i=0;i<CurrentMetaInfo->global_var.num;i++){
			if (CurrentMetaInfo->global_var[i].Name == param){
				p.value = (long)CurrentMetaInfo->global_var[i].Pos;
				p.type = ParamTImmediate;
				p.deref = true;
				so("global variable:  \"" + param + "\"\n");
				return;
			}
		}
		// not yet existing label...
		if (param[0]=='_'){
			so("label as param:  \"" + param + "\"\n");
			p.value = list.add_label(param, false);
			p.type = ParamTImmediate;
			p.is_label = true;
			return;
		}
	}
	if (p.type == ParamTInvalid)
		SetError("unknown parameter:  \"" + param + "\"\n");
}

inline void insert_val(char *oc, int &ocs, long long val, int size)
{
	if (size == 1)
		oc[ocs] = (char)val;
	else if (size == 2)
		*(short*)&oc[ocs] = (short)val;
	else if (size == 4)
		*(int*)&oc[ocs] = (int)val;
	else if (size == 8)
		*(long long int*)&oc[ocs] = val;
	else
		memcpy(&oc[ocs], &val, size);
}

inline void append_val(char *oc, int &ocs, long long val, int size)
{
	insert_val(oc, ocs, val, size);
	ocs += size;
}

void OpcodeAddImmideate(char *oc, int &ocs, InstructionParamFuzzy &ip, InstructionParam &p, sInstruction &inst, InstructionWithParamsList &list)
{
	int size = 0;
	if (p.type == ParamTImmediate){
		if (p.deref)
			size = state.AddrSize;
		else
			size = absolute_size(ip.size);
	}else if (p.type == ParamTImmediateDouble){
		size = state.ParamSize;  // bits 0-15  /  0-31
	}else if (p.type == ParamTRegister){
		if (p.disp == DispMode8)	size = 1;
		if (p.disp == DispMode16)	size = 2;
		if (p.disp == DispMode32)	size = 4;
	}else
		return;

	long long value = p.value;
	bool rel = ((inst.name[0] == 'j') && (inst.param1._type_ != ParamTImmediateDouble)) || (inst.name == "call") || (inst.name.find("loop") >= 0);
	if (p.is_label){
		WantedLabel w;
		w.Pos = ocs;// + CurrentMetaInfo->PreInsertionLength;
		w.Size = size;
		w.LabelNo = value;
		w.Name = list.label[p.value].Name;
		w.Relative = rel;
		w.InstNo = list.current_inst;
		list.wanted_label.add(w);
		so("add wanted label");
	}else if (rel){
		value -= (long)oc + ocs + size; // TODO ...first byte of next opcode
	}

	append_val(oc, ocs, value, size);


	if (p.type == ParamTImmediateDouble)
		append_val(oc, ocs, p.value >> 32, 2); // bits 33-47
}

void InstructionWithParamsList::LinkWantedLabels(void *oc)
{
	foreachib(WantedLabel &w, wanted_label, i){
		Label &l = label[w.LabelNo];
		if (l.Value < 0)
			continue;
		so("linking label");

		int value = l.Value;
		if (w.Relative)
			value -= (long)oc + w.Pos + w.Size; // TODO first byte after command

		insert_val((char*)oc, w.Pos, value, w.Size);


		wanted_label.erase(i);
		_foreach_it_.update();
	}
}

void InstructionWithParamsList::AppendFromSource(const char *code)
{
	msg_db_f("AppendFromSource", 1+ASM_DB_LEVEL);

	if (CurrentMetaInfo){
		CurrentMetaInfo->PreInsertionLength = CurrentMetaInfo->CurrentOpcodePos; // position of the block withing (overall) opcode
	}else{
		SetError("no CurrentMetaInfo");
	}

	state.LineNo = CurrentMetaInfo->LineOffset;
	state.ColumnNo = 0;

	// CurrentMetaInfo->CurrentOpcodePos // Anfang aktuelle Zeile im gesammten Opcode
	code_buffer = code; // Asm-Source-Puffer

	int pos = 0;
	InstructionParam p1, p2;
	state.DefaultSize = Size32;
	if (CurrentMetaInfo)
		if (CurrentMetaInfo->Mode16)
			state.DefaultSize = Size16;
	state.EndOfCode = false;
	while((unsigned)pos < strlen(code) - 2){

		string cmd, param1, param2;

		//msg_write("..");
		state.reset();
#if 0
		if (CurrentMetaInfo){
			CurrentMetaInfo->CurrentOpcodePos = CurrentMetaInfo->PreInsertionLength + CodeLength;
			//msg_write(CurrentMetaInfo->CurrentOpcodePos);
		}
#endif


	// interpret asm code (1 line)
		// find command
		cmd = FindMnemonic(pos);
		current_line = state.LineNo;
		current_col = state.ColumnNo;
		//msg_write(cmd);
		if (cmd.num == 0)
			break;
		// find parameters
		if (!state.EndOfLine){
			param1 = FindMnemonic(pos);
			if ((param1 == "dword") || (param1 == "word") || (param1 == "qword")){
				if (param1 == "word")
					state.ParamSize = Size16;
				else if (param1 == "dword")
					state.ParamSize = Size32;
				else if (param1 == "qword")
					state.ParamSize = Size64;
				if (!state.EndOfLine)
					param1 = FindMnemonic(pos);
			}
		}
		if (!state.EndOfLine)
			param2 = FindMnemonic(pos);
		//msg_write(string2("----: %s %s%s %s", cmd, param1, (strlen(param2)>0)?",":"", param2));
		if (state.EndOfCode)
			break;
		so("------------------------------");
		so(cmd);
		so(param1);
		so(param2);
		so("------");

		// parameters
		GetParam(p1, param1, *this, 0);
		GetParam(p2, param2, *this, 1);
		if ((p1.type == ParamTInvalid) || (p2.type == ParamTInvalid))
			return;

	// special stuff
		if (cmd == "bits_16"){
			so("16 bit Modus!");
			state.DefaultSize = Size16;
			state.reset();
			if (CurrentMetaInfo){
				CurrentMetaInfo->Mode16 = true;
				BitChange b;
				b.Pos = CurrentMetaInfo->CurrentOpcodePos;
				b.Bits = 16;
				CurrentMetaInfo->bit_change.add(b);
			}
			continue;
		}else if (cmd == "bits_32"){
			so("32 bit Modus!");
			state.DefaultSize = Size32;
			state.reset();
			if (CurrentMetaInfo){
				CurrentMetaInfo->Mode16 = false;
				BitChange b;
				b.Pos = CurrentMetaInfo->CurrentOpcodePos;
				b.Bits = 32;
				CurrentMetaInfo->bit_change.add(b);
			}
			continue;
#if 0
		}else if (cmd == "db"){
			so("Daten:   1 byte");
			if (CurrentMetaInfo){
				AsmData d;
				d.Pos = CurrentMetaInfo->CurrentOpcodePos;
				d.Size = 1;
				CurrentMetaInfo->data.add(d);
			}
			buffer[CodeLength++]=(char)(long)p1.value;
			continue;
		}else if (cmd == "dw"){
			so("Daten:   2 byte");
			if (CurrentMetaInfo){
				AsmData d;
				d.Pos = CurrentMetaInfo->CurrentOpcodePos;
				d.Size = 2;
				CurrentMetaInfo->data.add(d);
			}
			*(short*)&buffer[CodeLength]=(short)(long)p1.value;	CodeLength+=2;
			continue;
		}else if (cmd == "dd"){
			so("Daten:   4 byte");
			if (CurrentMetaInfo){
				AsmData d;
				d.Pos = CurrentMetaInfo->CurrentOpcodePos;
				d.Size = 4;
				CurrentMetaInfo->data.add(d);
			}
			*(int*)&buffer[CodeLength]=(long)p1.value;	CodeLength+=4;
			continue;
		}else if ((cmd == "ds") || (cmd == "dz")){
			so("Daten:   String");
			char *s = (char*)p1.value;
			int l=strlen(s);
			if (cmd == "dz")
				l ++;
			if (CurrentMetaInfo){
				AsmData d;
				d.Pos = CurrentMetaInfo->CurrentOpcodePos;
				d.Size = l;
				CurrentMetaInfo->data.add(d);
			}
			memcpy(&buffer[CodeLength], s, l);
			CodeLength += l;
			continue;
#endif
		}else if (cmd == "org"){
			if (CurrentMetaInfo)
				CurrentMetaInfo->CodeOrigin = (long)p1.value;
			continue;
		}else if (cmd[cmd.num - 1] == ':'){
			so("Label");
			cmd.resize(cmd.num - 1);
			so(cmd);
			add_label(cmd, true);

			continue;
		}

		// command
		int inst = -1;
		for (int i=0;i<NUM_INSTRUCTION_NAMES;i++)
			if (InstructionNames[i].name == cmd)
				inst = InstructionNames[i].inst;
		if (inst < 0)
			SetError("unknown instruction:  " + cmd);
		// prefix
		if (state.ParamSize != state.DefaultSize){
			//buffer[CodeLength ++] = 0x66;
			SetError("prefix unhandled:  " + cmd);
		}
		InstructionWithParams iwp;
		iwp.inst = inst;
		iwp.p1 = p1;
		iwp.p2 = p2;
		iwp.line = current_line;
		iwp.col = current_col;
		add(iwp);


		if (state.EndOfCode)
			break;
	}
}


// convert human readable asm code into opcode
bool Assemble(const char *code, char *oc, int &ocs)
{
	msg_db_f("Assemble", 1+ASM_DB_LEVEL);
	/*if (!Instruction)
		SetInstructionSet(InstructionSetDefault);*/

	InstructionWithParamsList list = InstructionWithParamsList(CurrentMetaInfo->LineOffset);

	list.AppendFromSource(code);

	list.Optimize(oc, ocs);

	// compile commands
	list.Compile(oc, ocs);

	return true;
}

inline bool _size_match_(InstructionParamFuzzy &inst_p, InstructionParam &wanted_p)
{
	if (inst_p.size == wanted_p.size)
		return true;
	if ((inst_p.size == SizeUnknown) || (wanted_p.size == SizeUnknown))
		return true;
	if ((inst_p.size == SizeVariable) && ((wanted_p.size == Size16) || (wanted_p.size == Size32)))
		return true;
	return false;
}

inline bool _deref_match_(InstructionParamFuzzy &inst_p, InstructionParam &wanted_p)
{
	if (wanted_p.deref)
		return (inst_p.allow_memory_address) || (inst_p.allow_memory_indirect);
	return true;
}

inline bool _test_param_(InstructionParamFuzzy &inst_p, InstructionParam &wanted_p)
{
	//ParamFuzzyOut(&inst_p);
	
	// none
	if ((wanted_p.type == ParamTNone) || (!inst_p.used))
		return (wanted_p.type == ParamTNone) && (!inst_p.used);

	// immediate
	if (wanted_p.type == ParamTImmediate){
		if ((inst_p.allow_memory_address) && (wanted_p.deref))
			return true;
		if ((inst_p.allow_immediate) && (!wanted_p.deref))
			return true;
		return false;
	}

	// immediate double
	if (wanted_p.type == ParamTImmediateDouble){
		if ((inst_p.allow_immediate) && (inst_p._type_ == ParamTImmediateDouble))
			return true;
	}

	// reg
	if (wanted_p.type == ParamTRegister){
		// direct match
		if ((inst_p.allow_register) && (inst_p.reg)){
			return ((inst_p.reg == wanted_p.reg) && (_deref_match_(inst_p, wanted_p)));
		}
		// fuzzy match
		/*if (inst_p.allow_register){
			msg_write("r2");
			
			return ((inst_p.reg_group == wanted_p.reg->group) && (_size_match_(inst_p, wanted_p)) && (_deref_match_(inst_p, wanted_p)));
		}*/
		// very fuzzy match
		if ((inst_p.allow_register) || (inst_p.allow_memory_indirect)){
			if (wanted_p.deref){
				if (inst_p.allow_memory_indirect)
					return ((inst_p.reg_group == wanted_p.reg->group) && (_deref_match_(inst_p, wanted_p)));
			}else if (inst_p.allow_register)
				return ((inst_p.reg_group == wanted_p.reg->group) && (_size_match_(inst_p, wanted_p))); // FIXME (correct?)
		}
	}

	return false;
}

// translate from easy parameters to assembler usable parameters
InstructionParam _make_param_(int type, long long param)
{
	InstructionParam i;
	i.reg = NULL;
	i.size = SizeUnknown;
	i.deref = false;
	i.value = 0;
	i.disp = DispModeNone;
	i.is_label = false;
	if (type == PKNone){
		i.type = ParamTNone;
	}else if (type == PKConstant8){
		i.type = ParamTImmediate;
		i.size = Size8;
		i.value = param;
	}else if (type == PKConstant16){
		i.type = ParamTImmediate;
		i.size = Size16;
		i.value = param;
	}else if (type == PKConstant32){
		i.type = ParamTImmediate;
		i.size = Size32;
		i.value = param;
	}else if (type == PKLabel){
		i.type = ParamTImmediate;
		i.size = Size32;
		i.value = param;
		i.is_label = true;
	}else if (type == PKDerefConstant){
		i.type = ParamTImmediate;
		i.deref = true;
		i.value = param;
	}else if ((type == PKLocal) || (type == PKEdxRel)){
		i.type = ParamTRegister;
		i.reg = (type == PKLocal) ? RegisterByID[RegEbp] : RegisterByID[RegEdx];
		i.deref = true;
		i.disp = ((param < 120) && (param > -120)) ? DispMode8 : DispMode32;
		i.value = param;
	}else if (type == PKLocal){
		i.type = ParamTRegister;
		i.reg = RegisterByID[RegEsp];
		i.deref = true;
		i.disp = ((param < 120) && (param > -120)) ? DispMode8 : DispMode32;
		i.value = param;
	}else if (type == PKRegister){
		i.type = ParamTRegister;
		i.reg = RegisterByID[(long)param];
		i.size = i.reg->size;
	}else if (type == PKDerefRegister){
		i.type = ParamTRegister;
		i.reg = RegisterByID[(long)param];
		//i.size = i.reg->size; // ???
		i.deref = true;
	}
	return i;
}

inline char CreatePartialModRMByte(InstructionParamFuzzy &pf, InstructionParam &p)
{
	if (pf.mrm_mode == MRMReg){
		if (p.reg == RegisterByID[RegEs])	return 0x00;
		if (p.reg == RegisterByID[RegCs])	return 0x08;
		if (p.reg == RegisterByID[RegSs])	return 0x10;
		if (p.reg == RegisterByID[RegDs])	return 0x18;
		if (p.reg == RegisterByID[RegFs])	return 0x20;
		if (p.reg == RegisterByID[RegGs])	return 0x28;
		if (p.reg == RegisterByID[RegCr0])	return 0x00;
		if (p.reg == RegisterByID[RegCr1])	return 0x08;
		if (p.reg == RegisterByID[RegCr2])	return 0x10;
		if (p.reg == RegisterByID[RegCr3])	return 0x18;
		if ((p.reg == RegisterByID[RegEax]) || (p.reg == RegisterByID[RegAx]) || (p.reg == RegisterByID[RegAl]))	return 0x00;
		if ((p.reg == RegisterByID[RegEcx]) || (p.reg == RegisterByID[RegCx]) || (p.reg == RegisterByID[RegCl]))	return 0x08;
		if ((p.reg == RegisterByID[RegEdx]) || (p.reg == RegisterByID[RegDx]) || (p.reg == RegisterByID[RegDl]))	return 0x10;
		if ((p.reg == RegisterByID[RegEbx]) || (p.reg == RegisterByID[RegBx]) || (p.reg == RegisterByID[RegBl]))	return 0x18;
		if ((p.reg == RegisterByID[RegEsp]) || (p.reg == RegisterByID[RegSp]) || (p.reg == RegisterByID[RegAh]))	return 0x20;
		if ((p.reg == RegisterByID[RegEbp]) || (p.reg == RegisterByID[RegBp]) || (p.reg == RegisterByID[RegCh]))	return 0x28;
		if ((p.reg == RegisterByID[RegEsi]) || (p.reg == RegisterByID[RegSi]) || (p.reg == RegisterByID[RegDh]))	return 0x30;
		if ((p.reg == RegisterByID[RegEdi]) || (p.reg == RegisterByID[RegDi]) || (p.reg == RegisterByID[RegBh]))	return 0x38;
	}else if (pf.mrm_mode == MRMModRM){
		if (p.deref){
			if (state.AddrSize == Size16){
				if ((p.type == ParamTImmediate) && (p.deref))	return 0x06;
			}else{
				if (p.reg == RegisterByID[RegEax])	return (p.disp == DispModeNone) ? 0x00 : ((p.disp == DispMode8) ? 0x40 : 0x80); // default = DispMode32
				if (p.reg == RegisterByID[RegEcx])	return (p.disp == DispModeNone) ? 0x01 : ((p.disp == DispMode8) ? 0x41 : 0x81);
				if (p.reg == RegisterByID[RegEdx])	return (p.disp == DispModeNone) ? 0x02 : ((p.disp == DispMode8) ? 0x42 : 0x82);
				if (p.reg == RegisterByID[RegEbx])	return (p.disp == DispModeNone) ? 0x03 : ((p.disp == DispMode8) ? 0x43 : 0x83);
				// sib			return 4;
				// disp32		return 5;
				if ((p.type == ParamTImmediate) && (p.deref))	return 0x05;
				if (p.reg == RegisterByID[RegEbp])	return (p.disp == DispMode8) ? 0x045 : 0x85;
				if (p.reg == RegisterByID[RegEsi])	return (p.disp == DispModeNone) ? 0x06 : ((p.disp == DispMode8) ? 0x46 : 0x86);
				if (p.reg == RegisterByID[RegEdi])	return (p.disp == DispModeNone) ? 0x07 : ((p.disp == DispMode8) ? 0x47 : 0x87);
			}
		}else{
			if ((p.reg == RegisterByID[RegEax]) || (p.reg == RegisterByID[RegAx]) || (p.reg == RegisterByID[RegAl]))	return 0xc0;
			if ((p.reg == RegisterByID[RegEcx]) || (p.reg == RegisterByID[RegCx]) || (p.reg == RegisterByID[RegCl]))	return 0xc1;
			if ((p.reg == RegisterByID[RegEdx]) || (p.reg == RegisterByID[RegDx]) || (p.reg == RegisterByID[RegDl]))	return 0xc2;
			if ((p.reg == RegisterByID[RegEbx]) || (p.reg == RegisterByID[RegBx]) || (p.reg == RegisterByID[RegBl]))	return 0xc3;
			if ((p.reg == RegisterByID[RegEsp]) || (p.reg == RegisterByID[RegSp]) || (p.reg == RegisterByID[RegAh]))	return 0xc4;
			if ((p.reg == RegisterByID[RegEbp]) || (p.reg == RegisterByID[RegBp]) || (p.reg == RegisterByID[RegCh]))	return 0xc5;
			if ((p.reg == RegisterByID[RegEsi]) || (p.reg == RegisterByID[RegSi]) || (p.reg == RegisterByID[RegDh]))	return 0xc6;
			if ((p.reg == RegisterByID[RegEdi]) || (p.reg == RegisterByID[RegDi]) || (p.reg == RegisterByID[RegBh]))	return 0xc7;
		}
	}
	if (pf.mrm_mode != MRMNone)
		SetError(format("unhandled modrm %d %d %s %d %s", pf.mrm_mode, p.type, (p.reg?p.reg->name.c_str():""), p.deref, SizeOut(pf.size).c_str()));
	return 0x00;
}

char CreateModRMByte(sInstruction &inst, InstructionParam &p1, InstructionParam &p2)
{
	char mrm = CreatePartialModRMByte(inst.param1, p1) | CreatePartialModRMByte(inst.param2, p2);
	if (inst.cap >= 0)
		mrm |= (inst.cap * 8);
	return mrm;
}

void OpcodeAddInstruction(char *oc, int &ocs, sInstruction &inst, InstructionParam &p1, InstructionParam &p2, InstructionWithParamsList &list)
{
	msg_db_f("OpcodeAddInstruction", 1+ASM_DB_LEVEL);

	// add opcode
	*(int*)&oc[ocs] = inst.code;
	ocs += inst.code_size;

	// create mod/rm-byte
	if (inst.has_modrm)
		oc[ocs ++] = CreateModRMByte(inst, p1, p2);

	InstructionParamFuzzy ip1 = inst.param1;
	InstructionParamFuzzy ip2 = inst.param2;
	ApplyParamSize(ip1);
	ApplyParamSize(ip2);

	OCParam = ocs;

	OpcodeAddImmideate(oc, ocs, ip1, p1, inst, list);
	OpcodeAddImmideate(oc, ocs, ip2, p2, inst, list);
}

void InstructionWithParamsList::AddInstruction(char *oc, int &ocs, int n)
{
	msg_db_f("AsmAddInstructionLow", 1+ASM_DB_LEVEL);

	int ocs0 = ocs;
	InstructionWithParams &iwp = (*this)[n];
	current_inst = n;

	// test if any instruction matches our wishes
	int ninst = -1;
	bool has_mod_rm = false;
	for (int i=0;i<Instruction.num;i++)
		if (Instruction[i].inst == iwp.inst){
			InstructionParamFuzzy ip1 = Instruction[i].param1;
			InstructionParamFuzzy ip2 = Instruction[i].param2;
			ApplyParamSize(ip1);
			ApplyParamSize(ip2);
			if ((_test_param_(ip1, iwp.p1)) && (_test_param_(ip2, iwp.p2))){

				if (((!Instruction[i].has_modrm) && (has_mod_rm)) || (ninst < 0)){
					has_mod_rm = Instruction[i].has_modrm;
					ninst = i;
				}
				
				if (DebugAsm)
					Instruction[i].print();
			}
		}

	// compile
	if (ninst >= 0){
		OpcodeAddInstruction(oc, ocs, Instruction[ninst], iwp.p1, iwp.p2, *this);
		iwp.size = ocs - ocs0;
	}else{
		for (int i=0;i<NUM_INSTRUCTION_NAMES;i++)
			if (InstructionNames[i].inst == iwp.inst){
				SetError("command not compatible with its parameters\n" + InstructionNames[i].name + " " + iwp.p1.str() + " " + iwp.p2.str());
			}
		state.LineNo = iwp.line;
		SetError(format("instruction unknown: %d", iwp.inst));
	}
	//msg_write(d2h(&oc[ocs0], ocs - ocs0, false));
}

void InstructionWithParamsList::ShrinkJumps(void *oc, int ocs)
{
	int _ocs = ocs;
	Compile(oc, _ocs);

	foreachi(InstructionWithParams &iwp, *this, i){
		if ((iwp.inst == inst_jmp) || (iwp.inst == inst_jz) || (iwp.inst == inst_jnz)){
			if (iwp.p1.is_label){
				int target = label[iwp.p1.value].InstNo;
				int dist = 0;
				for (int j=i+1;j<target;j++)
					dist += (*this)[j].size;
				for (int j=target;j<=i;j++)
					dist += (*this)[j].size;
				//msg_write(format("%d %d   %d", i, target, dist));

				if (dist < 127){
					so("really shrink");
					if (iwp.inst == inst_jmp)	iwp.inst = inst_jmp_b;
					if (iwp.inst == inst_jz)	iwp.inst = inst_jz_b;
					if (iwp.inst == inst_jnz)	iwp.inst = inst_jnz_b;
					iwp.p1.size = Size8;
				}
			}
		}
	}
}

void InstructionWithParamsList::Optimize(void *oc, int ocs)
{
	ShrinkJumps(oc, ocs);
}

void InstructionWithParamsList::Compile(void *oc, int &ocs)
{
	for (int i=0;i<num+1;i++){
		// defining a label?
		for (int j=0;j<label.num;j++)
			if (i == label[j].InstNo){
				so("defining found: " + label[j].Name);
				label[j].Value = (long)oc + ocs;
			}
		if (i >= num)
			break;

		// opcode
		AddInstruction((char*)oc, ocs, i);
	}

	LinkWantedLabels(oc);

	if (wanted_label.num > 0){
		state.LineNo = (*this)[wanted_label[0].InstNo].line;
		state.ColumnNo = (*this)[wanted_label[0].InstNo].col;
		SetError("undeclared label used: " + wanted_label[0].Name);
	}
}

// only used for error messages
void param2str(string &str, int type, void *param)
{
	switch(type){
		case PKNone:
			str = "---";
			break;
		case PKRegister:
			str = "reg ";
			if (((long)param >= 0) && ((long)param < RegisterByID.num)){
				if (RegisterByID[(long)param])
					str += RegisterByID[(long)param]->name;
				else
					str += "-----evil----";
			}else
				str += "-----evil----";
			break;
		case PKDerefRegister:
			str = "deref reg ";
			if (((long)param >= 0) && ((long)param < RegisterByID.num)){
				if (RegisterByID[(long)param])
					str += RegisterByID[(long)param]->name;
				else
					str += "-----evil----";
			}else
				str += "-----evil----";
			break;
		case PKLocal:
			str = "local " + d2h(&param, 4);
			break;
		case PKEdxRel:
			str = "edx rel " + d2h(&param, 4);
			break;
		case PKConstant32:
			str = "const32 " + d2h(&param, 4);
			break;
		case PKConstant16:
			str = "const16 " + d2h(&param, 2);
			break;
		case PKConstant8:
			str = "const8 " + d2h(&param, 1);
			break;
		case PKConstantDouble:
			str = "const 2x " + d2h(&param, 6);
			break;
		case PKDerefConstant:
			str = "deref const [" + d2h(&param, 4) + "]";
			break;
		default:
			str = format("??? (%d)", type);
			break;
	}
}

void AddInstruction(char *oc, int &ocs, int inst, int param1_type, void *param1, int param2_type, void *param2)
{
	msg_db_f("AsmAddInstruction", 1+ASM_DB_LEVEL);
	/*if (!Instruction)
		SetInstructionSet(InstructionSetDefault);*/
	state.DefaultSize = Size32;
	state.reset();
	/*msg_write("--------");
	for (int i=0;i<NUM_INSTRUCTION_NAMES;i++)
		if (InstructionName[i].inst == inst)
			printf("%s\n", InstructionName[i].name);*/

	InstructionParam wp1 = _make_param_(param1_type, (long)param1);
	InstructionParam wp2 = _make_param_(param2_type, (long)param2);

	OCParam = ocs;
	InstructionWithParamsList list = InstructionWithParamsList(0);
	InstructionWithParams iwp;
	iwp.inst = inst;
	iwp.p1 = wp1;
	iwp.p2 = wp2;
	iwp.line = -1;
	list.add(iwp);
	list.AddInstruction(oc, ocs, 0);
}

bool ImmediateAllowed(int inst)
{
	for (int i=0;i<Instruction.num;i++)
		if (Instruction[i].inst == inst)
			if ((Instruction[i].param1.allow_immediate) || (Instruction[i].param2.allow_immediate))
				return true;
	return false;
}

};
