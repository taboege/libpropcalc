/*
 * propcalc.hpp - Propositional calculus package
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

#ifndef PROPCALC_HPP
#define PROPCALC_HPP

#define __PROPCALC_MAKE_VERSION(api, major, minor)	\
	(api << 16 | major << 8 | minor)

#define PROPCALC_VERSION	__PROPCALC_MAKE_VERSION(0, 0, 1)

#include <propcalc/ast.hpp>
#include <propcalc/variable.hpp>
#include <propcalc/formula.hpp>

#endif /* PROPCALC_HPP */
