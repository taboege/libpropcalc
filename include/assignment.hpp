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

#include <iostream>

#include <vector>
#include <unordered_map>

#include <propcalc/varmap.hpp>

namespace Propcalc {
	/**
	 * Assignment is a VarMap used as input to Formula evaluation.
	 * It is not tied to a Domain object and remains only a partial
	 * assignment.
	 *
	 * Given the ordered set of variables of the VarMap, all Assignments
	 * on them are totally ordered. Assignment overloads the (post-)
	 * increment operator to produce the next one (or overflow).
	 *
	 * The Assignment created without constructor arguments is special
	 * as it is immediately marked as overflown. All other constructors
	 * mark the Assignment as not overflown, as even the empty set of
	 * variables has one (empty) assignment.
	 */
	class Assignment : public VarMap {
		bool overflow;

	public:
		/** Create a dummy assignment on no variables. */
		Assignment(void) : VarMap(), overflow(true) { }
		/** Create the all-false assignment on the given variables. */
		Assignment(std::vector<VarRef> vars) : VarMap(vars), overflow(false) { }
		/** Initialize the mapping with the given data. */
		Assignment(std::initializer_list<std::pair<VarRef, bool>> il) : VarMap(il), overflow(false) { }
		/** Initialize the assignment from a VarMap object. */
		Assignment(VarMap&& vm) : VarMap(vm), overflow(false) { }

		/**
		 * Whether or not the last increment caused the assignment
		 * to overflow back to all-false.
		 */
		bool  overflown(void) const { return overflow; }
		bool& overflown(void)       { return overflow; }

		/** The negated assignment. */
		Assignment operator~(void) const;

		/**
		 * Produce the lexicographically next assignment.
		 * This may overflow back to the all-false assignment.
		 */
		Assignment& operator++(void);
		Assignment  operator++(int);
	};

	namespace {
		[[maybe_unused]]
		std::ostream& operator<<(std::ostream& os, const Assignment& assign) {
			os << "{ ";
			for (auto& v : assign.vars())
				os << v->name << "(" << assign[v] << ") ";
			return os << "}";
		}
	}
}

#endif /* PROPCALC_ASSIGNMENT_HPP */
