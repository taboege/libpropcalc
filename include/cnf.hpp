/*
 * cnf.hpp - CNF clause stream
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

#ifndef PROPCALC_CNF_HPP
#define PROPCALC_CNF_HPP

#include <queue>
#include <memory>

#include <propcalc/ast.hpp>
#include <propcalc/stream.hpp>
#include <propcalc/clause.hpp>
#include <propcalc/formula.hpp>
#include <propcalc/assignment.hpp>

namespace Propcalc {
	class CNF : public Stream<Clause*> {
		Formula fm;
		std::queue<std::shared_ptr<Ast>> queue;
		std::shared_ptr<Ast> current = nullptr;
		Assignment last;
		Clause cl;

	public:
		CNF(const Formula& fm) : fm(fm) {
			/* Skip all And nodes at the root, recursively. These just
			 * tell us to concatenate the clauses of the maximal subtrees
			 * without and And at the root. This way, the truth tables
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

		bool exhausted(void) const { return queue.size() == 0 && current == nullptr; }
		Clause* value(void)  { return &cl; }
		Clause& clause(void) { return  cl; }

		CNF& operator++(void) {
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
	};
}

#endif /* PROPCALC_CNF_HPP */
