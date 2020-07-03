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
#include <cassert>

#include <string>
#include <memory>
#include <deque>
#include <stack>
#include <algorithm>
#include <unordered_set>
#include <set>

#include <propcalc/formula.hpp>
#include <propcalc/truthtable.hpp>
#include <propcalc/cnf.hpp>

using namespace std;

namespace Propcalc {

/* Initialize the Formula ctor's default Domain. */
Cache Formula::DefaultDomain;

/**
 * Types of operator nodes. These values are only used in the short
 * period between parsing an operator token and creating its AST node.
 * AST nodes have a 1-to-1-corresponding `enum Ast::Type` which is
 * part of the public interface as a clean enum class. Here, we want
 * to treat them as indices into the OPS table below.
 */
enum optype {
	OP_CONST,    /**< Constant       */
	OP_VAR,      /**< Symbol         */
	OP_NOT,      /**< Negation       */
	OP_AND,      /**< Conjunction    */
	OP_OR,       /**< Disjunction    */
	OP_IMPL,     /**< Implication    */
	OP_EQV,      /**< Equivalence    */
	OP_XOR,      /**< Contravalence  */
};

/**
 * Operator table. This data structure is consulted by the parser
 * to convert infix to AST representation.
 */
static const Ast OPS[] = {
	[OP_CONST] = { Ast::Type::Const, Ast::Arity::Leaf,   Ast::Prec::Symbolic, Ast::Assoc::Non,   false },
	[OP_VAR]   = { Ast::Type::Var,   Ast::Arity::Leaf,   Ast::Prec::Symbolic, Ast::Assoc::Non,   false },
	[OP_NOT]   = { Ast::Type::Not,   Ast::Arity::Unary,  Ast::Prec::Notish,   Ast::Assoc::Non,   false },
	[OP_AND]   = { Ast::Type::And,   Ast::Arity::Binary, Ast::Prec::Andish,   Ast::Assoc::Both,  false },
	[OP_OR]    = { Ast::Type::Or,    Ast::Arity::Binary, Ast::Prec::Orish,    Ast::Assoc::Both,  false },
	[OP_IMPL]  = { Ast::Type::Impl,  Ast::Arity::Binary, Ast::Prec::Implish,  Ast::Assoc::Right, false },
	[OP_EQV]   = { Ast::Type::Eqv,   Ast::Arity::Binary, Ast::Prec::Eqvish,   Ast::Assoc::Both,  false },
	[OP_XOR]   = { Ast::Type::Xor,   Ast::Arity::Binary, Ast::Prec::Xorish,   Ast::Assoc::Both,  false },
};

/**
 * Token type as returned by `next_token`.
 */
enum toktype {
	TOK_EOF = 0, /**< End of file    */
	TOK_CONST,   /**< Constant       */
	TOK_VAR,     /**< A [...] symbol */
	TOK_OP,      /**< An operator    */
	TOK_PAREN,   /**< Parenthesis    */
};

/**
 * Description of a parsed token, including token type, the starting
 * offset in the formula and type-specific data.
 */
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

/**
 * The parser always expects a term or an infix. The current state
 * is described by a variable of this type.
 */
enum expect {
	EXPECT_TERM,
	EXPECT_INFIX,
};

/**
 * Scan for the next token starting at `s[i]` which is returned in the
 * `tok` reference. The unsigned int reference `i` is updated to point
 * just after the last character of the read token (unless it was EOF).
 *
 * Whitespace is skipped. Unrecognized characters throw an exception
 * of type Propcalc::X::Formula::Parser.
 */
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
	return tok.type == TOK_OP && OPS[tok.op].arity == Ast::Arity::Unary;
}

static bool is_binary(const token& tok) {
	return tok.type == TOK_OP && OPS[tok.op].arity == Ast::Arity::Binary;
}

/**
 * Reduction step in the parser. Take an operator (token) `tok` and make
 * a new subformula from it with operands at the top of `astdq` and then
 * push the subformula to that deque. The elements of `astdq` at any time
 * are preorder traversals of some subtree of the formula's AST.
 *
 * Throws a Propcalc::X::Formula::Parser exception if not enough operands
 * are available.
 */
