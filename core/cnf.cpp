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

CNF::CNF(const Formula& fm) : fm(fm) {
	/* Skip all And nodes at the root, recursively. These just
	 * tell us to concatenate the clauses of the maximal subtrees
	 * without an And at the root. This way, the truthtables
	 * of subtrees are smaller. */
	queue.push(fm.root);
	while (queue.front()->type() == Ast::Type::And) {
		Ast::And* v = static_cast<Ast::And*>(queue.front().get());
		queue.pop();
		queue.push(v->lhs);
		queue.push(v->rhs);
	}
	++*this; /* forward to the first clause */
}

CNF& CNF::operator++(void) {
	while (true) {
		if (!current) {
			if (queue.size() == 0)
				break; /* no more clauses */
			current = queue.front();
			queue.pop();
			last = Assignment(Formula(current, fm.domain).vars());
		}
		else {
			++last;
			if (last.overflown()) {
				current = nullptr;
				continue;
			}
		}

		if (!fm.eval(last)) {
			cl = ~last;
			break; /* found the next clause */
		}
	}
	return *this;
}

}
