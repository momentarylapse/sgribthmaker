#include "../script.h"
#include "../../file/file.h"

#ifdef OS_LINUX
	#include <sys/mman.h>
	#if (!defined(__x86_64__)) && (!defined(__amd64__))
		#define MAP_32BIT		0
	#endif
#endif
#ifdef OS_WINDOWS
	#include "windows.h"
#endif

namespace Script{


#define so		script_db_out
#define right	script_db_right
#define left	script_db_left


void script_db_out(const string &str); // -> script.cpp
void script_db_out(int i);
void script_db_right();
void script_db_left();


int LocalOffset,LocalOffsetMax;

/*int get_func_temp_size(Function *f)
{
}*/

inline int add_temp_var(int size)
{
	LocalOffset += size;
	if (LocalOffset > LocalOffsetMax)
		LocalOffsetMax = LocalOffset;
	return LocalOffset;
}


int TaskReturnOffset;


#define CallRel32OCSize			5
#define AfterWaitOCSize			10

void AddEspAdd(Asm::InstructionWithParamsList *list,int d)
{
	if (d>0){
		if (d>120)
			list->add_easy(Asm::inst_add, Asm::PKRegister, (void*)Asm::RegEsp, Asm::PKConstant32, (void*)(long)d);
		else
			list->add_easy(Asm::inst_add, Asm::PKRegister, (void*)Asm::RegEsp, Asm::PKConstant8, (void*)(long)d);
	}else if (d<0){
		if (d<-120)
			list->add_easy(Asm::inst_sub, Asm::PKRegister, (void*)Asm::RegEsp, Asm::PKConstant32, (void*)(long)(-d));
		else
			list->add_easy(Asm::inst_sub, Asm::PKRegister, (void*)Asm::RegEsp, Asm::PKConstant8, (void*)(long)(-d));
	}
}

void init_all_global_objects(SyntaxTree *ps, Function *f, Array<char*> &g_var)
{
	foreachi(LocalVariable &v, f->var, i){
		foreach(ClassFunction &cf, v.type->function){
			typedef void init_func(void *);
			if (cf.name == "__init__"){ // TODO test signature "void __init__()"
				//msg_write("global init: " + v.type->name);
				init_func *ff = NULL;
				if (cf.kind == KindCompilerFunction){
					ff = (init_func*)PreCommands[cf.nr].func;
				}else if (cf.kind == KindFunction){
					ff = (init_func*)v.type->owner->script->func[cf.nr];
				}
				if (ff)
					ff(g_var[i]);
			}
		}
	}
}

void Script::AllocateMemory()
{
	// get memory size needed
	MemorySize = 0;
	for (int i=0;i<syntax->RootOfAllEvil.var.num;i++)
		MemorySize += mem_align(syntax->RootOfAllEvil.var[i].type->size);
	foreachi(Constant &c, syntax->Constants, i){
		int s = c.type->size;
		if (c.type == TypeString){
			// const string -> variable length   (+ super array frame)
			s = strlen(c.data) + 1 + SuperArraySize;
		}
		MemorySize += mem_align(s);
	}
	if (MemorySize > 0)
		Memory = (char*)mmap(0, MemorySize, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS | MAP_EXECUTABLE | MAP_32BIT, 0, 0);
		//Memory = new char[MemorySize];
}

void Script::AllocateStack()
{
	// use your own stack if needed
	//   wait() used -> needs to switch stacks ("tasks")
	Stack = NULL;
	foreach(Command *cmd, syntax->Commands){
		if (cmd->kind == KindCompilerFunction)
			if ((cmd->link_nr == CommandWait) || (cmd->link_nr == CommandWaitRT) || (cmd->link_nr == CommandWaitOneFrame)){
				Stack = new char[StackSize];
				break;
			}
	}
}

void Script::AllocateOpcode()
{
	// allocate some memory for the opcode......    has to be executable!!!   (important on amd64)
#ifdef OS_WINDOWS
	Opcode=(char*)VirtualAlloc(NULL,SCRIPT_MAX_OPCODE,MEM_COMMIT | MEM_RESERVE,PAGE_EXECUTE_READWRITE);
	ThreadOpcode=(char*)VirtualAlloc(NULL,SCRIPT_MAX_THREAD_OPCODE,MEM_COMMIT | MEM_RESERVE,PAGE_EXECUTE_READWRITE);
#else
	Opcode = (char*)mmap(0, SCRIPT_MAX_OPCODE, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED | MAP_ANONYMOUS | MAP_EXECUTABLE | MAP_32BIT, 0, 0);
	ThreadOpcode = (char*)mmap(0, SCRIPT_MAX_THREAD_OPCODE, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED | MAP_ANONYMOUS | MAP_EXECUTABLE | MAP_32BIT, 0, 0);
#endif
	if (((long)Opcode==-1)||((long)ThreadOpcode==-1))
		DoErrorInternal("CScript:  could not allocate executable memory");
	OpcodeSize=0;
	ThreadOpcodeSize=0;
}

void Script::MapConstantsToMemory()
{
	// constants -> Memory
	so("Konstanten");
	cnst.resize(syntax->Constants.num);
	foreachi(Constant &c, syntax->Constants, i){
		cnst[i] = &Memory[MemorySize];
		int s = c.type->size;
		if (c.type == TypeString){
			// const string -> variable length
			s = strlen(syntax->Constants[i].data) + 1;

			*(void**)&Memory[MemorySize] = &Memory[MemorySize + SuperArraySize]; // .data
			*(int*)&Memory[MemorySize + PointerSize    ] = s - 1; // .num
			*(int*)&Memory[MemorySize + PointerSize + 4] = 0; // .reserved
			*(int*)&Memory[MemorySize + PointerSize + 8] = 1; // .item_size
			MemorySize += SuperArraySize;
		}
		memcpy(&Memory[MemorySize], (void*)c.data, s);
		MemorySize += mem_align(s);
	}
}

void Script::MapGlobalVariablesToMemory()
{
	// global variables -> into Memory
	so("glob.Var.");
	g_var.resize(syntax->RootOfAllEvil.var.num);
	for (int i=0;i<syntax->RootOfAllEvil.var.num;i++){
		if (syntax->FlagOverwriteVariablesOffset)
			g_var[i] = (char*)(long)(MemorySize + syntax->VariablesOffset);
		else
			g_var[i] = &Memory[MemorySize];
		so(format("%d: %s", MemorySize, syntax->RootOfAllEvil.var[i].name.c_str()));
		MemorySize += mem_align(syntax->RootOfAllEvil.var[i].type->size);
	}
	memset(Memory, 0, MemorySize); // reset all global variables to 0
}

static int OCORA;
void Script::CompileOsEntryPoint()
{
	int nf=-1;
	foreachi(Function *ff, syntax->Functions, index)
		if (ff->name == "main")
			nf = index;
	// call
	if (nf>=0)
		Asm::AddInstruction(Opcode, OpcodeSize, Asm::inst_call, Asm::PKConstant32, NULL);
	TaskReturnOffset=OpcodeSize;
	OCORA = Asm::OCParam;

	// put strings into Opcode!
	foreachi(Constant &c, syntax->Constants, i){
		if ((syntax->FlagCompileOS) || (c.type == TypeString)){
			int offset = 0;
			if (syntax->AsmMetaInfo)
				offset = syntax->AsmMetaInfo->CodeOrigin;
			cnst[i] = (char*)(long)(OpcodeSize + offset);
			int s = c.type->size;
			if (c.type == TypeString)
				s = strlen(c.data) + 1;
			memcpy(&Opcode[OpcodeSize], (void*)c.data, s);
			OpcodeSize += s;
		}
	}
}

void Script::LinkOsEntryPoint()
{
	int nf=-1;
	foreachi(Function *ff, syntax->Functions, index)
		if (ff->name == "main")
			nf = index;
	if (nf>=0){
		int lll=((long)func[nf]-(long)&Opcode[TaskReturnOffset]);
		if (syntax->FlagCompileInitialRealMode)
			lll+=5;
		else
			lll+=3;
		//printf("insert   %d  an %d\n", lll, OCORA);
		//msg_write(lll);
		//msg_write(d2h(&lll,4,false));
		*(int*)&Opcode[OCORA]=lll;
	}
}

void Script::CompileTaskEntryPoint()
{
	// "stack" usage for waiting:
	//  -4| -8 - ebp (before execution)
	//  -8|-16 - ebp (script continue)
	// -12|-24 - esp (script continue)
	// -16|-32 - eip (script continue)
	// -20|-40 - script stack...

	// call
	void *_main_ = MatchFunction("main", "void", 0);

	if ((!Stack) || (!_main_)){
		first_execution = (t_func*)_main_;
		continue_execution = NULL;
		return;
	}

	Asm::InstructionWithParamsList *list = new Asm::InstructionWithParamsList(0);

	int label_first = list->add_label("_first_execution", true);

	first_execution = (t_func*)&ThreadOpcode[ThreadOpcodeSize];
	// intro
	list->add_easy(Asm::inst_push, Asm::PKRegister, (void*)Asm::RegEbp); // within the actual program
	list->add_easy(Asm::inst_mov, Asm::PKRegister, (void*)Asm::RegEbp, Asm::PKRegister, (void*)Asm::RegEsp);
	list->add_easy(Asm::inst_mov, Asm::PKRegister, (void*)Asm::RegEsp, Asm::PKDerefConstant, (void*)&Stack[StackSize]); // start of the script stack
	list->add_easy(Asm::inst_push, Asm::PKRegister, (void*)Asm::RegEbp); // address of the old stack
	AddEspAdd(list, -12); // space for wait() task data
	list->add_easy(Asm::inst_mov, Asm::PKRegister, (void*)Asm::RegEbp, Asm::PKRegister, (void*)Asm::RegEsp);
	list->add_easy(Asm::inst_mov, Asm::PKRegister, (void*)Asm::RegEax, Asm::PKConstant32, (void*)WaitingModeNone); // "reset"
	list->add_easy(Asm::inst_mov, Asm::PKDerefConstant, (void*)&WaitingMode, Asm::PKRegister, (void*)Asm::RegEax);

	// call main()
	list->add_easy(Asm::inst_call, Asm::PKConstant32, _main_);

	// outro
	AddEspAdd(list, 12); // make space for wait() task data
	list->add_easy(Asm::inst_pop, Asm::PKRegister, (void*)Asm::RegEsp);
	list->add_easy(Asm::inst_mov, Asm::PKRegister, (void*)Asm::RegEbp, Asm::PKRegister, (void*)Asm::RegEsp);
	list->add_easy(Asm::inst_leave);
	list->add_easy(Asm::inst_ret);

	// "task" for execution after some wait()
	int label_cont = list->add_label("_continue_execution", true);

	// Intro
	list->add_easy(Asm::inst_push, Asm::PKRegister, (void*)Asm::RegEbp); // within the external program
	list->add_easy(Asm::inst_mov, Asm::PKRegister, (void*)Asm::RegEbp, Asm::PKRegister, (void*)Asm::RegEsp);
	list->add_easy(Asm::inst_mov, Asm::PKDerefConstant, &Stack[StackSize - 4], Asm::PKRegister, (void*)Asm::RegEbp); // save the external ebp
	list->add_easy(Asm::inst_mov, Asm::PKRegister, (void*)Asm::RegEsp, Asm::PKDerefConstant, &Stack[StackSize - 16]); // to the eIP of the script
	list->add_easy(Asm::inst_pop, Asm::PKRegister, (void*)Asm::RegEax);
	list->add_easy(Asm::inst_add, Asm::PKRegister, (void*)Asm::RegEax, Asm::PKConstant32, (void*)AfterWaitOCSize);
	list->add_easy(Asm::inst_jmp, Asm::PKRegister, (void*)Asm::RegEax);
	//list->add_easy(Asm::inst_leave);
	//list->add_easy(Asm::inst_ret);
	/*OCAddChar(0x90);
	OCAddChar(0x90);
	OCAddChar(0x90);*/

	list->Compile(ThreadOpcode, ThreadOpcodeSize);

	first_execution = (t_func*)(long)list->label[label_first].Value;
	continue_execution = (t_func*)(long)list->label[label_cont].Value;

	delete(list);
}

// generate opcode
void Script::Compiler()
{
	msg_db_f("Compiler",2);

	syntax->MapLocalVariablesToStack();

	syntax->BreakDownComplicatedCommands();
#ifdef ScriptDebug
	syntax->Show();
#endif

	syntax->Simplify();
	syntax->PreProcessor(this);

	syntax->Show();

	AllocateMemory();
	AllocateStack();

	MemorySize = 0;
	MapGlobalVariablesToMemory();
	MapConstantsToMemory();

	AllocateOpcode();


	syntax->PreProcessorAddresses(this);



// compiling an operating system?
//   -> create an entry point for execution... so we can just call Opcode like a function
	if ((syntax->FlagCompileOS)||(syntax->FlagCompileInitialRealMode))
		CompileOsEntryPoint();


// compile functions into Opcode
	so("Funktionen");
	func.resize(syntax->Functions.num);
	foreachi(Function *f, syntax->Functions, i){
		func[i] = (t_func*)&Opcode[OpcodeSize];
		CompileFunction(f, Opcode, OpcodeSize);
	}


// "task" for the first execution of main() -> ThreadOpcode
	if (!syntax->FlagCompileOS)
		CompileTaskEntryPoint();




	if (syntax->FlagCompileOS)
		LinkOsEntryPoint();


	// initialize global super arrays and objects
	init_all_global_objects(syntax, &syntax->RootOfAllEvil, g_var);

	//msg_db_out(1,GetAsm(Opcode,OpcodeSize));

	//_expand(Opcode,OpcodeSize);

	WaitingMode = WaitingModeFirst;

	if (ShowCompilerStats){
		msg_write("--------------------------------");
		msg_write(format("Opcode: %db, Memory: %db",OpcodeSize,MemorySize));
	}
}

};



