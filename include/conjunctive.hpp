/*
 * conjunctive.hpp - Clause and Conjunctive
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

#ifndef PROPCALC_CONJUNCTIVE_HPP
#define PROPCALC_CONJUNCTIVE_HPP

#include <propcalc/varmap.hpp>
#include <propcalc/stream.hpp>
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

		/**
		 * Evaluate the Clause on an Assignment. This uses the natural partial
		 * assignment semantics, that is when a variable in the assignment is
		 * not mentioned in the clause, this fact is ignored. Such a variable
		 * cannot make the clause true.
		 *
		 * In particular an empty clause always yields false (the identity
		 * element with respect to disjunction).
		 */
		bool eval(const Assignment& assign) const {
			for (auto v : assign.vars()) {
				if (exists(v) && (*this)[v] == assign[v])
					return true;
			}
			return false;
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

	class Conjunctive : public Stream<Clause> {
	public:
		/**
		 * Evaluate the conjunction of clauses enumerated. If there is no
		 * clause, returns true (the identity element with respect to
		 * conjunction), as all clauses are satisfied.
		 *
		 * If you want to evaluate the Conjunctive multiple times, it must
		 * be put into caching mode before the first clause is iterated..
		 */
		bool eval(const Assignment& assign) {
			for (auto cl : *this) {
				if (!cl.eval(assign))
					return false;
			}
			return true;
		}
	};
}

#endif /* PROPCALC_CONJUNCTIVE_HPP */
