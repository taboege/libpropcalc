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

VarRef Tseitin::Domain::get(std::shared_ptr<Ast> ast) {
	const lock_guard<mutex> lock(access);

	VarRef var;
	auto it = astcache.find(ast);
	if (it != astcache.end()) {
		var = it->second;
	}
	else {
		/* If the AST node is not cached, we might still have the same
		 * tree, but not the same AST object cached. We have to do a
		 * full scan and look for the AST structure. */
		for (auto& [a, v] : astcache) {
			if (ast->equals(*a)) {
				var = v;
				astcache.insert({ ast, var });
				goto end;
			}
		}
		auto uvar = make_unique<Tseitin::Variable>(ast);
		tie(std::ignore, var) = put_variable(move(uvar));
		astcache.insert({ ast, var });
	}
end:
	return var;
}

Tseitin::Tseitin(const Formula& fm) : fm(fm) {
	vars = std::make_shared<Tseitin::Domain>();
	domain = vars.get();
	/* Require that the root node be true. */
	clauses.push(make_clause({ {vars->get(fm.root), true} }));
	/* Kick off recursive conversion of the AST structure
	 * into CNF clauses. */
	queue.push(fm.root);
	++*this; /* make the first clause available */
}

Tseitin& Tseitin::operator++(void) {
	while (true) {
		if (clauses.size() > 0) {
			last = move(clauses.front());
			clauses.pop();
			produce(*last);
			valid = true;
			break; /* found the next clause */
		}

		if (queue.size() == 0) {
			valid = false;
			break; /* exhausted */
		}

		/* Each Ast::Type has its own CNF template to convert `(a <op> b) = c`.
		 * Note: there are some clauses below which are conditioned on a != b.
		 * This is because the Clause class cannot hold the same variable a
		 * in both a positive and a negative literal. Since whenever this is
		 * a problem the clause is also vacuously fulfilled, we leave them out. */
		auto ast = queue.front();
		queue.pop();
		switch (ast->type()) {
			case Ast::Type::Const: {
				auto C = static_cast<Ast::Const*>(ast.get());
				auto c = vars->get(ast);
				clauses.push(make_clause({ {c, C->value} }));
				break;
			}

			case Ast::Type::Var: {
				/* nothing to do */
				break;
			}

			case Ast::Type::Not: {
				auto C = static_cast<Ast::Not*>(ast.get());
				auto c = vars->get(ast);
				auto a = vars->get(C->rhs);
				clauses.push(make_clause({ {a, false}, {c, false} }));
				clauses.push(make_clause({ {a,  true}, {c,  true} }));
				queue.push(C->rhs);
				break;
			}

			case Ast::Type::And: {
				auto C = static_cast<Ast::And*>(ast.get());
				auto c = vars->get(ast);
				auto a = vars->get(C->lhs);
				auto b = vars->get(C->rhs);
				clauses.push(make_clause({ {a, false}, {b, false}, {c,  true} }));
				clauses.push(make_clause({ {a,  true},             {c, false} }));
				clauses.push(make_clause({             {b,  true}, {c, false} }));
				queue.push(C->lhs);
				queue.push(C->rhs);
				break;
			}

			case Ast::Type::Or: {
				auto C = static_cast<Ast::Or*>(ast.get());
				auto c = vars->get(ast);
				auto a = vars->get(C->lhs);
				auto b = vars->get(C->rhs);
				clauses.push(make_clause({ {a,  true}, {b,  true}, {c, false} }));
				clauses.push(make_clause({ {a, false},             {c,  true} }));
				clauses.push(make_clause({             {b, false}, {c,  true} }));
				queue.push(C->lhs);
				queue.push(C->rhs);
				break;
			}

			case Ast::Type::Impl: {
				auto C = static_cast<Ast::Impl*>(ast.get());
				auto c = vars->get(ast);
				auto a = vars->get(C->lhs);
				auto b = vars->get(C->rhs);
				if (a != b)
					clauses.push(make_clause({ {a, false}, {b,  true}, {c, false} }));
				clauses.push(    make_clause({ {a,  true},             {c,  true} }));
				clauses.push(    make_clause({             {b, false}, {c,  true} }));
				queue.push(C->lhs);
				queue.push(C->rhs);
				break;
			}

			case Ast::Type::Eqv: {
				auto C = static_cast<Ast::Eqv*>(ast.get());
				auto c = vars->get(ast);
				auto a = vars->get(C->lhs);
				auto b = vars->get(C->rhs);
				clauses.push(    make_clause({ {a, false}, {b, false}, {c,  true} }));
				clauses.push(    make_clause({ {a,  true}, {b,  true}, {c,  true} }));
				if (a != b) {
					clauses.push(make_clause({ {a,  true}, {b, false}, {c, false} }));
					clauses.push(make_clause({ {a, false}, {b,  true}, {c, false} }));
				}
				queue.push(C->lhs);
				queue.push(C->rhs);
				break;
			}

			case Ast::Type::Xor: {
				auto C = static_cast<Ast::Xor*>(ast.get());
				auto c = vars->get(ast);
				auto a = vars->get(C->lhs);
				auto b = vars->get(C->rhs);
				clauses.push(    make_clause({ {a, false}, {b, false}, {c, false} }));
				clauses.push(    make_clause({ {a,  true}, {b,  true}, {c, false} }));
				if (a != b) {
					clauses.push(make_clause({ {a,  true}, {b, false}, {c,  true} }));
					clauses.push(make_clause({ {a, false}, {b,  true}, {c,  true} }));
				}
				queue.push(C->lhs);
				queue.push(C->rhs);
				break;
			}
		}
	}
	return *this;
}

}
