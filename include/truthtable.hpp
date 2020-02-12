/*
 * truthtable.hpp - Truthtable
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

#ifndef PROPCALC_TRUTHTABLE_HPP
#define PROPCALC_TRUTHTABLE_HPP

#include <propcalc/stream.hpp>
#include <propcalc/formula.hpp>
#include <propcalc/assignment.hpp>

namespace Propcalc {
	/**
	 * This stream takes a Formula and lazily runs through all of its
	 * assignments in lexicographic order, producing an std::pair of
	 * the assignment and the bool the Formula yields on it.
	 */
	class Truthtable : public Stream<std::pair<Assignment, bool>> {
		Formula fm;
		Assignment last;

	public:
		Truthtable(const Formula& fm) :
			fm(fm), last(fm.vars()) { }

		bool eval(void) const { return fm.eval(last); }
		const Assignment& assigned(void) const { return last; }

		std::pair<Assignment, bool> operator*(void) const {
			return std::make_pair(this->assigned(), this->eval());
		}

		operator bool(void) const {
			return !last.overflown();
		}

		Truthtable& operator++(void) {
			++last;
			return *this;
		}
	};
}

#endif /* PROPCALC_TRUTHTABLE_HPP */
