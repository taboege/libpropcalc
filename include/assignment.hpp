/*
 * assignment.hpp - Assignment
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

#ifndef PROPCALC_ASSIGNMENT_HPP
#define PROPCALC_ASSIGNMENT_HPP

#include <set>
#include <vector>
#include <unordered_map>

#include <propcalc/variable.hpp>

namespace Propcalc {
	class Assignment {
		std::vector<VarRef> order;
		std::unordered_map<VarRef, bool> assign;
		bool overflow;

	public:
		Assignment() : overflow(true) { }
		Assignment(std::vector<VarRef> vars);
		Assignment(std::initializer_list<std::pair<VarRef, bool>> il);

		bool  overflown(void) const { return overflow; }
		bool& overflown(void)       { return overflow; }

		bool exists(VarRef var) const;
		std::set<VarRef> set(void) const;

		const std::vector<VarRef>& vars(void) const { return order; }

		Assignment  operator~(void) const;
		Assignment& operator++(void);
		Assignment  operator++(int);
		bool&       operator[](VarRef v);
		bool        operator[](VarRef v) const;
		bool&       operator[](size_t nr);
		bool        operator[](size_t nr) const;
	};
}

#endif /* PROPCALC_ASSIGNMENT_HPP */
