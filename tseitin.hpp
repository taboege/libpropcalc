/*
 * tseitin.hpp - Tseitin transform
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

#ifndef PROPCALC_TSEITIN_HPP
#define PROPCALC_TSEITIN_HPP

#include <queue>
#include <memory>
#include <mutex>

#include <propcalc/ast.hpp>
#include <propcalc/stream.hpp>
#include <propcalc/clause.hpp>
#include <propcalc/formula.hpp>
#include <propcalc/variable.hpp>

namespace Propcalc {
	class Tseitin : public Stream<Clause> {
		class Variable : public Propcalc::Variable {
		public:
			std::shared_ptr<Ast> ast;

			Variable(std::shared_ptr<Ast> ast) :
				/* XXX: This may be too costly for people who use the
				 * Tseitin transform because it is fast. */
				Propcalc::Variable("Tseitin[" + ast->to_infix() + "]"),
				ast(ast)
			{ }
		};

		/* This is not a real Domain because we don't create formulas from
		 * Tseitin, but a stream of clauses. The full domain interface is
		 * useless here. FIXME: This is basically half a copy of Cache. */
		class Domain {
		private:
			std::mutex access;
			std::map<
				std::shared_ptr<Ast>,
				std::unique_ptr<Tseitin::Variable>
			> cache;

		public:
			const Variable* get(std::shared_ptr<Ast> ast) {
				const std::lock_guard<std::mutex> lock(access);
				const Variable* var;

				auto it = cache.find(ast);
				if (it == cache.end()) {
					auto uvar = std::make_unique<Tseitin::Variable>(ast);
					var = uvar.get();
					cache.insert({ ast, std::move(uvar) });
				}
				else {
					var = it->second.get();
				}
				return var;
			}
		};

		Formula fm;
		std::unique_ptr<Domain> vars;
		std::queue<std::shared_ptr<Ast>> queue;
		std::queue<Clause> clauses;
		Clause last;

	public:
		Tseitin(const Formula& fm) : fm(fm) {
			vars = std::make_unique<Domain>();
			queue.push(fm.root);
			++*this; /* make the first clause available */
		}

		bool exhausted(void) const { return queue.size() == 0 && clauses.size() == 0; }
		Clause  value(void)  { return last; }
		Clause& clause(void) { return last; }

		Tseitin& operator++(void) {
			while (true) {
				if (clauses.size() > 0) {
					last = std::move(clauses.front());
					clauses.pop();
					break; /* found the next clause */
				}

				if (queue.size() == 0)
					break; /* exhausted */

				auto ast = queue.front();
				queue.pop();
				switch (ast->type()) {
					case Ast::Type::Const: {
						auto C = static_cast<Ast::Const*>(ast.get());
						auto c = vars->get(ast);
						clauses.push(Clause({ {c, C->value} }));
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
						clauses.push(Clause({ {a, false}, {c, false} }));
						clauses.push(Clause({ {a,  true}, {c,  true} }));
						queue.push(C->rhs);
						break;
					}

					case Ast::Type::And: {
						auto C = static_cast<Ast::And*>(ast.get());
						auto c = vars->get(ast);
						auto a = vars->get(C->lhs);
						auto b = vars->get(C->rhs);
						clauses.push(Clause({ {a, false}, {b, false}, {c,  true} }));
						clauses.push(Clause({ {a,  true},             {c, false} }));
						clauses.push(Clause({             {b,  true}, {c, false} }));
						queue.push(C->lhs);
						queue.push(C->rhs);
						break;
					}

					case Ast::Type::Or: {
						auto C = static_cast<Ast::Or*>(ast.get());
						auto c = vars->get(ast);
						auto a = vars->get(C->lhs);
						auto b = vars->get(C->rhs);
						clauses.push(Clause({ {a,  true}, {b,  true}, {c, false} }));
						clauses.push(Clause({ {a, false},             {c,  true} }));
						clauses.push(Clause({             {b, false}, {c,  true} }));
						queue.push(C->lhs);
						queue.push(C->rhs);
						break;
					}

					case Ast::Type::Impl: {
						auto C = static_cast<Ast::Impl*>(ast.get());
						auto c = vars->get(ast);
						auto a = vars->get(C->lhs);
						auto b = vars->get(C->rhs);
						clauses.push(Clause({ {a, false}, {b,  true}, {c, false} }));
						clauses.push(Clause({ {a,  true},             {c,  true} }));
						clauses.push(Clause({             {b, false}, {c,  true} }));
						queue.push(C->lhs);
						queue.push(C->rhs);
						break;
					}

					case Ast::Type::Eqv: {
						auto C = static_cast<Ast::Eqv*>(ast.get());
						auto c = vars->get(ast);
						auto a = vars->get(C->lhs);
						auto b = vars->get(C->rhs);
						clauses.push(Clause({ {a, false}, {b, false}, {c,  true} }));
						clauses.push(Clause({ {a,  true}, {b,  true}, {c,  true} }));
						clauses.push(Clause({ {a,  true}, {b, false}, {c, false} }));
						clauses.push(Clause({ {a, false}, {b,  true}, {c, false} }));
						queue.push(C->lhs);
						queue.push(C->rhs);
						break;
					}

					case Ast::Type::Xor: {
						auto C = static_cast<Ast::Xor*>(ast.get());
						auto c = vars->get(ast);
						auto a = vars->get(C->lhs);
						auto b = vars->get(C->rhs);
						clauses.push(Clause({ {a, false}, {b, false}, {c, false} }));
						clauses.push(Clause({ {a,  true}, {b,  true}, {c, false} }));
						clauses.push(Clause({ {a,  true}, {b, false}, {c,  true} }));
						clauses.push(Clause({ {a, false}, {b,  true}, {c,  true} }));
						queue.push(C->lhs);
						queue.push(C->rhs);
						break;
					}
				}
			}
			return *this;
		}
	};
}

#endif /* PROPCALC_TSEITIN_HPP */
