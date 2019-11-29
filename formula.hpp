/*
 * formula.hpp - Formula class, parser
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

#ifndef PROPCALC_FORMULA_HPP
#define PROPCALC_FORMULA_HPP

#include <queue>
#include <memory>
#include <iterator>

#include <propcalc/ast.hpp>
#include <propcalc/variable.hpp>
#include <propcalc/assignment.hpp>

namespace Propcalc {
	extern std::shared_ptr<Cache> DefaultDomain;

	class Truthtable;

	class Formula {
		std::shared_ptr<Domain> domain;
		std::shared_ptr<Ast>    root;

		friend class Truthtable;

	public:
		Formula(std::string fm, std::shared_ptr<Domain> domain = DefaultDomain);
		Formula(std::shared_ptr<Ast> root, std::shared_ptr<Domain> domain = DefaultDomain) :
			domain(domain),
			root(root)
		{ }

		std::vector<const Variable*> vars(void) const;
		bool eval(const Assignment& assign) const { return root->eval(assign); }

		Truthtable truthtable(void) const;

		std::string to_infix(void)   const { return root->to_infix();   }
		std::string to_prefix(void)  const { return root->to_prefix();  }
		std::string to_postfix(void) const { return root->to_postfix(); }

		Formula operator~(void);
		Formula operator&(const Formula& rhs);
		Formula operator|(const Formula& rhs);
		Formula operator>>(const Formula& rhs);
		Formula operator==(const Formula& rhs);
		Formula operator^(const Formula& rhs);
	};

	class Truthtable {
		Formula fm;
		Assignment last;

	public:
		Truthtable(const Formula& fm) :
			fm(fm),
			last(Assignment(fm.vars()))
		{ }

		bool end(void) const       { return last.overflow; }
		bool value(void) const     { return fm.eval(last); }
		Assignment& assigned(void) { return last;          }

		Truthtable& operator++(void) {
			++last;
			return *this;
		}
		Truthtable operator++(int) {
			Truthtable tmp(*this);
			operator++();
			return tmp;
		}

		bool operator*(void) const { return this->value(); }
	};
}

#endif /* PROPCALC_FORMULA_HPP */
