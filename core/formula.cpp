/*
 * formula.cpp - Formula class, parser
 *
 * Copyright (C) 2019-2020 Tobias Boege
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the Artistic License 2.0
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * Artistic License 2.0 for more details.
 */

#include <cctype>
#include <cstring>

#include <string>
#include <memory>
#include <deque>
#include <stack>
#include <unordered_set>

#include <propcalc/formula.hpp>
#include <propcalc/truthtable.hpp>
#include <propcalc/cnf.hpp>

using namespace std;

namespace Propcalc {

/* Initialize the Formula ctor's default Domain. */
shared_ptr<Cache> Formula::DefaultDomain = make_shared<Cache>();

enum optype {
	OP_CONST,    /* Constant       */
	OP_VAR,      /* Symbol         */
	OP_NOT,      /* Negation       */
	OP_AND,      /* Conjunction    */
	OP_OR,       /* Disjunction    */
	OP_IMPL,     /* Implication    */
	OP_EQV,      /* Equivalence    */
	OP_XOR,      /* Contravalence  */
};

struct opdesc {
	Ast::Prec prec;
	size_t arity;
};

static const struct opdesc OPS[] = {
	/* XXX: No associativity information because this parser has
	 * very limited scope and everything can be supposed to be
	 * right-associative. */
	[OP_CONST] = { .prec = Ast::Prec::Symbolic, .arity = 0 },
	[OP_VAR]   = { .prec = Ast::Prec::Symbolic, .arity = 0 },
	[OP_NOT]   = { .prec = Ast::Prec::Notish,   .arity = 1 },
	[OP_AND]   = { .prec = Ast::Prec::Andish,   .arity = 2 },
	[OP_OR]    = { .prec = Ast::Prec::Orish,    .arity = 2 },
	[OP_IMPL]  = { .prec = Ast::Prec::Implish,  .arity = 2 },
	[OP_EQV]   = { .prec = Ast::Prec::Eqvish,   .arity = 2 },
	[OP_XOR]   = { .prec = Ast::Prec::Xorish,   .arity = 2 },
};

enum toktype {
	TOK_EOF = 0, /* End of file    */
	TOK_CONST,   /* Constant       */
	TOK_VAR,     /* A [...] symbol */
	TOK_OP,      /* An operator    */
	TOK_PAREN,   /* Parenthesis    */
};

struct token {
	toktype type;
	unsigned int offset;
	union {
		unsigned int val;
		unsigned int var;
		optype op;
		struct {
			const char* s;
			size_t len;
		} sym;
	};
};

enum expect {
	EXPECT_TERM,
	EXPECT_INFIX,
};

static bool next_token(const char* s, unsigned int& i, token& tok) {
	/* Skip whitespace */
	while (isblank(s[i]))
		i++;
	tok.offset = i;

	if (!s[i]) {
		tok.type = TOK_EOF;
		return false;
	}

	if (!strncmp(&s[i], "\\T", 2) || !strncmp(&s[i], "\\F", 2)) {
		tok.type = TOK_CONST;
		tok.val = s[++i] == 'T';
		i++;
		return true;
	}

	if (s[i] == '[') {
		tok.type = TOK_VAR;
		tok.sym.s = &s[++i];
		tok.sym.len = 0;
		while (s[i++] != ']') {
			tok.sym.len++;
		}
		return true;
	}

	if (isalnum(s[i])) {
		tok.type = TOK_VAR;
		tok.sym.s = &s[i++];
		tok.sym.len = 1;
		while (isalnum(s[i]) || s[i] == '_') {
			i++;
			tok.sym.len++;
		}
		return true;
	}

	if (s[i] == '(' || s[i] == ')') {
		tok.type = TOK_PAREN;
		tok.val = s[i++] == '(';
		return true;
	}

	tok.type = TOK_OP;

	if (s[i] == '~') {
		tok.op = OP_NOT;
		i++;
	}
	else if (s[i] == '&') {
		tok.op = OP_AND;
		i++;
	}
	else if (s[i] == '|') {
		tok.op = OP_OR;
		i++;
	}
	else if (s[i] == '^') {
		tok.op = OP_XOR;
		i++;
	}
	else if (s[i] == '>' || !strncmp(&s[i], "->", 2)) {
		tok.op = OP_IMPL;
		i += s[i] == '>' ? 1 : 2;
	}
	else if (s[i] == '=' || !strncmp(&s[i], "<->", 3)) {
		tok.op = OP_EQV;
		i += s[i] == '=' ? 1 : 3;
	}
	else {
		throw X::Formula::Parser("Unrecognized token", tok.offset);
	}

	return true;
}

static bool is_opening_paren(const token& tok) {
	return tok.type == TOK_PAREN && !!tok.val;
}

static bool is_unary(const token& tok) {
	return tok.type == TOK_OP && OPS[tok.op].arity == 1;
}

static bool is_binary(const token& tok) {
	return tok.type == TOK_OP && OPS[tok.op].arity == 2;
}

static void reduce(token tok, deque<pair<shared_ptr<Ast>, token>>& astdq) {
	shared_ptr<Ast> lhs, rhs;

	if (is_opening_paren(tok))
		return;

	if (astdq.size() < OPS[tok.op].arity)
		throw X::Formula::Parser("Missing operands", tok.offset);
	rhs = astdq.back().first;
	astdq.pop_back();
	if (OPS[tok.op].arity > 1) {
		lhs = astdq.back().first;
		astdq.pop_back();
	}

	switch (tok.op) {
	case OP_NOT:
		astdq.push_back({ make_shared<Ast::Not>(rhs), tok });
		break;
	case OP_AND:
		astdq.push_back({ make_shared<Ast::And>(lhs, rhs), tok });
		break;
	case OP_OR:
		astdq.push_back({ make_shared<Ast::Or>(lhs, rhs), tok });
		break;
	case OP_IMPL:
		astdq.push_back({ make_shared<Ast::Impl>(lhs, rhs), tok });
		break;
	case OP_EQV:
		astdq.push_back({ make_shared<Ast::Eqv>(lhs, rhs), tok });
		break;
	case OP_XOR:
		astdq.push_back({ make_shared<Ast::Xor>(lhs, rhs), tok });
		break;
	default:
		throw X::Formula::Parser("Unrecognized operator", tok.offset);
	}
}

shared_ptr<Ast> parse(const char* s, shared_ptr<Domain> domain) {
	deque<pair<shared_ptr<Ast>, token>> astdq;
	stack<token> ops;
	expect next = EXPECT_TERM;
	token tok;

	auto check_expect = [&] (const expect got) -> bool {
		if (next == got)
			return true;

		switch (got) {
		case EXPECT_TERM:
			throw X::Formula::Parser("Infix expected but got term", tok.offset);
			break;

		case EXPECT_INFIX:
			throw X::Formula::Parser("Term expected but got infix", tok.offset);
			break;
		}
		return false;
	};

	auto toggle_expect = [&] {
		switch (next) {
		case EXPECT_TERM:
			next = EXPECT_INFIX;
			break;
		case EXPECT_INFIX:
			next = EXPECT_TERM;
			break;
		}
	};

	unsigned int i = 0;
	while (next_token(s, i, tok)) {
		shared_ptr<Ast> ast;

		switch (tok.type) {
		case TOK_CONST:
			check_expect(EXPECT_TERM);
			astdq.push_back({ make_shared<Ast::Const>(tok.val), tok });
			toggle_expect();
			break;

		case TOK_VAR:
			check_expect(EXPECT_TERM);
			astdq.push_back({ make_shared<Ast::Var>(
				domain->resolve(string(tok.sym.s, tok.sym.len))
			), tok });
			toggle_expect();
			break;

		case TOK_OP:
			check_expect(is_unary(tok) ? EXPECT_TERM : EXPECT_INFIX);
			/* All operators are right-associative */
			while (!ops.empty()) {
				token op = ops.top();
				if (is_opening_paren(op) || OPS[op.op].prec <= OPS[tok.op].prec)
					break;
				ops.pop();
				reduce(op, astdq);
			}
			ops.push(tok);
			if (is_binary(tok))
				toggle_expect();
			break;

		case TOK_PAREN:
			if (is_opening_paren(tok)) {
				check_expect(EXPECT_TERM);
				ops.push(tok);
			}
			else {
				token op;
				do {
					if (ops.empty())
						throw X::Formula::Parser("Missing opening parenthesis", tok.offset);
					op = ops.top();
					ops.pop();
					reduce(op, astdq);
				}
				while (!is_opening_paren(op));
			}
			break;

		default:
			throw X::Formula::Parser("Invalid token", tok.offset);
		}
	}

	while (!ops.empty()) {
		token op = ops.top();
		ops.pop();
		if (is_opening_paren(op))
			throw X::Formula::Parser("Missing closing parenthesis", op.offset);
		reduce(op, astdq);
	}

	switch (astdq.size()) {
	case 0:
		throw X::Formula::Parser("Empty formula", i);
	case 1:
		return astdq.front().first;
	default:
		token xs = astdq.front().second;
		throw X::Formula::Parser("Excess operands after reduction", xs.offset);
	}
}

Formula::Formula(const string& fm, shared_ptr<Domain> domain) :
	domain(domain),
	root(parse(fm.c_str(), domain))
{ }

static shared_ptr<Ast> clause_ast(Clause& cl) {
	vector<shared_ptr<Ast>> lits;
	for (auto& v : cl.vars()) {
		shared_ptr<Ast> astsp = make_shared<Ast::Var>(v);
		if (not cl[v])
			astsp = make_shared<Ast::Not>(astsp);
		lits.push_back(astsp);
	}

	/* Empty clause is false (the identity of disjunction) */
	if (lits.size() == 0)
		return make_shared<Ast::Const>(false);

	auto astsp = lits.back();
	lits.pop_back();
	for (auto it = lits.rbegin(); it != lits.rend(); ++it)
		astsp = make_shared<Ast::Or>(*it, astsp);
	return astsp;
}

Formula::Formula(Clause& cl, shared_ptr<Domain> domain) :
	domain(domain) {
	root = clause_ast(cl);
}

Formula::Formula(Stream<Clause>& clauses, shared_ptr<Domain> domain) :
	domain(domain) {
	vector<shared_ptr<Ast>> cls;
	for (auto cl : clauses)
		cls.push_back(clause_ast(cl));

	/* Empty CNF is true (the identity of conjunction) */
	if (cls.size() == 0) {
		root = make_shared<Ast::Const>(true);
		return;
	}

	auto astsp = cls.back();
	cls.pop_back();
	for (auto it = cls.rbegin(); it != cls.rend(); ++it)
		astsp = make_shared<Ast::And>(*it, astsp);
	root = astsp;
}

vector<VarRef> Formula::vars(void) const {
	unordered_set<const Variable *> pile;
	root->fill_vars(pile);
	return domain->sort(pile);
}

Truthtable Formula::truthtable(void) const {
	return Truthtable(*this);
}

Tseitin Formula::tseitin(void) const {
	return Tseitin(*this);
}

CNF Formula::cnf(void) const {
	return CNF(*this);
}

Formula Formula::operator~(void) {
	return Formula(make_shared<Ast::Not>(root), domain);
}

Formula Formula::operator&(const Formula& rhs) {
	if (domain.get() != rhs.domain.get())
		throw X::Formula::Connective(Ast::Type::And, domain, rhs.domain);
	return Formula(make_shared<Ast::And>(root, rhs.root), domain);
}

Formula Formula::operator|(const Formula& rhs) {
	if (domain.get() != rhs.domain.get())
		throw X::Formula::Connective(Ast::Type::Or, domain, rhs.domain);
	return Formula(make_shared<Ast::Or>(root, rhs.root), domain);
}

Formula Formula::operator>>(const Formula& rhs) {
	if (domain.get() != rhs.domain.get())
		throw X::Formula::Connective(Ast::Type::Impl, domain, rhs.domain);
	return Formula(make_shared<Ast::Impl>(root, rhs.root), domain);
}

Formula Formula::operator==(const Formula& rhs) {
	if (domain.get() != rhs.domain.get())
		throw X::Formula::Connective(Ast::Type::Eqv, domain, rhs.domain);
	return Formula(make_shared<Ast::Eqv>(root, rhs.root), domain);
}

Formula Formula::operator^(const Formula& rhs) {
	if (domain.get() != rhs.domain.get())
		throw X::Formula::Connective(Ast::Type::Xor, domain, rhs.domain);
	return Formula(make_shared<Ast::Xor>(root, rhs.root), domain);
}

} /* namespace Propcalc */