static void reduce(token tok, deque<pair<vector<Ast>, token>>& astdq) {
	vector<Ast> lhs, rhs;

	if (is_opening_paren(tok))
		return;

	if (astdq.size() < (size_t) OPS[tok.op].arity)
		throw X::Formula::Parser("Missing operands", tok.offset);
	rhs = astdq.back().first;
	astdq.pop_back();
	if (is_binary(tok)) {
		lhs = astdq.back().first;
		astdq.pop_back();
	}

	vector<Ast> op;
	op.reserve(1 + lhs.size() + rhs.size());
	op.push_back(OPS[tok.op]);
	op.insert(op.end(), lhs.begin(), lhs.end());
	op.insert(op.end(), rhs.begin(), rhs.end());
	astdq.push_back({ op, tok });
}

/**
 * Parse a given formula string into a preorder traversal of its AST,
 * resolving variables with the given `domain`.
 */
vector<Ast> parse(const char* s, Domain* domain) {
	deque<pair<vector<Ast>, token>> astdq;
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
		vector<Ast> ast;
		Ast cv; /* new constant or variable */

		switch (tok.type) {
		case TOK_CONST:
			check_expect(EXPECT_TERM);
			cv = OPS[OP_CONST];
			cv.value = tok.val;
			astdq.push_back({ { cv }, tok });
			toggle_expect();
			break;

		case TOK_VAR:
			check_expect(EXPECT_TERM);
			cv = OPS[OP_VAR];
			cv.var = domain->pack(domain->resolve(string(tok.sym.s, tok.sym.len)));
			astdq.push_back({ { cv }, tok });
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
				/* A closing paren terminates a term, always, so we better
				 * not still be expecting one. */
				if (next == EXPECT_TERM)
					throw X::Formula::Parser("Term expected when encountering closing parenthesis", tok.offset);

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

	/* The parser cannot stop expecting a term. This ensures that the
	 * formula is non-empty and it ensures that each infix has a RHS
	 * operand, so that they don't steal other operators' operands in
	 * the reduce loop and mess up error reporting. */
	if (next == EXPECT_TERM)
		throw X::Formula::Parser("Term expected but EOF reached", i);

	while (!ops.empty()) {
		token op = ops.top();
		ops.pop();
		if (is_opening_paren(op))
			throw X::Formula::Parser("Missing closing parenthesis", op.offset);
		reduce(op, astdq);
	}

	switch (astdq.size()) {
	case 0:
		throw X::Formula::Parser("No operands left after reduction", i);
	case 1:
		return astdq.front().first;
	default:
		token xs = astdq.front().second;
		throw X::Formula::Parser("Excess operands after reduction", xs.offset);
	}
}

Formula::Formula(const string& fm, Domain* domain) :
	domain(domain),
	pn(parse(fm.c_str(), domain))
{ }

/**
 * Convert a Clause into a Formula. Returns a Formula which is a disjunction
 * of positive or negative variables. If the clause is empty, the formula
 * consists of a single Ast::Const node, which is false (false being the
 * identity element with respect to disjunction).
 */
static vector<Ast> clause_ast(Clause& cl, Domain* domain) {
	vector<Ast> pn;
	auto last_or = pn.begin();
	for (auto& v : cl.vars()) {
		Ast lit = OPS[OP_VAR];
		lit.var = domain->pack(v);
		/* Keep a pointer to the last OR because we have to remove that
		 * in the end. */
		pn.push_back(OPS[OP_OR]);
		last_or = pn.end() - 1;
		if (not cl[v])
			pn.push_back(OPS[OP_NOT]);
		pn.push_back(lit);
	}

	/* Empty clause is false (the identity of disjunction) */
	if (last_or == pn.end()) {
		Ast c = OPS[OP_CONST];
		c.value = false;
		pn.push_back(c);
	}
	/* Otherwise we still have to remove that one disjunction
	 * that was added in the last iteration. */
	else {
		pn.erase(last_or);
	}

	return pn;
}

Formula::Formula(Clause& cl, Domain* domain) : domain(domain) {
	pn = clause_ast(cl, domain);
}

Formula::Formula(Stream<Clause>& clauses, Domain* domain) : domain(domain) {
	auto last_and = pn.begin();
	for (auto cl : clauses) {
		/* Keep a pointer to the last AND because we have to remove that
		 * in the end. */
		pn.push_back(OPS[OP_AND]);
		last_and = pn.end() - 1;
		auto clast = clause_ast(cl, domain);
		pn.insert(pn.end(), clast.begin(), clast.end());
	}

	/* Empty CNF is true (the identity of conjunction) */
	if (last_and == pn.end()) {
		Ast c = OPS[OP_CONST];
		c.value = true;
		pn.push_back(c);
	}
	/* Otherwise we still have to remove that one conjunction
	 * that was added in the last iteration. */
	else {
		pn.erase(last_and);
	}
}

vector<VarNr> Formula::varnrs(void) const {
	vector<VarNr> out;
	set<VarNr> seen;
	for (auto ast : pn) {
		if (ast.type == Ast::Type::Var) {
			auto nr = ast.var;
			bool was_new;
			tie(ignore, was_new) = seen.emplace(nr);
			if (was_new)
				out.push_back(nr);
		}
	}
	return out;
}

vector<VarRef> Formula::vars(void) const {
	auto nrs = varnrs();
	sort(nrs.begin(), nrs.end());

	vector<VarRef> out;
	for (auto nr : nrs)
		out.push_back(domain->unpack(nr));
	return out;
}

bool Formula::eval(const Assignment& assign) const {
	vector<bool> terms;
	/* Reserve heuristic size, probably too much. */
	terms.reserve(pn.size() / 4);

	for (unsigned int i = pn.size(); i-- > 0; ) {
		auto ast = pn[i];
		switch (ast.type) {
			case Ast::Type::Const:
				terms.push_back(ast.value);
				break;
			case Ast::Type::Var:
				terms.push_back(assign[domain->unpack(ast.var)]);
				break;
			case Ast::Type::Not:
				terms.back() = not terms.back();
				break;
			case Ast::Type::And: {
				bool lhs = terms.back();
				terms.pop_back();
				terms.back() = lhs && terms.back();
				break;
			}
			case Ast::Type::Or: {
				bool lhs = terms.back();
				terms.pop_back();
				terms.back() = lhs || terms.back();
				break;
			}
			case Ast::Type::Impl: {
				bool lhs = terms.back();
				terms.pop_back();
				terms.back() = !lhs || terms.back();
				break;
			}
			case Ast::Type::Eqv: {
				bool lhs = terms.back();
				terms.pop_back();
				terms.back() = lhs == terms.back();
				break;
			}
			case Ast::Type::Xor: {
				bool lhs = terms.back();
				terms.pop_back();
				terms.back() = lhs != terms.back();
				break;
			}
		}
	}

	assert(terms.size() == 1);
	return terms[0];
}

bool Formula::EVAL(const Assignment& assign, unsigned int root) const {
	auto it = begin(root);
	auto ast = *it;
	switch (ast.type) {
		case Ast::Type::Const:
			return ast.value;

		case Ast::Type::Var:
			return assign[domain->unpack(ast.var)];

		case Ast::Type::Not:
			return !EVAL(assign, (++it).current);

		case Ast::Type::And: {
			auto ops = it.operands();
			auto il = ops[0].current;
			auto ir = ops[1].current;
			return EVAL(assign, il) && EVAL(assign, ir);
		}

		case Ast::Type::Or: {
			auto ops = it.operands();
			auto il = ops[0].current;
			auto ir = ops[1].current;
			return EVAL(assign, il) || EVAL(assign, ir);
		}

		case Ast::Type::Impl: {
			auto ops = it.operands();
			auto il = ops[0].current;
			auto ir = ops[1].current;
			return !EVAL(assign, il) || EVAL(assign, ir);
		}

		case Ast::Type::Eqv: {
			auto ops = it.operands();
			auto il = ops[0].current;
			auto ir = ops[1].current;
			return EVAL(assign, il) == EVAL(assign, ir);
		}

		case Ast::Type::Xor: {
			auto ops = it.operands();
			auto il = ops[0].current;
			auto ir = ops[1].current;
			return EVAL(assign, il) != EVAL(assign, ir);
		}
	}

	throw "unexpected Ast::Type encountered";
}

static inline Ast make_const(bool value) {
	auto c = OPS[OP_CONST];
	c.value = value;
	return c;
}

static inline vector<Ast> make_unary(optype op, const vector<Ast>& rhs) {
	vector<Ast> v{OPS[op]};
	v.insert(v.end(), rhs.begin(), rhs.end());
	return v;
}

static inline vector<Ast> make_binary(optype op, const vector<Ast>& lhs, const vector<Ast>& rhs) {
	vector<Ast> v{OPS[op]};
	v.insert(v.end(), lhs.begin(), lhs.end());
	v.insert(v.end(), rhs.begin(), rhs.end());
	return v;
}

vector<Ast> Formula::SIMPLIFY(const Assignment& assign, unsigned int root) const {
	auto it = begin(root);
	auto ast = *it;
	switch (ast.type) {
		case Ast::Type::Const:
			return vector{ast};

		case Ast::Type::Var: {
			/* Evaluate the variable if possible. */
			auto ref = domain->unpack(ast.var);
			if (assign.exists(ref))
				return vector{make_const(assign[ref])};
			return vector{ast};
		}

		case Ast::Type::Not: {
			/* Remove double negations. */
			auto rhs = SIMPLIFY(assign, (++it).current);
			auto rhsi = rhs.begin();
			size_t toggles = 1;
			while (rhsi->type == Ast::Type::Not)
				++rhsi;

			/* Reduce Not Const */
			if (rhsi->type == Ast::Type::Const)
				return vector{make_const(!rhsi->value)};

			vector<Ast> op;
			if (toggles % 2 != 0)
				op.push_back(OPS[OP_NOT]);
			op.insert(op.end(), rhsi, rhs.end());
			return op;
		}

		case Ast::Type::And: {
			auto ops = it.operands();
			auto lhs = SIMPLIFY(assign, ops[0].current);
			auto rhs = SIMPLIFY(assign, ops[1].current);

			if (lhs[0].type == Ast::Type::Const)
				return lhs[0].value ? rhs : vector{make_const(false)};
			if (rhs[0].type == Ast::Type::Const)
				return rhs[0].value ? lhs : vector{make_const(false)};

			return make_binary(OP_AND, lhs, rhs);
		}

		case Ast::Type::Or: {
			auto ops = it.operands();
			auto lhs = SIMPLIFY(assign, ops[0].current);
			auto rhs = SIMPLIFY(assign, ops[1].current);

			if (lhs[0].type == Ast::Type::Const)
				return lhs[0].value ? vector{make_const(true)} : rhs;
			if (rhs[0].type == Ast::Type::Const)
				return rhs[0].value ? vector{make_const(true)} : lhs;

			return make_binary(OP_OR, lhs, rhs);
		}

		case Ast::Type::Impl: {
			auto ops = it.operands();
			auto lhs = SIMPLIFY(assign, ops[0].current);
			auto rhs = SIMPLIFY(assign, ops[1].current);

			if (lhs[0].type == Ast::Type::Const)
				return lhs[0].value ? rhs : vector{make_const(true)};
			if (rhs[0].type == Ast::Type::Const)
				return rhs[0].value ? vector{make_const(true)} : SIMPLIFY(assign, make_unary(OP_NOT, lhs));

			return make_binary(OP_IMPL, lhs, rhs);
		}

		case Ast::Type::Eqv: {
			auto ops = it.operands();
			auto lhs = SIMPLIFY(assign, ops[0].current);
			auto rhs = SIMPLIFY(assign, ops[1].current);

			if (lhs[0].type == Ast::Type::Const)
				return lhs[0].value ? rhs : SIMPLIFY(assign, make_unary(OP_NOT, rhs));
			if (rhs[0].type == Ast::Type::Const)
				return rhs[0].value ? lhs : SIMPLIFY(assign, make_unary(OP_NOT, lhs));

			return make_binary(OP_EQV, lhs, rhs);
		}

		case Ast::Type::Xor: {
			auto ops = it.operands();
			auto lhs = SIMPLIFY(assign, ops[0].current);
			auto rhs = SIMPLIFY(assign, ops[1].current);

			if (lhs[0].type == Ast::Type::Const)
				return lhs[0].value ? SIMPLIFY(assign, make_unary(OP_NOT, rhs)) : rhs;
			if (rhs[0].type == Ast::Type::Const)
				return rhs[0].value ? SIMPLIFY(assign, make_unary(OP_NOT, lhs)) : lhs;

			return make_binary(OP_XOR, lhs, rhs);
		}
	}

	throw "unexpected Ast::Type encountered";
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

string Formula::to_prefix(void) const {
	auto it = pn.begin();
	auto end = pn.end();
	auto s = to_string(*it); /* guaranteed valid */
	while (++it != end)
		s += " " + to_string(*it);
	return s;
}

string Formula::to_postfix(void) const {
	vector<string> terms;
	terms.reserve(pn.size() / 4);

	for (unsigned int i = pn.size(); i-- > 0; ) {
		auto ast = pn[i];
		switch (ast.arity) {
			case Ast::Arity::Leaf: {
				terms.push_back(to_string(ast));
				break;
			}
			case Ast::Arity::Unary: {
				terms.back() = terms.back() + " " + to_string(ast);
				break;
			}
			case Ast::Arity::Binary: {
				string lhs = terms.back();
				terms.pop_back();
				terms.back() = lhs + " " + terms.back() + " " + to_string(ast);
				break;
			}
		}
	}

	assert(terms.size() == 1);
	return terms[0];
}

string Formula::to_infix(void) const {
	vector<pair<Ast, string>> terms;
	terms.reserve(pn.size() / 4);

	for (unsigned int i = pn.size(); i-- > 0; ) {
		auto ast = pn[i];
		switch (ast.arity) {
			case Ast::Arity::Leaf: {
				terms.push_back({ ast, to_string(ast) });
				break;
			}
			case Ast::Arity::Unary: {
				auto [opr, rhs] = terms.back();
				if (opr.prec < ast.prec)
					rhs = "(" + rhs + ")";
				terms.back() = make_pair(ast, to_string(ast) + rhs);
				break;
			}
			case Ast::Arity::Binary: {
				auto [opl, lhs] = terms.back();
				terms.pop_back();
				auto [opr, rhs] = terms.back();
				if (opl.prec < ast.prec)
					lhs = "(" + lhs + ")";
				if (opr.prec < ast.prec)
					rhs = "(" + rhs + ")";
				terms.back() = make_pair(ast, lhs + " " + to_string(ast) + " " + rhs);
				break;
			}
		}
	}

	assert(terms.size() == 1);
	return terms[0].second;
}

Formula Formula::operator~(void) {
	return Formula(make_unary(OP_NOT, pn), domain);
}

Formula Formula::operator&(const Formula& rhs) {
	if (domain != rhs.domain)
		throw X::Formula::Connective(Ast::Type::And, domain, rhs.domain);
	return Formula(make_binary(OP_AND, pn, rhs.pn), domain);
}

Formula Formula::operator|(const Formula& rhs) {
	if (domain != rhs.domain)
		throw X::Formula::Connective(Ast::Type::Or, domain, rhs.domain);
	return Formula(make_binary(OP_OR, pn, rhs.pn), domain);
}

#if 0
Formula Formula::operator>>(const Formula& rhs) {
	if (domain != rhs.domain)
		throw X::Formula::Connective(Ast::Type::Impl, domain, rhs.domain);
	return Formula(make_binary(OP_IMPL, pn, rhs.pn), domain);
}

Formula Formula::operator==(const Formula& rhs) {
	if (domain != rhs.domain)
		throw X::Formula::Connective(Ast::Type::Eqv, domain, rhs.domain);
	return Formula(make_binary(OP_EQV, pn, rhs.pn), domain);
}
#endif

Formula Formula::operator^(const Formula& rhs) {
	if (domain != rhs.domain)
		throw X::Formula::Connective(Ast::Type::Xor, domain, rhs.domain);
	return Formula(make_binary(OP_XOR, pn, rhs.pn), domain);
}

} /* namespace Propcalc */
