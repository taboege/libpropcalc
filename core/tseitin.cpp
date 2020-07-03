/*
 * tseitin.cpp - Tseitin transform
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

#include <mutex>

#include <propcalc/tseitin.hpp>

using namespace std;

namespace Propcalc {

// TODO: Turn this initializer list into a custom literal?
using ClauseData = initializer_list<pair<VarRef, bool>>;
static inline unique_ptr<Clause> make_clause(ClauseData&& cd) {
	return make_unique<Clause>(forward<ClauseData>(cd));
}

Tseitin::Tseitin(const Formula& fm) : fm(fm) {
	vars = make_unique<Tseitin::Domain>(fm);
	domain = vars.get();
	/* Require that the root node be true. */
	clauses.push(make_clause({ {vars->get(0), true} }));
	index = 0;
	++*this; /* make the first clause available */
}

Tseitin& Tseitin::operator++(void) {
	while (true) {
		if (clauses.size() > 0) {
			last = move(clauses.front());
			clauses.pop();
			valid = true;
			break; /* found the next clause */
		}

		if (index == fm.pn.size()) {
			valid = false;
			break; /* exhausted */
		}

		/* Each Ast::Type has its own CNF template to convert `(a <op> b) = c`.
		 * Note: there are some clauses below which are conditioned on a != b.
		 * This is because the Clause class cannot hold the same variable a
		 * in both a positive and a negative literal. Since whenever this is
		 * a problem the clause is also vacuously fulfilled, we leave them out. */
		/* TODO: This iterates over the subformula for each subformula,
		 * meaning quadratic time, while it could be done with a single
		 * iteration over the whole formula. */
		auto it = fm.begin(index++);
		auto ast = *it;
		switch (ast.type) {
			case Ast::Type::Const: {
				auto c = vars->get(it);
				auto C = ast.value;
				clauses.push(make_clause({ {c, C} }));
				break;
			}

			case Ast::Type::Var: {
				/* nothing to do */
				(void) vars->get(it);
				break;
			}

			case Ast::Type::Not: {
				auto c = vars->get(it);
				auto C = it.operands();
				auto a = vars->get(C[0]);
				clauses.push(make_clause({ {a, false}, {c, false} }));
				clauses.push(make_clause({ {a,  true}, {c,  true} }));
				break;
			}

			case Ast::Type::And: {
				auto c = vars->get(it);
				auto C = it.operands();
				auto a = vars->get(C[0]);
				auto b = vars->get(C[1]);
				clauses.push(make_clause({ {a, false}, {b, false}, {c,  true} }));
				clauses.push(make_clause({ {a,  true},             {c, false} }));
				clauses.push(make_clause({             {b,  true}, {c, false} }));
				break;
			}

			case Ast::Type::Or: {
				auto c = vars->get(it);
				auto C = it.operands();
				auto a = vars->get(C[0]);
				auto b = vars->get(C[1]);
				clauses.push(make_clause({ {a,  true}, {b,  true}, {c, false} }));
				clauses.push(make_clause({ {a, false},             {c,  true} }));
				clauses.push(make_clause({             {b, false}, {c,  true} }));
				break;
			}

			case Ast::Type::Impl: {
				auto c = vars->get(it);
				auto C = it.operands();
				auto a = vars->get(C[0]);
				auto b = vars->get(C[1]);
				if (a != b)
					clauses.push(make_clause({ {a, false}, {b,  true}, {c, false} }));
				clauses.push(    make_clause({ {a,  true},             {c,  true} }));
				clauses.push(    make_clause({             {b, false}, {c,  true} }));
				break;
			}

			case Ast::Type::Eqv: {
				auto c = vars->get(it);
				auto C = it.operands();
				auto a = vars->get(C[0]);
				auto b = vars->get(C[1]);
				clauses.push(    make_clause({ {a, false}, {b, false}, {c,  true} }));
				clauses.push(    make_clause({ {a,  true}, {b,  true}, {c,  true} }));
				if (a != b) {
					clauses.push(make_clause({ {a,  true}, {b, false}, {c, false} }));
					clauses.push(make_clause({ {a, false}, {b,  true}, {c, false} }));
				}
				break;
			}

			case Ast::Type::Xor: {
				auto c = vars->get(it);
				auto C = it.operands();
				auto a = vars->get(C[0]);
				auto b = vars->get(C[1]);
				clauses.push(    make_clause({ {a, false}, {b, false}, {c, false} }));
				clauses.push(    make_clause({ {a,  true}, {b,  true}, {c, false} }));
				if (a != b) {
					clauses.push(make_clause({ {a,  true}, {b, false}, {c,  true} }));
					clauses.push(make_clause({ {a, false}, {b,  true}, {c,  true} }));
				}
				break;
			}
		}
	}
	return *this;
}

}
