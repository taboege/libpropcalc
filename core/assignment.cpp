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
