/*
 * assignment.cpp - Assignment
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

#include <propcalc/assignment.hpp>

using namespace std;

namespace Propcalc {

Assignment::Assignment(vector<const Variable*> vars) :
	order(vars)
{
	for (auto& v : vars)
		assign.insert({ v, false });
	overflow = vars.size() == 0;
}

set<const Variable*> Assignment::set(void) const {
	auto set = std::set<const Variable*>();
	for (auto& p : assign) {
		if (p.second)
			set.insert(p.first);
	}
	return set;
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

bool& Assignment::operator[](const Variable *v) {
	return assign.at(v);
}

bool Assignment::operator[](const Variable *v) const {
	return assign.at(v);
}

bool& Assignment::operator[](size_t nr) {
	return assign.at(order[nr - 1]);
}

bool Assignment::operator[](size_t nr) const {
	return assign.at(order[nr - 1]);
}

}