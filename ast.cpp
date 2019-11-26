/*
 * ast.cpp - AST classes
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

#include <propcalc/ast.hpp>

using namespace std;

namespace Propcalc {

/*
 * Ast::Not
 */

string Ast::Not::to_infix(void) const {
	string op = rhs->to_infix();
	if (rhs->prec() < this->prec())
		op = "(" + op + ")";
	return "~" + op;
}

string Ast::Not::to_prefix(void) const {
	return "~ " + rhs->to_prefix();
}

string Ast::Not::to_postfix(void) const {
	return rhs->to_postfix() + " ~";
}

/*
 * Ast::And
 */

static inline string _to_infix(string sym, const Ast* lhs, const Ast* infix, const Ast* rhs) {
	string op1 = lhs->to_infix();
	string op2 = rhs->to_infix();

	if (lhs->prec() < infix->prec())
		op1 = "(" + op1 + ")";
	if (rhs->prec() < infix->prec())
		op2 = "(" + op2 + ")";
	return op1 + " " + sym + " " + op2;
}

string Ast::And::to_infix(void) const {
	return _to_infix("&", lhs.get(), this, rhs.get());
}

string Ast::And::to_prefix(void) const {
	return "& " + lhs->to_prefix() + " " + rhs->to_prefix();
}

string Ast::And::to_postfix(void) const {
	return lhs->to_postfix() + " " + rhs->to_postfix() + " &";
}

/*
 * Ast::Or
 */

string Ast::Or::to_infix(void) const {
	return _to_infix("|", lhs.get(), this, rhs.get());
}

string Ast::Or::to_prefix(void) const {
	return "| " + lhs->to_prefix() + " " + rhs->to_prefix();
}

string Ast::Or::to_postfix(void) const {
	return lhs->to_postfix() + " " + rhs->to_postfix() + " |";
}

/*
 * Ast::Impl
 */

string Ast::Impl::to_infix(void) const {
	return _to_infix(">", lhs.get(), this, rhs.get());
}

string Ast::Impl::to_prefix(void) const {
	return "> " + lhs->to_prefix() + " " + rhs->to_prefix();
}

string Ast::Impl::to_postfix(void) const {
	return lhs->to_postfix() + " " + rhs->to_postfix() + " >";
}

/*
 * Ast::Eqv
 */

string Ast::Eqv::to_infix(void) const {
	return _to_infix("=", lhs.get(), this, rhs.get());
}

string Ast::Eqv::to_prefix(void) const {
	return "= " + lhs->to_prefix() + " " + rhs->to_prefix();
}

string Ast::Eqv::to_postfix(void) const {
	return lhs->to_postfix() + " " + rhs->to_postfix() + " =";
}

/*
 * Ast::Xor
 */

string Ast::Xor::to_infix(void) const {
	return _to_infix("^", lhs.get(), this, rhs.get());
}

string Ast::Xor::to_prefix(void) const {
	return "^ " + lhs->to_prefix() + " " + rhs->to_prefix();
}

string Ast::Xor::to_postfix(void) const {
	return lhs->to_postfix() + " " + rhs->to_postfix() + " ^";
}

} /* namespace Propcalc */
