/*
 * varmap.hpp - VarMap
 *
 * Copyright (C) 2020 Tobias Boege
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the Artistic License 2.0
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * Artistic License 2.0 for more details.
 */

#ifndef PROPCALC_VARMAP_HPP
#define PROPCALC_VARMAP_HPP

#include <vector>
#include <unordered_map>

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
		VarMap(std::vector<VarRef> vars) : order(vars) {
			for (auto& v : order)
				vmap.insert({ v, false });
		}

		/** Initialize the mapping with the given data. */
		VarMap(std::initializer_list<std::pair<VarRef, bool>> il) {
			/* Order specified by the list */
			for (auto& p : il) {
				order.push_back(p.first);
				vmap.insert(p);
			}
		}

		/** Whether a variable is referenced in the assignment at all. */
		bool exists(VarRef var) const {
			return vmap.count(var) > 0;
		}

		/** A const reference to the internal order of variables. */
		const std::vector<VarRef>& vars(void) const {
			return order;
		}

		/**
		 * Return the bool associated with the given variable.
		 * In the value-assignment form, a non-existent variable
		 * is added (as the last variable). In the value-read form,
		 * an std::out_of_range exception is thrown.
		 */
		bool& operator[](VarRef v) {
			if (not vmap.count(v))
				order.push_back(v);
			return vmap[v];
		}

		bool operator[](VarRef v) const {
			return vmap.at(v);
		}

		bool operator==(const VarMap& b) const {
			return order == b.order && vmap == b.vmap;
		}
	};
}

#endif /* PROPCALC_VARMAP_HPP */
