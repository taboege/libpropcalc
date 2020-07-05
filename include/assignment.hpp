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
#include <unordered_set>

#include <propcalc/domain.hpp>

namespace Propcalc {
	/**
	 * VarMap represents a (partial) mapping from a collection of variables
	 * to truth values. It is not tied to a Domain object but the variables
	 * are totally ordered.
	 */
	class VarMap {
	protected:
		std::vector<VarRef> order;
		std::unordered_map<VarRef, bool> vmap;

	public:
		/** Create a dummy assignment on no variables. */
		VarMap(void) { }
		/** Create the all-false assignment on the given variables. */
		VarMap(std::vector<VarRef> vars);
		/** Initialize the mapping with the given data. */
		VarMap(std::initializer_list<std::pair<VarRef, bool>> il);

		/** Whether a variable is referenced in the assignment at all. */
		bool exists(VarRef var) const;

		/** A const reference to the internal order of variables. */
		const std::vector<VarRef>& vars(void) const { return order; }

		/**
		 * Return the bool associated with the given variable.
		 * In the value-assignment form, a non-existent variable
		 * is added (as the last variable). In the value-read form,
		 * an std::out_of_range exception is thrown.
		 */
		bool& operator[](VarRef v);
		bool  operator[](VarRef v) const;

		bool operator==(const VarMap& b) const {
			return order == b.order && vmap == b.vmap;
		}
	};

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
