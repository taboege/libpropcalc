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
	 * subformula of the formula. For this reason, the domain of the
	 * Clause objects returned by this stream is different from the
	 * original formula. It is an instance of Tseitin::Domain which
	 * is available as Tseitin::domain and which allows to look up
	 * Tseitin::Variable objects by their subformula's root. Conversely,
	 * the variable objects also store their subformula root.
	 *
	 * The original formula must not be destroyed while any of these
	 * objects are still in use.
	 */
	class Tseitin : public Stream<Clause> {
		class Variable : public Propcalc::Variable {
		public:
			const Formula& fm;
			unsigned int root;

			Variable(const Formula& fm, unsigned int root) :
				/* XXX: Computing the infix stringification is very costly
				 * but nice for debugging. This could be sped up by moving
				 * to_infix() code to Formula::iterator. */
				Propcalc::Variable("Tseitin[" + fm.begin(root).materialize().to_infix() + "]"),
				fm(fm),
				root(root)
			{ }
		};

		class Domain : public Cache {
		private:
			const Formula& fm;
			/* TODO: This is also time-consuming and memory-hungry but we have
			 * to ensure that equal subformulas of fm are mapped to the same
			 * Tseitin::Variable. */
			std::unordered_map<Formula, VarRef> subcache;

		public:
			Domain(const Formula& fm) : fm(fm) { }

			VarRef get(unsigned int root) {
				const std::lock_guard<std::mutex> lock(access);

				VarRef var;
				auto subfm = fm.begin(root).materialize();
				auto it = subcache.find(subfm);
				if (it != subcache.end()) {
					var = it->second;
				}
				else {
					auto uvar = std::make_unique<Tseitin::Variable>(fm, root);
					std::tie(std::ignore, var) = put_variable(std::move(uvar));
					subcache.insert({ subfm, var });
				}
				return var;
			}

			VarRef get(Formula::iterator it) {
				return get(it.current);
			}
		};

		Formula fm;
		unsigned int index;
		std::unique_ptr<Tseitin::Domain> vars;
		std::queue<std::unique_ptr<Clause>> clauses;
		std::unique_ptr<Clause> last;
		/* Whether the iterator is valid, i.e. last was populated with
		 * a new Assignment when operator++ last ran. */
		bool valid;

	public:
		Propcalc::Domain* domain; /* = vars.get() */

		Tseitin(const Formula& fm);

		/** Lift an assignment from the source domain to the Tseitin domain. */
		Assignment lift(const Assignment& assign) {
			Assignment lassign;

			for (auto& v : vars->list()) {
				auto tv = static_cast<const Tseitin::Variable*>(v);
				// TODO: Do not materialize here!
				// I think eval, simplify, to_*fix should work on either
				// the Formula.pn or Formula::iterator.
				lassign[v] = fm.begin(tv->root).materialize().eval(assign);
			}
			return lassign;
		}

		/** Project an assignment from the Tseitin domain to the source domain. */
		Assignment project(const Assignment& lassign) {
			Assignment assign;

			for (auto& v : vars->list()) {
				auto tv = static_cast<const Tseitin::Variable*>(v);
				if (fm.pn[tv->root].type != Ast::Type::Var)
					continue;
				auto w = fm.domain->unpack(fm.pn[tv->root].var);
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
