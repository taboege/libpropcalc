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
#include <unordered_map>

#include <propcalc/ast.hpp>
#include <propcalc/stream.hpp>
#include <propcalc/clause.hpp>
#include <propcalc/formula.hpp>
#include <propcalc/variable.hpp>

namespace Propcalc {
	/**
	 * This stream takes a Formula and lazily enumerates the clauses of
	 * its Tseitin transform, an equisatisfiable formula in CNF, whose
	 * size is linear in and which mimics the evaluation of the original
	 * formula.
	 *
	 * The Tseitin transform algorithm assigns a new variable to each
	 * node in the AST of the formula. For this reason, the domain of
	 * the Clause objects returned by this stream is different from the
	 * original formula. It is an instance of Tseitin::Domain which
	 * can be obtained from Tseitin::get_domain() and which allows to
	 * look up Tseitin::Variable objects by their AST node in the
	 * original formula. Conversel, the variable objects also store
	 * an std::shared_ptr to the AST node.
	 */
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
			std::unordered_map<std::shared_ptr<Ast>, VarRef> astcache;

		public:
			VarRef get(std::shared_ptr<Ast> ast);
		};

		Formula fm;
		std::shared_ptr<Tseitin::Domain> vars;
		std::queue<std::shared_ptr<Ast>> queue;
		std::queue<std::unique_ptr<Clause>> clauses;
		std::unique_ptr<Clause> last;
		/* Whether the iterator is valid, i.e. last was populated with
		 * a new Assignment when operator++ last ran. */
		bool valid;

	public:
		Tseitin(const Formula& fm);

		std::shared_ptr<Propcalc::Domain> get_domain() {
			return vars;
		}

		/** Lift an assignment from the source domain to the Tseitin domain. */
		Assignment lift(const Assignment& assign) {
			Assignment lassign;

			for (auto& v : vars->list()) {
				auto tv = static_cast<const Tseitin::Variable*>(v);
				lassign[v] = tv->ast->eval(assign);
			}
			return lassign;
		}

		/** Project an assignment from the Tseitin domain to the source domain. */
		Assignment project(const Assignment& lassign) {
			Assignment assign;

			for (auto& v : vars->list()) {
				auto tv = static_cast<const Tseitin::Variable*>(v);
				if (tv->ast->type() != Ast::Type::Var)
					continue;
				auto w = static_cast<const Ast::Var*>(tv->ast.get())->var;
				assign[w] = lassign[v];
			}

			return assign;
		}

		operator bool(void) const {
			return valid;
		}

		Clause operator*(void) const {
			return *last;
		}

		Tseitin& operator++(void);
	};
}

#endif /* PROPCALC_TSEITIN_HPP */
