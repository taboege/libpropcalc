/*
 * assignment.cpp - Assignment
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

#include <propcalc/assignment.hpp>

using namespace std;

namespace Propcalc {

VarMap::VarMap(vector<VarRef> vars) {
	for (auto& v : vars) {
		order.push_back(v);
		vmap.insert({ v, false });
	}
}

VarMap::VarMap(initializer_list<pair<VarRef, bool>> il) {
	/* Order specified by the list */
	for (auto& p : il) {
		order.push_back(p.first);
		vmap.insert(p);
	}
}

bool VarMap::exists(VarRef var) const {
	return vmap.count(var) > 0;
}

bool& VarMap::operator[](VarRef v) {
	if (not vmap.count(v))
		order.push_back(v);
	return vmap[v];
}

bool VarMap::operator[](VarRef v) const {
	return vmap.at(v);
}

Assignment Assignment::operator~(void) const {
	Assignment neg(order);
	for (auto& v : order)
		neg[v] = !vmap.at(v);
	return neg;
}

Assignment& Assignment::operator++(void) {
	/* Use the given order to implement a (consistent) binary incrementer
	 * which adheres to the variable order given at construction. */
	size_t n = 0;
	for (auto& v : order) {
		bool bit = vmap.at(v) ^= true;
		if (bit)
			break;
		n++;
	}
	overflow = n >= vmap.size();
	return *this;
}

Assignment Assignment::operator++(int) {
	Assignment tmp(*this);
	operator++();
	return tmp;
}

}
