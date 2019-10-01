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

Ast::Const::Const(bool value) :
	value(value)
{ }

std::string Ast::Const::to_pn(void) const {
	return value ? "\\T" : "\\F";
}

std::string Ast::Const::to_rpn(void) const {
	return value ? "\\T" : "\\F";
}

Ast::Var::Var(unsigned int nr) :
	nr(nr)
{ }

std::string Ast::Var::to_pn(void) const {
	return std::to_string(nr);
}

std::string Ast::Var::to_rpn(void) const {
	return std::to_string(nr);
}

Ast::Sym::Sym(std::string str) :
	str(str),
	nr(0),
	obj(nullptr)
{ }

Ast::Sym::Sym(const char *s, size_t len) :
	str(string(s, len)),
	nr(0),
	obj(nullptr)
{ }

std::string Ast::Sym::to_pn(void) const {
	return "[" + str + "]";
}

std::string Ast::Sym::to_rpn(void) const {
	return "[" + str + "]";
}

Ast::Not::Not(shared_ptr<Ast> rhs) :
	rhs(rhs)
{ }

std::string Ast::Not::to_pn(void) const {
	return "~ " + rhs->to_pn();
}

std::string Ast::Not::to_rpn(void) const {
	return rhs->to_rpn() + " ~";
}

Ast::And::And(shared_ptr<Ast> lhs, shared_ptr<Ast> rhs) :
	lhs(lhs),
	rhs(rhs)
{ }

std::string Ast::And::to_pn(void) const {
	return "& " + lhs->to_pn() + " " + rhs->to_pn();
}

std::string Ast::And::to_rpn(void) const {
	return lhs->to_rpn() + " " + rhs->to_rpn() + " &";
}

Ast::Or::Or(shared_ptr<Ast> lhs, shared_ptr<Ast> rhs) :
	lhs(lhs),
	rhs(rhs)
{ }

std::string Ast::Or::to_pn(void) const {
	return "| " + lhs->to_pn() + " " + rhs->to_pn();
}

std::string Ast::Or::to_rpn(void) const {
	return lhs->to_rpn() + " " + rhs->to_rpn() + " |";
}

Ast::Impl::Impl(shared_ptr<Ast> lhs, shared_ptr<Ast> rhs) :
	lhs(lhs),
	rhs(rhs)
{ }

std::string Ast::Impl::to_pn(void) const {
	return "> " + lhs->to_pn() + " " + rhs->to_pn();
}

std::string Ast::Impl::to_rpn(void) const {
	return lhs->to_rpn() + " " + rhs->to_rpn() + " >";
}

Ast::Eqv::Eqv(shared_ptr<Ast> lhs, shared_ptr<Ast> rhs) :
	lhs(lhs),
	rhs(rhs)
{ }

std::string Ast::Eqv::to_pn(void) const {
	return "= " + lhs->to_pn() + " " + rhs->to_pn();
}

std::string Ast::Eqv::to_rpn(void) const {
	return lhs->to_rpn() + " " + rhs->to_rpn() + " =";
}

Ast::Xor::Xor(shared_ptr<Ast> lhs, shared_ptr<Ast> rhs) :
	lhs(lhs),
	rhs(rhs)
{ }

std::string Ast::Xor::to_pn(void) const {
	return "^ " + lhs->to_pn() + " " + rhs->to_pn();
}

std::string Ast::Xor::to_rpn(void) const {
	return lhs->to_rpn() + " " + rhs->to_rpn() + " ^";
}

} /* namespace Propcalc */
