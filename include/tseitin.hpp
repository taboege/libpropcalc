/*
 * tseitin.hpp - Tseitin transform
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
	// TODO: Turn this initializer list into a custom literal?
	using ClauseData = std::initializer_list<std::pair<VarRef, bool>>;
	static inline std::unique_ptr<Clause> make_clause(ClauseData&& cd) {
		return std::make_unique<Clause>(std::forward<ClauseData>(cd));
	}

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

		class Domain : public Cache {
		private:
			std::map<std::shared_ptr<Ast>, VarRef> astcache;

		public:
			VarRef get(std::shared_ptr<Ast> ast) {
				const std::lock_guard<std::mutex> lock(access);

				VarRef var;
				auto it = astcache.find(ast);
				if (it == astcache.end()) {
					auto uvar = std::make_unique<Tseitin::Variable>(ast);
					std::tie(std::ignore, var) = this->put_variable(std::move(uvar));
					astcache.insert({ ast, var });
				}
				else {
					var = it->second;
				}
				return var;
			}
		};

		Formula fm;
		std::unique_ptr<Domain> vars;
		std::queue<std::shared_ptr<Ast>> queue;
		std::queue<std::unique_ptr<Clause>> clauses;
		std::unique_ptr<Clause> last;

	public:
		Tseitin(const Formula& fm) : fm(fm) {
			vars = std::make_unique<Domain>();
			queue.push(fm.root);
			++*this; /* make the first clause available */
		}

		operator bool(void) const {
			return queue.size() > 0 || clauses.size() > 0;
		}

		Clause operator*(void) const {
			return *last;
		}

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
						clauses.push(make_clause({ {a, false}, {b,  true}, {c, false} }));
						clauses.push(make_clause({ {a,  true},             {c,  true} }));
						clauses.push(make_clause({             {b, false}, {c,  true} }));
						queue.push(C->lhs);
						queue.push(C->rhs);
						break;
					}

					case Ast::Type::Eqv: {
						auto C = static_cast<Ast::Eqv*>(ast.get());
						auto c = vars->get(ast);
						auto a = vars->get(C->lhs);
						auto b = vars->get(C->rhs);
						clauses.push(make_clause({ {a, false}, {b, false}, {c,  true} }));
						clauses.push(make_clause({ {a,  true}, {b,  true}, {c,  true} }));
						clauses.push(make_clause({ {a,  true}, {b, false}, {c, false} }));
						clauses.push(make_clause({ {a, false}, {b,  true}, {c, false} }));
						queue.push(C->lhs);
						queue.push(C->rhs);
						break;
					}

					case Ast::Type::Xor: {
						auto C = static_cast<Ast::Xor*>(ast.get());
						auto c = vars->get(ast);
						auto a = vars->get(C->lhs);
						auto b = vars->get(C->rhs);
						clauses.push(make_clause({ {a, false}, {b, false}, {c, false} }));
						clauses.push(make_clause({ {a,  true}, {b,  true}, {c, false} }));
						clauses.push(make_clause({ {a,  true}, {b, false}, {c,  true} }));
						clauses.push(make_clause({ {a, false}, {b,  true}, {c,  true} }));
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
