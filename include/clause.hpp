/*
 * clause.hpp - Clause
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

#ifndef PROPCALC_CLAUSE_HPP
#define PROPCALC_CLAUSE_HPP

#include <propcalc/assignment.hpp>

namespace Propcalc {
	/**
	 * A clause represents a collection of literals, that is a mapping of
	 * variables to signs. A variable mapping to true is a positive literal,
	 * one mapping to false a negative literal.
	 *
	 * Put another way, the value a variable maps to is the assignment to
	 * that variable which would satisfy the clause.
	 */
	class Clause : public VarMap {
	public:
		/** Create a dummy clause on no variables. */
		Clause(void) : VarMap() { }
		/** Create the any-false clause on the given variables. */
		Clause(std::vector<VarRef> vars) : VarMap(vars) { }
		/** Initialize the mapping with the given data. */
		Clause(std::initializer_list<std::pair<VarRef, bool>> il) : VarMap(il) { }
		/** Initialize the clause from a VarMap object. */
		Clause(VarMap&& vm) : VarMap(vm) { }

		/** Flip all signs in the clause. */
		Clause operator~(void) const {
			Clause neg(order);
			for (auto& v : order)
				neg[v] = !vmap.at(v);
			return neg;
		}
	};

	namespace {
		[[maybe_unused]]
		std::ostream& operator<<(std::ostream& os, const Clause& cl) {
			os << "{ ";
			for (auto& v : cl.vars())
				os << (cl[v] ? "" : "-") << v->name << " ";
			return os << "}";
		}
	}
}

#endif /* PROPCALC_CLAUSE_HPP */
