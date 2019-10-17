/*
 * AutoComplete.cpp
 *
 *  Created on: 02.08.2018
 *      Author: michi
 */

#include "AutoComplete.h"
#include "lib/kaba/kaba.h"
#include <stdio.h>


void AutoComplete::Data::add(const string &name, const string &context)
{
	AutoComplete::Item i;
	i.name = name;
	i.context = context;
	suggestions.add(i);
}

void AutoComplete::Data::append(const AutoComplete::Data &d)
{
	for (auto &s: d.suggestions)
		suggestions.add(s);
}


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
		more_to_parse = syntax->parse_function_command(f, this_line);
	}
}

AutoComplete::Data find_top_level_from_class(const Class *t, const string &yyy)
{
	if (t->is_pointer())
		t = t->parent;
	AutoComplete::Data suggestions;
	for (auto &e: t->elements)
		if (e.name.head(yyy.num) == yyy)
			suggestions.add(e.name, e.type->name + " " + t->name + "." + e.name);
	for (auto *f: t->functions)
		if (f->name.head(yyy.num) == yyy)
			suggestions.add(f->name, f->signature());
	for (auto *c: t->classes)
		if (c->name.find("[") < 0 and c->name.find("*") < 0)
			if (c->name.head(yyy.num) == yyy)
				suggestions.add(c->name, "class " + c->long_name());
	for (auto *c: t->constants)
		if (c->name.head(yyy.num) == yyy)
			suggestions.add(c->name, "const " + c->type->long_name() + " " + c->name);
	return suggestions;
}

AutoComplete::Data find_top_level(SyntaxTree *syntax, Function *f, const string &yyy)
{
	AutoComplete::Data suggestions;
	
	// general expressions
	Array<string> expressions = {"class", "extends", "while", "virtual", "extern", "override", "enum", "and", "or", "while", "for", "if", "else", "const", "new", "delete", "break", "continue", "return", "pass"};
	for (string &e: expressions)
		if (yyy == e.head(yyy.num))
			suggestions.add(e, e);
			
	// function local
	if (f){
		for (auto *v: f->var)
			if (yyy == v->name.head(yyy.num))
				suggestions.add(v->name, v->type->name + " " + v->name);
		if (f->name_space)
			suggestions.append(find_top_level_from_class(f->name_space, yyy));
	}
	
	// global var
	for (auto *v: syntax->root_of_all_evil->var)
		if (yyy == v->name.head(yyy.num))
			suggestions.add(v->name, v->type->name + " " + v->name);
	if (f){
		for (auto *f: syntax->base_class->functions)
			if (f->name.find(".") < 0)
				if (yyy == f->name.head(yyy.num))
					suggestions.add(f->name, f->signature());
	}
	
	
	suggestions.append(find_top_level_from_class(syntax->base_class, yyy));
	
	for (auto *i: syntax->includes)
		suggestions.append(find_top_level_from_class(i->syntax->base_class, yyy));
	return suggestions;
}


// get deep "tail" block
Block* guess_block(SyntaxTree *syntax, Function *f)
{
	Block *b = f->block;
	/*while (true){
		Block *b_next = nullptr;
		for (auto *n: b->nodes){
			if (n->kind == NodeKind::BLOCK)
				b_next = n->as_block();
		}
		if (b_next)
			b = b_next;
		else
			break;
	}*/
	return b;
	//syntax->blocks.back()
}

const Kaba::Class *simplify_type(const Kaba::Class *c) {
	if (c->is_pointer())
		return c->parent;
	return c;
}

const Kaba::Class *node_namespace(Kaba::Node *n) {
	if (n->kind == NodeKind::CLASS)
		return n->as_class();
	return simplify_type(n->type);
}

AutoComplete::Data simple_parse(SyntaxTree *syntax, Function *f, const string &cur_line)
{
	AutoComplete::Data data;
	Array<string> ops = {"+", "-", "*", "/", "=", "/=", "*=", "+=", "&", "%", "and", "or", "!", "(", ")", ",", "\t"};
	string xx = cur_line;
	for (string &o: ops)
		xx = xx.replace(o, " ");
	xx = xx.explode(" ").back();
	//printf("-->>>>>  %s\n", xx.c_str());
	if (xx.num == 0)
		return data;
	auto yy = xx.explode(".");
	//printf("yy=%s\n", sa2s(yy).c_str());
	data.offset = yy.back().num;
	if (yy.num == 1){
		data.append(find_top_level(syntax, f, yy[0]));
	}else{
		if (!f)
			f = syntax->root_of_all_evil;

		//printf("first:  %s\n", yy[0].c_str());
	//	if (syntax->blocks.num == 0)
	//		return data;
		// FIXME?

		// base layer
		Array<const Class*> types;
		auto nodes = syntax->get_existence(yy[0], guess_block(syntax, f), syntax->base_class, false);
		//printf("res: %d\n", nodes.num);
		for (auto *n: nodes){
			//printf("%s\n", n.type->name.c_str());
			types.add(node_namespace(n));
		}

		// middle layers
		for (int i=1; i<yy.num-1; i++){
			Array<const Class*> types2;
			for (auto *t: types){
				for (auto &e: t->elements)
					if (e.name == yy[i])
						types2.add(simplify_type(e.type));
				for (auto *f: t->functions)
					if (f->name == yy[i])
						types2.add(simplify_type(f->literal_return_type));
				for (auto *c: t->constants)
					if (c->name == yy[i])
						types2.add(simplify_type(c->type));
				for (auto *c: t->classes)
					if (c->name == yy[i])
						types2.add(simplify_type(c));
			}
			types = types2;
		}

		// top layer
		string yyy = yy.back();
		for (auto *t: types){
			//printf("type %s\n", t->name.c_str());
			data.append(find_top_level_from_class(t, yyy));
		}
	}

	return data;
}

}



AutoComplete::Data AutoComplete::run(const string& _code, int line, int pos)
{
	auto *s = new Kaba::Script;
	auto ll = _code.explode("\n");
	auto lines_pre = ll.sub(0, line);//+1);
	auto lines_post = ll.sub(line+1, -1);

	string cur_line = ll[line].substr(0, pos);
	//ll2.back() = ll2.back().substr(0, pos);
	string code = implode(lines_pre, "\n") + "\n" + implode(lines_post, "\n");
	//printf("%s\n", code.c_str());
	//printf("---->>  %s\n", cur_line.c_str());
	s->just_analyse = true;
	Data data;
	Kaba::Function *ff = nullptr;

	try{
		//s->syntax->ParseBuffer(code, true);


		//printf("--a\n");
		s->syntax->Exp.analyse(s->syntax, code + string("\0", 1)); // compatibility... expected by lexical

		//printf("--b\n");
		s->syntax->pre_compiler(true);

		//printf("--c\n");
		s->syntax->parse_top_level();

		//printf("--d\n");
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
		data = simple_parse(s->syntax, ff, cur_line);
	}
	//delete(s);
	Kaba::DeleteAllScripts(true, true);

	for (int i=0; i<data.suggestions.num; i++)
		for (int j=i+1; j<data.suggestions.num; j++){
			if (data.suggestions[j].context == data.suggestions[i].context){
				data.suggestions.erase(j);
				j --;
			}else if (data.suggestions[j].name < data.suggestions[i].name)
				data.suggestions.swap(i, j);
		}
	return data;
}
