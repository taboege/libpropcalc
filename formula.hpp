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
#include <propcalc/clause.hpp>

namespace Propcalc {
	extern std::shared_ptr<Cache> DefaultDomain;

	class Truthtable;
	class CNF;

	class Formula {
		std::shared_ptr<Domain> domain;
		std::shared_ptr<Ast>    root;

		friend class Truthtable;
		friend class CNF;

	public:
		Formula(std::string fm, std::shared_ptr<Domain> domain = DefaultDomain);
		Formula(std::shared_ptr<Ast> root, std::shared_ptr<Domain> domain = DefaultDomain) :
			domain(domain),
			root(root)
		{ }

		std::vector<const Variable*> vars(void) const;
		bool eval(const Assignment& assign) const { return root->eval(assign); }

		Truthtable truthtable(void) const;
		CNF cnf(void) const;

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

	class Truthtable : public Stream<bool> {
		Formula fm;
		Assignment last;

	public:
		Truthtable(const Formula& fm) :
			fm(fm),
			last(Assignment(fm.vars()))
		{ }

		bool exhausted(void) const { return last.overflown(); }
		bool value(void)           { return fm.eval(last); }
		Assignment& assigned(void) { return last; }

		Truthtable& operator++(void) {
			++last;
			return *this;
		}
	};

	class CNF : public Stream<Clause> {
		Formula fm;
		std::queue<std::shared_ptr<Ast>> queue;
		std::shared_ptr<Ast> current = nullptr;
		Assignment last;

	public:
		CNF(const Formula& fm);

		bool exhausted(void) const { return queue.size() == 0 && current == nullptr; }
		Clause value(void)         { return last; }
		Assignment& assigned(void) { return last; }

		CNF& operator++(void);
	};
}

#endif /* PROPCALC_FORMULA_HPP */
