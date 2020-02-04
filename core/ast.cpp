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

shared_ptr<Ast> Ast::Not::simplify(const Assignment& assign) const {
	auto newrhs = rhs->simplify(assign);
	/* Remove double negations */
	size_t toggles = 1;
	while (newrhs->type() == Ast::Type::Not) {
		newrhs = static_cast<Ast::Not*>(newrhs.get())->rhs;
		++toggles;
	}
	if (toggles % 2 == 0)
		return newrhs;
	/* Reduce Not Const */
	if (newrhs->type() == Ast::Type::Const) {
		return make_shared<Ast::Const>(
			not static_cast<Ast::Const*>(newrhs.get())->value
		);
	}
	return make_shared<Ast::Not>(newrhs);
}

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

shared_ptr<Ast> Ast::And::simplify(const Assignment& assign) const {
	auto newlhs = lhs->simplify(assign);
	auto newrhs = rhs->simplify(assign);
	if (newlhs->type() == Ast::Type::Const) {
		return static_cast<Ast::Const*>(newlhs.get())->value
			? newrhs : make_shared<Ast::Const>(false);
	}
	if (newrhs->type() == Ast::Type::Const) {
		return static_cast<Ast::Const*>(newrhs.get())->value
			? newlhs : make_shared<Ast::Const>(false);
	}
	return make_shared<Ast::And>(newlhs, newrhs);
}

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

shared_ptr<Ast> Ast::Or::simplify(const Assignment& assign) const {
	auto newlhs = lhs->simplify(assign);
	auto newrhs = rhs->simplify(assign);
	if (newlhs->type() == Ast::Type::Const) {
		return static_cast<Ast::Const*>(newlhs.get())->value
			? make_shared<Ast::Const>(true) : newrhs;
	}
	if (newrhs->type() == Ast::Type::Const) {
		return static_cast<Ast::Const*>(newrhs.get())->value
			? make_shared<Ast::Const>(true) : newlhs;
	}
	return make_shared<Ast::Or>(newlhs, newrhs);
}

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

shared_ptr<Ast> Ast::Impl::simplify(const Assignment& assign) const {
	auto newlhs = lhs->simplify(assign);
	auto newrhs = rhs->simplify(assign);
	if (newlhs->type() == Ast::Type::Const) {
		return static_cast<Ast::Const*>(newlhs.get())->value
			? newrhs : make_shared<Ast::Const>(true);
	}
	if (newrhs->type() == Ast::Type::Const) {
		return static_cast<Ast::Const*>(newrhs.get())->value
			? make_shared<Ast::Const>(true)
			: Ast::Not(newlhs).simplify(assign);
	}
	return make_shared<Ast::Impl>(newlhs, newrhs);
}

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

shared_ptr<Ast> Ast::Eqv::simplify(const Assignment& assign) const {
	auto newlhs = lhs->simplify(assign);
	auto newrhs = rhs->simplify(assign);
	if (newlhs->type() == Ast::Type::Const) {
		return static_cast<Ast::Const*>(newlhs.get())->value
			? newrhs : Ast::Not(newrhs).simplify(assign);
	}
	if (newrhs->type() == Ast::Type::Const) {
		return static_cast<Ast::Const*>(newrhs.get())->value
			? newlhs : Ast::Not(newlhs).simplify(assign);
	}
	return make_shared<Ast::Eqv>(newlhs, newrhs);
}

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

shared_ptr<Ast> Ast::Xor::simplify(const Assignment& assign) const {
	auto newlhs = lhs->simplify(assign);
	auto newrhs = rhs->simplify(assign);
	if (newlhs->type() == Ast::Type::Const) {
		return static_cast<Ast::Const*>(newlhs.get())->value
			? Ast::Not(newrhs).simplify(assign) : newrhs;
	}
	if (newrhs->type() == Ast::Type::Const) {
		return static_cast<Ast::Const*>(newrhs.get())->value
			? Ast::Not(newlhs).simplify(assign) : newlhs;
	}
	return make_shared<Ast::Xor>(newlhs, newrhs);
}

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
