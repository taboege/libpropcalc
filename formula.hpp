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
	class Tseitin;
	class CNF;

	class Formula {
		std::shared_ptr<Domain> domain;
		std::shared_ptr<Ast>    root;

		friend class Truthtable;
		friend class Tseitin;
		friend class CNF;

	public:
		Formula(std::string fm, std::shared_ptr<Domain> domain = DefaultDomain);
		Formula(std::shared_ptr<Ast> root, std::shared_ptr<Domain> domain = DefaultDomain) :
			domain(domain),
			root(root)
		{ }

		std::vector<VarRef> vars(void) const;
		bool eval(const Assignment& assign) const { return root->eval(assign); }

		Formula simplify(void) const { return this->simplify(Assignment()); }
		Formula simplify(const Assignment& assign) const {
			return Formula(root->simplify(assign), domain);
		}

		Truthtable truthtable(void) const;
		Tseitin    tseitin(void)    const;
		CNF        cnf(void)        const;

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
}

/* Complete the interface of Formula. */
#include <propcalc/truthtable.hpp>
#include <propcalc/tseitin.hpp>
#include <propcalc/cnf.hpp>

#endif /* PROPCALC_FORMULA_HPP */
