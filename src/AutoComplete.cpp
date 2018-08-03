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

Array<string> find_top_level_from_class(Class *t, const string &yyy)
{
	if (t->is_pointer)
		t = t->parent;
	Array<string> suggestions;
	for (auto &e: t->elements)
		if (e.name.head(yyy.num) == yyy)
			suggestions.add(e.name);
	for (auto &f: t->functions)
		if (f.name.head(yyy.num) == yyy)
			suggestions.add(f.name);
	return suggestions;
}

Array<string> find_top_level(SyntaxTree *syntax, Function *f, const string &yyy)
{
	Array<string> suggestions;
	Array<string> expressions = {"class", "extends", "while", "virtual", "extern", "override", "enum", "and", "or", "while", "for", "if", "else", "const", "new", "delete", "break", "continue", "return"};
	for (string &e: expressions)
		if (yyy == e.head(yyy.num))
			suggestions.add(e);
	if (f){
		for (auto &v: f->var)
			if (yyy == v.name.head(yyy.num))
				suggestions.add(v.name);
		if (f->_class)
			suggestions.append(find_top_level_from_class(f->_class, yyy));
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
	for (auto *c: syntax->constants)
		if (yyy == c->name.head(yyy.num))
			suggestions.add(c->name);
	for (auto *t: syntax->classes)
		if (t->name.find("[") < 0 and t->name.find("*") < 0)
		if (yyy == t->name.head(yyy.num))
			suggestions.add(t->name);
	for (auto *i: syntax->includes){
		for (auto *f: i->syntax->functions)
			if (f->name.find(".") < 0)
				if (yyy == f->name.head(yyy.num))
					suggestions.add(f->name);
		for (auto *t: i->syntax->classes)
			if (t->name.find("[") < 0 and t->name.find("*") < 0)
			if (yyy == t->name.head(yyy.num))
				suggestions.add(t->name);
		for (auto *c: i->syntax->constants)
			if (yyy == c->name.head(yyy.num))
				suggestions.add(c->name);
	}
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
	if (xx.num == 0)
		return data;
	auto yy = xx.explode(".");
	//printf("yy=%s\n", sa2s(yy).c_str());
	data.offset = yy.back().num;
	if (yy.num == 1){
		data.suggestions = find_top_level(syntax, f, yy[0]);
	}else{
		if (!f)
			f = &syntax->root_of_all_evil;

		//printf("first:  %s\n", yy[0].c_str());
		if (syntax->blocks.num == 0)
			return data;

		// base layer
		Array<Class*> types;
		auto nodes = syntax->GetExistence(yy[0], syntax->blocks.back());
		//printf("res: %d\n", nodes.num);
		for (auto &n: nodes){
		//	printf("%s\n", n.type->name.c_str());
			types.add(n.type);
		}

		// middle layers
		for (int i=1; i<yy.num-1; i++){
			Array<Class*> types2;
			for (Class *t: types){
				for (auto &e: t->elements)
					if (e.name == yy[i])
						types2.add(e.type);
				for (auto &f: t->functions)
					if (f.name == yy[i])
						types2.add(f.return_type);
			}
			types = types2;
		}

		// top layer
		string yyy = yy.back();
		for (auto *t: types){
			data.suggestions.append(find_top_level_from_class(t, yyy));
		}
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
		s->syntax->ParseTopLevel();

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

	for (int i=0; i<data.suggestions.num; i++)
		for (int j=i+1; j<data.suggestions.num; j++){
			if (data.suggestions[j] == data.suggestions[i]){
				data.suggestions.erase(j);
				j --;
			}else if (data.suggestions[j] < data.suggestions[i])
				data.suggestions.swap(i, j);
		}
	return data;
}
