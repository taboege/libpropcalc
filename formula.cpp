/*
 * formula.cpp - Formula class, parser
 *
 * Copyright (C) 2019 Tobias Boege
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

#include <propcalc/ast.hpp>
#include <propcalc/formula.hpp>

using namespace std;

namespace Propcalc {

enum optype {
	OP_CONST,    /* Constant       */
	OP_VAR,      /* Symbol         */
	OP_NOT,      /* Negation       */
	OP_AND,      /* Conjunction    */
	OP_OR,       /* Disjunction    */
	OP_IMPL,     /* Implication    */
	OP_EQV,      /* Equivalence    */
	OP_XOR,      /* Contravalence  */
	OP_OPAR,     /* Opening paren
	                (internal use) */
};

struct op {
	int prec;
	unsigned int arity;
	optype type;
};

static const struct op OPS[] = {
	/* XXX: No associativity information because this parser has
	 * very limited scope and everything can be supposed to be
	 * right-associative. */
	/* FIXME: Should use ast.hpp enum classes! */
	[OP_CONST] = { .prec = 20, .arity = 0, .type = OP_CONST },
	[OP_VAR]   = { .prec = 20, .arity = 0, .type = OP_VAR   },
	[OP_NOT]   = { .prec = 14, .arity = 1, .type = OP_NOT   },
	[OP_AND]   = { .prec = 12, .arity = 2, .type = OP_AND   },
	[OP_OR]    = { .prec = 10, .arity = 2, .type = OP_OR    },
	[OP_IMPL]  = { .prec =  8, .arity = 2, .type = OP_IMPL  },
	[OP_EQV]   = { .prec =  6, .arity = 2, .type = OP_EQV   },
	[OP_XOR]   = { .prec =  6, .arity = 2, .type = OP_XOR   },
	[OP_OPAR]  = { .prec = -1, .arity = 0, .type = OP_OPAR  },
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
	union {
		unsigned int val;
		unsigned int var;
		optype op;
		struct {
			const char *s;
			size_t len;
		} sym;
	};
};

static int next_token(const char *&s, token& tok) {
	/* Skip whitespace */
	while (isblank(*s))
		s++;

	if (!*s) {
		tok.type = TOK_EOF;
		return 0;
	}

	if (!strncmp(s, "\\T", 2) || !strncmp(s, "\\F", 2)) {
		tok.type = TOK_CONST;
		tok.val = *++s == 'T';
		s++;
		return 1;
	}

	if (*s == '[') {
		tok.type = TOK_VAR;
		tok.sym.s = ++s;
		tok.sym.len = 0;
		while (*s++ != ']') {
			tok.sym.len++;
		}
		return 1;
	}

	if (isalnum(*s)) {
		tok.type = TOK_VAR;
		tok.sym.s = s++;
		tok.sym.len = 1;
		while (isalnum(*s) || *s == '_') {
			s++;
			tok.sym.len++;
		}
		return 1;
	}

	if (*s == '(' || *s == ')') {
		tok.type = TOK_PAREN;
		tok.val = *s == '(';
		s++;
		return 1;
	}

	tok.type = TOK_OP;

	if (*s == '~') {
		tok.op = OP_NOT;
		s++;
	}
	else if (*s == '&') {
		tok.op = OP_AND;
		s++;
	}
	else if (*s == '|') {
		tok.op = OP_OR;
		s++;
	}
	else if (*s == '^') {
		tok.op = OP_XOR;
		s++;
	}
	else if (*s == '>' || !strncmp(s, "->", 2)) {
		tok.op = OP_IMPL;
		s += *s == '>' ? 1 : 2;
	}
	else if (*s == '=' || !strncmp(s, "<->", 3)) {
		tok.op = OP_EQV;
		s += *s == '=' ? 1 : 3;
	}
	else {
		throw "unrecognized token";
	}

	return 1;
}

