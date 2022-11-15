/*
 * AutoComplete.cpp
 *
 *  Created on: 02.08.2018
 *      Author: michi
 */

#include "AutoComplete.h"
#include "lib/kaba/kaba.h"
#include "lib/kaba/parser/Parser.h"
#include <stdio.h>

static bool verbose = false;


void AutoComplete::Data::add(const string &name, const string &context) {
	AutoComplete::Item i;
	i.name = name;
	i.context = context;
	suggestions.add(i);
}

void AutoComplete::Data::append(const AutoComplete::Data &d) {
	for (auto &s: d.suggestions)
		suggestions.add(s);
}


namespace kaba {


const kaba::Class *simplify_type(const kaba::Class *c) {
	if (c->is_pointer())
		return c->param[0];
	return c;
}

const kaba::Class *node_namespace(shared<kaba::Node> n) {
	if (n->kind == NodeKind::CLASS)
		return n->as_class();
	return simplify_type(n->type);
}


void _ParseFunctionBody(SyntaxTree *syntax, Function *f) {
	syntax->parser->Exp.cur_line = syntax->parser->Exp.token_logical_line(f->token_id);

	int indent0 = syntax->parser->Exp.cur_line->indent;
	bool more_to_parse = true;

	syntax->parser->parser_loop_depth = 0;

// instructions
	try {
		while (more_to_parse) {
			more_to_parse = syntax->parser->parse_abstract_function_command(f, indent0);
		}
	} catch (...) {}
}


bool allow(const string &name) {
	if (name.head(1) == "-")
		return false;
	if (name.head(1) == "@")
		return false;
	if (name.find(".") >= 0)
		return false;
	if (name.find("[") >= 0 or name.find("*") >= 0 or name.find("{") >= 0 or name.find("&") >= 0)
		return false;
	return true;
}

AutoComplete::Data find_top_level_from_class(const Class *t, const string &yyy) {
	t = simplify_type(t);
	AutoComplete::Data suggestions;
	for (auto &e: t->elements)
		if (e.name.head(yyy.num) == yyy and allow(e.name))
			suggestions.add(e.name, format("%s.%s: %s", t->name, e.name, e.type->name));
	for (auto f: t->functions)
		if (f->name.head(yyy.num) == yyy and allow(f->name))
			suggestions.add(f->name, f->signature());
	for (auto c: t->classes)
			if (c->name.head(yyy.num) == yyy and allow(c->name))
				suggestions.add(c->name, "class " + c->long_name());
	for (auto c: t->constants)
		if (c->name.head(yyy.num) == yyy and allow(c->name))
			suggestions.add(c->name, format("const %s: %s", c->name, c->type->long_name()));
	return suggestions;
}

AutoComplete::Data find_top_level(SyntaxTree *syntax, Function *f, const string &yyy) {
	AutoComplete::Data suggestions;
	
	// general expressions
	Array<string> expressions = {"class", "func", "extends", "while", "virtual", "extern", "override", "enum", "and", "or", "while", "for", "if", "else", "const", "selfref", "new", "delete", "break", "continue", "return", "pass" "use", "import", "in", "is"};
	for (string &e: expressions)
		if (yyy == e.head(yyy.num))
			suggestions.add(e, e);
			
	// function local
	if (f) {
		for (auto v: f->var)
			if (yyy == v->name.head(yyy.num) and allow(v->name))
				suggestions.add(v->name, v->type->name + " " + v->name);
		if (f->name_space)
			suggestions.append(find_top_level_from_class(f->name_space, yyy));
	}
	
	// global var
	for (auto v: syntax->root_of_all_evil->var)
		if (yyy == v->name.head(yyy.num) and allow(v->name))
			suggestions.add(v->name, v->type->name + " " + v->name);
	if (f) {
		for (auto f: syntax->base_class->functions)
			if (yyy == f->name.head(yyy.num) and allow(f->name))
				suggestions.add(f->name, f->signature());
	}
	
	
	suggestions.append(find_top_level_from_class(syntax->base_class, yyy));
	
	for (auto i: syntax->includes)
		suggestions.append(find_top_level_from_class(i->syntax->base_class, yyy));
	return suggestions;
}


// get deep "tail" block
Block* guess_block(SyntaxTree *syntax, Function *f) {
	Block *b = f->block.get();
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

AutoComplete::Data simple_parse(SyntaxTree *syntax, Function *f, const string &cur_line) {
	AutoComplete::Data data;
	Array<string> ops = {"+", "-", "*", "/", "=", "/=", "*=", "+=", "&", "%", "and", "or", "!", "(", ")", ",", "\t"};
	string xx = cur_line;
	for (string &o: ops)
		xx = xx.replace(o, " ");
	xx = xx.explode(" ").back();
	if (verbose)
		printf("-->>>>>  %s\n", xx.c_str());
	if (xx.num == 0)
		return data;
	auto yy = xx.explode(".");
	if (verbose)
		printf("yy=%s\n", str(yy).c_str());
	data.offset = yy.back().num;
	if (yy.num == 1) {
		data.append(find_top_level(syntax, f, yy[0]));
	} else {
		if (!f)
			f = syntax->root_of_all_evil.get();

		if (verbose)
			printf("first:  %s\n", yy[0].c_str());
	//	if (syntax->blocks.num == 0)
	//		return data;
		// FIXME?

		// base layer
		Array<const Class*> types;
		auto nodes = syntax->get_existence(yy[0], guess_block(syntax, f), syntax->base_class, -1);
		if (verbose)
			printf("res: %d\n", nodes.num);
		for (auto n: nodes) {
			if (verbose)
				printf("%s\n", n->type->name.c_str());
			types.add(node_namespace(n));
		}

		// middle layers
		for (int i=1; i<yy.num-1; i++) {
			Array<const Class*> types2;
			for (auto *t: types){
				for (auto &e: t->elements)
					if (e.name == yy[i])
						types2.add(simplify_type(e.type));
				for (auto f: t->functions)
					if (f->name == yy[i])
						types2.add(simplify_type(f->literal_return_type));
				for (auto c: t->constants)
					if (c->name == yy[i])
						types2.add(simplify_type(c->type.get()));
				for (auto c: weak(t->classes))
					if (c->name == yy[i])
						types2.add(simplify_type(c));
			}
			types = types2;
		}

		// top layer
		string yyy = yy.back();
		for (auto *t: types) {
			if (verbose)
				printf("type %s\n", t->name.c_str());
			data.append(find_top_level_from_class(t, yyy));
		}
	}

	return data;
}

}



AutoComplete::Data AutoComplete::run(const string& _code, const Path &filename, int line, int pos) {
	kaba::Context context;
	auto s = context.create_empty("<auto-complete>");
	s->filename = filename;
	auto ll = _code.explode("\n");
	auto lines_pre = ll.sub_ref(0, line);//+1);
	auto lines_post = ll.sub_ref(line+1);

	string cur_line = ll[line].sub(0, pos);
	//ll2.back() = ll2.back().sub(0, pos);
	string code = implode(lines_pre, "\n") + "\n" + implode(lines_post, "\n");
	//printf("%s\n", code.c_str());
	//printf("---->>  %s\n", cur_line.c_str());
	s->just_analyse = true;
	Data data;
	kaba::Function *ff = nullptr;
	s->syntax->parser = new kaba::Parser(s->syntax);

	try {
		//s->syntax->ParseBuffer(code, true);


		//printf("--a\n");
		s->syntax->parser->Exp.analyse(s->syntax, code + string("\0", 1)); // compatibility... expected by lexical

		//printf("--b\n");
		s->syntax->parser->parse_macros(true);

		//printf("--c\n");
		s->syntax->parser->parse_top_level();



	} catch (const kaba::Exception &e) {
		printf("err: %s\n", e.message().c_str());
		//if (e.line)
		//throw e;
		//data = simple_parse(s->syntax, ff, cur_line);
	}





	//printf("--d\n");
	for (auto *f: s->syntax->functions) {
		int f_line_no = s->syntax->parser->Exp.token_logical_line(f->token_id)->physical_line;
		if (!f->is_extern() and (f_line_no >= 0) and (f_line_no < line))
			ff = f;
	}
	if (ff) {
//		printf("func: %s\n", ff->name.c_str());
		_ParseFunctionBody(s->syntax, ff);
	}

	data = simple_parse(s->syntax, ff, cur_line);




	for (int i=0; i<data.suggestions.num; i++)
		for (int j=i+1; j<data.suggestions.num; j++) {
			if (data.suggestions[j].context == data.suggestions[i].context) {
				data.suggestions.erase(j);
				j --;
			} else if (data.suggestions[j].name < data.suggestions[i].name) {
				data.suggestions.swap(i, j);
			}
		}
	return data;
}
