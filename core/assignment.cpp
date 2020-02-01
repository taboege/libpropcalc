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

Assignment::Assignment(vector<VarRef> vars) {
	for (auto& v : vars) {
		order.push_back(v);
		assign.insert({ v, false });
	}
	overflow = order.size() == 0;
}

Assignment::Assignment(initializer_list<pair<VarRef, bool>> il) {
	/* Order specified by the list */
	for (auto& p : il) {
		order.push_back(p.first);
		assign.insert(p);
	}
	overflow = order.size() == 0;
}

bool Assignment::exists(VarRef var) const {
	return assign.count(var) > 0;
}

set<VarRef> Assignment::set(void) const {
	auto set = std::set<VarRef>();
	for (auto& p : assign) {
		if (p.second)
			set.insert(p.first);
	}
	return set;
}

Assignment Assignment::operator~(void) const {
	Assignment neg(order);
	for (auto& v : order)
		neg[v] = !assign.at(v);
	return neg;
}

Assignment& Assignment::operator++(void) {
	/* Use the given order to implement a (consistent) binary incrementer
	 * which adheres to the variable order given at construction. */
	size_t n = 0;
	for (auto& v : order) {
		bool bit = assign.at(v) ^= true;
		if (bit)
			break;
		n++;
	}
	overflow = n >= assign.size();
	return *this;
}

Assignment Assignment::operator++(int) {
	Assignment tmp(*this);
	operator++();
	return tmp;
}

bool& Assignment::operator[](VarRef v) {
	if (not assign.count(v))
		order.push_back(v);
	return assign[v];
}

bool Assignment::operator[](VarRef v) const {
	return assign.at(v);
}

}