static void reduce(optype op, deque<shared_ptr<Ast>>& astdq) {
	shared_ptr<Ast> lhs, rhs;

	if (op == OP_OPAR)
		return;

	if (astdq.size() < OPS[op].arity)
		throw "missing operands";
	rhs = astdq.back();
	astdq.pop_back();
	if (OPS[op].arity > 1) {
		lhs = astdq.back();
		astdq.pop_back();
	}

	switch (op) {
	case OP_NOT:
		astdq.push_back(make_shared<Ast::Not>(rhs));
		break;
	case OP_AND:
		astdq.push_back(make_shared<Ast::And>(lhs, rhs));
		break;
	case OP_OR:
		astdq.push_back(make_shared<Ast::Or>(lhs, rhs));
		break;
	case OP_IMPL:
		astdq.push_back(make_shared<Ast::Impl>(lhs, rhs));
		break;
	case OP_EQV:
		astdq.push_back(make_shared<Ast::Eqv>(lhs, rhs));
		break;
	case OP_XOR:
		astdq.push_back(make_shared<Ast::Xor>(lhs, rhs));
		break;
	default:
		throw "unknown operator encountered";
	}
}

shared_ptr<Ast> parse(const char *s, shared_ptr<Domain> domain) {
	deque<shared_ptr<Ast>> astdq;
	stack<optype> ops;

	token tok;
	while (next_token(s, tok)) {
		shared_ptr<Ast> ast;

		switch (tok.type) {
		case TOK_CONST:
			astdq.push_back(make_shared<Ast::Const>(tok.val));
			break;

		case TOK_VAR:
			astdq.push_back(domain->get(string(tok.sym.s, tok.sym.len)));
			break;

		case TOK_OP:
			/* All operators are right-associative */
			while (!ops.empty() && OPS[ops.top()].prec > OPS[tok.op].prec) {
				optype op = ops.top();
				ops.pop();
				reduce(op, astdq);
			}
			ops.push(tok.op);
			break;

		case TOK_PAREN:
			/* opening */
			if (tok.val) {
				ops.push(OP_OPAR);
			}
			/* closing */
			else {
				optype op;
				do {
					if (ops.empty())
						throw "missing opening parenthesis";
					op = ops.top();
					ops.pop();
					reduce(op, astdq);
				}
				while (op != OP_OPAR);
			}
			break;

		default:
			throw "invalid token encountered";
		}
	}

	while (!ops.empty()) {
		optype op = ops.top();
		ops.pop();
		if (op == OP_OPAR)
			throw "missing closing parenthesis";
		reduce(op, astdq);
	}

	switch (astdq.size()) {
	case 0:
		throw "empty formula";
	case 1:
		return astdq.front();
	default:
		throw "excess operands";
	}
}

Formula::Formula(string fm, shared_ptr<Domain> domain) :
	domain(domain),
	root(parse(fm.c_str(), domain))
{ }

Formula::Formula(shared_ptr<Ast> root, shared_ptr<Domain> domain) :
	domain(domain),
	root(root)
{ }

string Formula::to_infix(void) const {
	return root->to_infix();
}

string Formula::to_prefix(void) const {
	return root->to_prefix();
}

string Formula::to_postfix(void) const {
	return root->to_postfix();
}

Formula Formula::operator~(void) {
	return Formula(make_shared<Ast::Not>(root), domain);
}

Formula Formula::operator&(const Formula& rhs) {
	if (domain.get() != rhs.domain.get())
		throw "domains must be equal";
	return Formula(make_shared<Ast::And>(root, rhs.root), domain);
}

Formula Formula::operator|(const Formula& rhs) {
	if (domain.get() != rhs.domain.get())
		throw "domains must be equal";
	return Formula(make_shared<Ast::Or>(root, rhs.root), domain);
}

Formula Formula::operator>>(const Formula& rhs) {
	if (domain.get() != rhs.domain.get())
		throw "domains must be equal";
	return Formula(make_shared<Ast::Impl>(root, rhs.root), domain);
}

Formula Formula::operator==(const Formula& rhs) {
	if (domain.get() != rhs.domain.get())
		throw "domains must be equal";
	return Formula(make_shared<Ast::Eqv>(root, rhs.root), domain);
}

Formula Formula::operator^(const Formula& rhs) {
	if (domain.get() != rhs.domain.get())
		throw "domains must be equal";
	return Formula(make_shared<Ast::Xor>(root, rhs.root), domain);
}

} /* namespace Propcalc */
