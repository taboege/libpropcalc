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

#include <propcalc/ast.hpp>
#include <propcalc/variable.hpp>
#include <propcalc/assignment.hpp>

namespace Propcalc {
	extern std::shared_ptr<Cache> DefaultDomain;

	class Formula {
		std::shared_ptr<Domain> domain;
		std::shared_ptr<Ast>    root;

	public:
		Formula(std::string fm, std::shared_ptr<Domain> domain = DefaultDomain);
		Formula(std::shared_ptr<Ast> root, std::shared_ptr<Domain> domain = DefaultDomain);

		std::vector<const Variable*> vars(void) const;

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

#endif /* PROPCALC_FORMULA_HPP */
