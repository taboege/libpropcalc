/*
 * cnf.cpp - CNF clause stream
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

#include <propcalc/cnf.hpp>

using namespace std;

namespace Propcalc {

CNF::CNF(const Formula& fm) : cur(fm) {
	it = fm.begin();
	endit = fm.end();
	tabulating = false;
	++*this; /* forward to the first clause */
}

CNF& CNF::operator++(void) {
	while (true) {
		if (not tabulating) {
			/* Skip all And nodes at the root, recursively. These just
			 * tell us to concatenate the clauses of the maximal subtrees
			 * without an And at the root. This way, the truthtables
			 * of subtrees are smaller. */
			while (it != endit && (*it).type == Ast::Type::And)
				++it;
			if (it == endit)
				break; /* no more clauses */
			tabulating = true;
			cur = it.materialize();
			last = cur.assignment();
		}
		else {
			++last;
			if (last.overflown()) {
				tabulating = false;
				continue;
			}
		}

		if (!cur.eval(last)) {
			cl = ~last;
			break; /* found the next clause */
		}
	}
	return *this;
}

}
