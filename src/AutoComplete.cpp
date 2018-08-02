/*
 * AutoComplete.cpp
 *
 *  Created on: 02.08.2018
 *      Author: michi
 */

#include "AutoComplete.h"
#include "lib/kaba/kaba.h"
#include <stdio.h>

namespace Kaba
{
	extern bool next_extern;
	extern bool next_const;

void _Parser_shallow(Kaba::SyntaxTree *syntax)
{
	syntax->root_of_all_evil.name = "RootOfAllEvil";
	syntax->cur_func = nullptr;

	// syntax analysis

	syntax->ParseAllClassNames();

	syntax->Exp.reset_parser();

	// global definitions (enum, class, variables and functions)
	while (!syntax->Exp.end_of_file()){
		next_extern = false;
		next_const = false;

		// extern?
		if (syntax->Exp.cur == IDENTIFIER_EXTERN){
			next_extern = true;
			syntax->Exp.next();
		}

		// const?
		if (syntax->Exp.cur == IDENTIFIER_CONST){
			next_const = true;
			syntax->Exp.next();
		}


		/*if ((Exp.cur == "import") or (Exp.cur == "use")){
			ParseImport();

		// enum
		}else*/ if (syntax->Exp.cur == IDENTIFIER_ENUM){
			syntax->ParseEnum();

		// class
		}else if (syntax->Exp.cur == IDENTIFIER_CLASS){
			syntax->ParseClass();

		}else{

			// type of definition
			bool is_function = false;
			for (int j=1;j<syntax->Exp.cur_line->exp.num-1;j++)
				if (syntax->Exp.cur_line->exp[j].name == "(")
				    is_function = true;

			// function?
			if (is_function){
				syntax->ParseFunctionHeader(nullptr, next_extern);
				syntax->SkipParsingFunctionBody();

			// global variables
			}else{
				syntax->ParseVariableDef(false, syntax->root_of_all_evil.block);
			}
		}
		if (!syntax->Exp.end_of_file())
			syntax->Exp.next_line();
	}
}



void _ParseFunctionBody(SyntaxTree *syntax, Function *f)
{
	syntax->Exp.cur_line = &syntax->Exp.line[f->_logical_line_no];

	ExpressionBuffer::Line *this_line = syntax->Exp.cur_line;
	bool more_to_parse = true;

	syntax->parser_loop_depth = 0;

// instructions
	while(more_to_parse){
		more_to_parse = syntax->ParseFunctionCommand(f, this_line);
	}
}

Array<string> find_top_level(SyntaxTree *syntax, Function *f, const string &yyy)
{
	Array<string> suggestions;
	if (f){
		for (auto &v: f->var)
			if (yyy == v.name.head(yyy.num))
				suggestions.add(v.name);
	}
	for (auto &v: syntax->root_of_all_evil.var)
		if (yyy == v.name.head(yyy.num))
			suggestions.add(v.name);
	if (f){
		for (auto *f: syntax->functions)
			if (f->name.find(".") < 0)
				if (yyy == f->name.head(yyy.num))
					suggestions.add(f->name);
	}
	for (auto *i: syntax->includes)
		for (auto *f: i->syntax->functions)
			if (f->name.find(".") < 0)
				if (yyy == f->name.head(yyy.num))
					suggestions.add(f->name);
	return suggestions;
}

AutoComplete::Data simple_parse(SyntaxTree *syntax, Function *f, const string &cur_line)
{
	AutoComplete::Data data;
	Array<string> ops = {"+", "-", "*", "/", "=", "/=", "*=", "+=", "(", ")", ",", "\t"};
	string xx = cur_line;
	for (string &o: ops)
		xx = cur_line.replace(o, " ");
	xx = xx.explode(" ").back();
	//printf("-->>>>>  %s\n", xx.c_str());
	auto yy = xx.explode(".");
	data.offset = yy.back().num;
	if (yy.num == 1){
		data.suggestions = find_top_level(syntax, f, yy[0]);
	}else{

	}

	return data;
}

}



AutoComplete::Data AutoComplete::run(const string& _code, int line, int pos)
{
	auto *s = new Kaba::Script;
	auto ll = _code.explode("\n");
	auto ll2 = ll.sub(0, line);//+1);

	string cur_line = ll[line].substr(0, pos);
	//ll2.back() = ll2.back().substr(0, pos);
	string code = implode(ll2, "\n");
	//printf("%s\n", code.c_str());
	//printf("---->>  %s\n", cur_line.c_str());
	s->just_analyse = true;
	Data data;
	try{
		//s->syntax->ParseBuffer(code, true);


		//printf("--a\n");
		s->syntax->Exp.Analyse(s->syntax, code + string("\0", 1)); // compatibility... expected by lexical

		//printf("--b\n");
		s->syntax->PreCompiler(true);

		//printf("--c\n");
		_Parser_shallow(s->syntax);

		//printf("--d\n");
		Kaba::Function *ff = NULL;
		for (auto *f: s->syntax->functions){
			if ((!f->is_extern) and (f->_logical_line_no >= 0) and (f->_logical_line_no < line))
				ff = f;
		}
		if (ff){
	//		printf("func: %s\n", ff->name.c_str());
			_ParseFunctionBody(s->syntax, ff);
		}

		data = simple_parse(s->syntax, ff, cur_line);



	}catch(const Kaba::Exception &e){
		printf("err: %s\n", e.message().c_str());
		//if (e.line)
		//throw e;
		data = simple_parse(s->syntax, NULL, cur_line);
	}
	//delete(s);
	Kaba::DeleteAllScripts(true, true);
	return data;
}
