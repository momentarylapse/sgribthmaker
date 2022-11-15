#include "Context.h"
#include "kaba.h"

namespace kaba {


Exception::Exception(const string &_message, const string &_expression, int _line, int _column, Module *s) :
	Asm::Exception(_message, _expression, _line, _column)
{
	filename = s->filename;
}

Exception::Exception(const Asm::Exception &e, Module *s, Function *f) :
	Asm::Exception(e)
{
	filename = s->filename;
	text = format("assembler: %s, %s", message(), f->long_name());
}

string Exception::message() const {
	return format("%s, %s", Asm::Exception::message(), filename);
}




}