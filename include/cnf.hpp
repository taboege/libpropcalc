/*
 * cnf.hpp - CNF clause stream
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
	/**
	 * This stream takes a Formula and lazily enumerates the clauses of
	 * an equivalent formula in conjunctive normal form.
	 *
	 * It does this by first skipping all Ast::And nodes at the root
	 * recursively collecting their non-And children. Concatenating the
	 * CNFs of these children yields a CNF of the entire. Then the
	 * truthtable of each child is enumerated: every non-satisfying
	 * assignment becomes one clause forbidding that assignment.
	 */
	class CNF : public Stream<Clause> {
		Formula fm;
		std::queue<std::shared_ptr<Ast>> queue;
		std::shared_ptr<Ast> current = nullptr;
		Assignment last;

	public:
		CNF(const Formula& fm);

		operator bool(void) const {
			return queue.size() > 0 || current != nullptr;
		}

		CNF& operator++(void);
	};
}

#endif /* PROPCALC_CNF_HPP */
