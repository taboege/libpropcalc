/*
 * truthtable.hpp - Truthtable
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

#ifndef PROPCALC_TRUTHTABLE_HPP
#define PROPCALC_TRUTHTABLE_HPP

#include <propcalc/stream.hpp>
#include <propcalc/formula.hpp>
#include <propcalc/assignment.hpp>

namespace Propcalc {
	class Truthtable : public Stream<bool> {
		Formula fm;
		Assignment last;

	public:
		Truthtable(const Formula& fm) :
			fm(fm),
			last(Assignment(fm.vars()))
		{ }

		bool exhausted(void) const { return last.overflown(); }
		bool value(void)           { return fm.eval(last); }
		Assignment& assigned(void) { return last; }

		Truthtable& operator++(void) {
			++last;
			return *this;
		}
	};
}

#endif /* PROPCALC_TRUTHTABLE_HPP */
