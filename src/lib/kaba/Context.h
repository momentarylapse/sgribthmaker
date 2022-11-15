/*
 * Context.h
 *
 *  Created on: Nov 15, 2022
 *      Author: michi
 */

#pragma once

#include "../base/base.h"
#include "../os/path.h"
#include "asm/asm.h"

namespace kaba {

class Module;
class Function;

class Exception : public Asm::Exception {
public:
	Exception(const string &message, const string &expression, int line, int column, Module *s);
	Exception(const Asm::Exception &e, Module *s, Function *f);
	string message() const override;
	//Module *module;
	Path filename;
};
/*struct SyntaxException : Exception{};
struct LinkerException : Exception{};
struct LinkerException : Exception{};*/

class Context {

};

extern Context default_context;

}
