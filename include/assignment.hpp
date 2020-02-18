/*
 * assignment.hpp - Assignment
 *
 * Copyright (C) 2019-2020 Tobias Boege
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

#include <vector>
#include <unordered_map>
#include <unordered_set>

#include <propcalc/variable.hpp>

namespace Propcalc {
	/**
	 * Assignment represents a mapping from a collection of variables to
	 * truth values. It is not tied to a Domain object and with respect
	 * to evaluating a formula, Assignment is in general an incomplete
	 * or partial assignment.
	 *
	 * Given an ordered set of variables, all Assignments on them are
	 * totally ordered. Assignment overloads the (post-)increment operator
	 * to produce the next one (or overflow).
	 */
	class Assignment {
		std::vector<VarRef> order;
		std::unordered_map<VarRef, bool> assign;
		bool overflow;

	public:
		/** Create a dummy assignment on no variables. */
		Assignment() : overflow(true) { }
		/** Create the all-false assignment on the given variables. */
		Assignment(std::vector<VarRef> vars);
		/** Initialize the mapping with the given data. */
		Assignment(std::initializer_list<std::pair<VarRef, bool>> il);

		/** Whether or not the last increment caused the assignment
		 * to overflow back to all-false. */
		bool  overflown(void) const { return overflow; }
		bool& overflown(void)       { return overflow; }

		/** Whether a variable is referenced in the assignment at all. */
		bool exists(VarRef var) const;
		/** The set of all variables mapping to true. */
		std::unordered_set<VarRef> set(void) const;

		/** A const reference to the internal order of variables. */
		const std::vector<VarRef>& vars(void) const { return order; }

		/** The negated assignment. */
		Assignment  operator~(void) const;
		/** Produce the lexicographically next assignment.
		 * This may overflow back to the all-false assignment. */
		Assignment& operator++(void);
		Assignment  operator++(int);
		/** Return the bool associated with the given variable.
		 * In the value-assignment form, a non-existent variable is
		 * added (as the last variable). In the value-read form,
		 * an std::out_of_range exception is thrown. */
		bool&       operator[](VarRef v);
		bool        operator[](VarRef v) const;
	};
}

#endif /* PROPCALC_ASSIGNMENT_HPP */
