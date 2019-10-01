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

namespace Propcalc {
	class Formula {
		std::shared_ptr<Ast> root;

	public:
		Formula(std::string fm);
		Formula(std::shared_ptr<Ast> root);

		std::string to_rpn(void) const;
		std::string to_pn(void)  const;

		Formula operator~(void);
		Formula operator&(const Formula& rhs);
		Formula operator|(const Formula& rhs);
		Formula operator>>(const Formula& rhs);
		Formula operator==(const Formula& rhs);
		Formula operator^(const Formula& rhs);
	};
}

#endif /* PROPCALC_FORMULA_HPP */
