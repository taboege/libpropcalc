/*
 * formula.hpp - Formula class, parser
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

#ifndef PROPCALC_FORMULA_HPP
#define PROPCALC_FORMULA_HPP

#include <queue>
#include <memory>
#include <stdexcept>

#include <propcalc/ast.hpp>
#include <propcalc/variable.hpp>
#include <propcalc/assignment.hpp>
#include <propcalc/clause.hpp>
#include <propcalc/stream.hpp>

namespace Propcalc {
	namespace X::Formula {
		/**
		 * Various errors originating from the Formula parser.
		 */
		struct Parser : std::runtime_error {
			unsigned int offset = 0;

			Parser(const std::string& what, unsigned int offset)
				: std::runtime_error(what), offset(offset)
			{ }
		};

		/**
		 * Logical connectives require their operand formulas to have
		 * the same Domain. This exception is thrown if that precondition
		 * is violated.
		 */
		struct Connective : std::invalid_argument {
			Ast::Type op;
			std::shared_ptr<Domain> lhs;
			std::shared_ptr<Domain> rhs;

			Connective(Ast::Type op, std::shared_ptr<Domain> lhs, std::shared_ptr<Domain> rhs)
				: std::invalid_argument("Arguments to logical connective have different domains"),
				  op(op), lhs(lhs), rhs(rhs)
			{ }
		 };
	}

	class Truthtable;
	class Tseitin;
	class CNF;

	/**
	 * A Formula object represents a memory-managed formula. It consists
	 * of a shared_ptr to the root AST node and to a Domain object which
	 * the internals consult when they need information about variables
	 * appearing in the formula.
	 */
	class Formula {
		std::shared_ptr<Domain> domain;
		std::shared_ptr<Ast>    root;

		friend class Truthtable;
		friend class Tseitin;
		friend class CNF;

	public:
		/**
		 * DefaultDomain is the default global domain for variables used
		 * by the Formula parser.
		 */
		static std::shared_ptr<Cache> DefaultDomain;

		/**
		 * Parse a formula.
		 *
		 * TODO: Describe the syntax and sagemath compatibility.
		 */
		Formula(const std::string& fm, std::shared_ptr<Domain> domain = DefaultDomain);

		/** Wrap existing AST in a formula. */
		Formula(std::shared_ptr<Ast> root, std::shared_ptr<Domain> domain = DefaultDomain) :
			domain(domain),
			root(root)
		{ }

		/**
		 * Convert a Clause into a Formula. Returns a Formula which is a
		 * disjunction of positive or negative variables. If the clause is
		 * empty, the formula consists of a single Ast::Const node, which
		 * is false (false being the identity element with respect to
		 * disjunction).
		 */
		Formula(Clause& cl, std::shared_ptr<Domain> domain);

		/**
		 * Convert a stream of Clause objects into a Formula. Returns a Formula
		 * which is a conjunction of the formulas returned by the Formula
		 * constructor applied to an individual clause. If the Stream has no
		 * element, the constructed formula consists of a single Ast::Const
		 * node, which is true (true being the identity element with respect
		 * to conjunction).
		 */
		Formula(Stream<Clause>& clauses, std::shared_ptr<Domain> domain);

		/** Get a shared_ptr of the Formula's domain. */
		std::shared_ptr<Domain> get_domain() { return domain; }

		/**
		 * Return all variables appearing in the formula sorted in ascending
		 * order by their `Domain.pack` value.
		 */
		std::vector<VarRef> vars(void) const;

		/**
		 * Evaluate the formula on the given assignment. If the assignment
		 * is undefined on a variable that is encountered while evaluating,
		 * an std::out_of_range exception is thrown.
		 *
		 * Beware that evaluation of even a partially defined assignment
		 * can succeed (is not guaranteed to throw an exception), because
		 * conjunction, disjunction and implication short-circuit.
		 */
		bool eval(const Assignment& assign) const { return root->eval(assign); }

		/**
		 * Evaluate the formula on the given (partial) assignment. Without
		 * an assignment, the empty assignment is used. This has the effect
		 * of replacing all variables defined in the assignment with their
		 * values and inductively simplifying the AST nodes involving
		 * constants. The resulting formula is either a sole constant or
		 * does not have any constant nodes or variable nodes referred to
		 * in the assignment anymore.
		 */
		Formula simplify(void) const { return simplify(Assignment()); }
		Formula simplify(const Assignment& assign) const {
			return Formula(root->simplify(assign), domain);
		}

		/** Return a Truthtable stream for the formula. */
		Truthtable truthtable(void) const;
		/** Return a Tseitin transform stream for the formula. */
		Tseitin    tseitin(void)    const;
		/** Return a CNF stream for the formula. */
		CNF        cnf(void)        const;

		/** Return an infix stringification of the formula using a minimal amount of parenthesis. */
		std::string to_infix(void)   const { return root->to_infix();   }
		/** Return the postfix (reverse polish notation) stringification of the formula. */
		std::string to_prefix(void)  const { return root->to_prefix();  }
		/** Return the prefix (polish notation) stringification of the formula. */
		std::string to_postfix(void) const { return root->to_postfix(); }

		/** Construct a new formula negating this one. */
		Formula operator~(void);
		/** Construct a new formula as the conjunction of this one and rhs. */
		Formula operator&(const Formula& rhs);
		/** Construct a new formula as the disjunction of this one and rhs. */
		Formula operator|(const Formula& rhs);
		/** Construct a new formula as the implication of this one and rhs. */
		Formula operator>>(const Formula& rhs);
		/** Construct a new formula as the equivalence of this one and rhs. */
		Formula operator==(const Formula& rhs);
		/** Construct a new formula as the exclusive-or of this one and rhs. */
		Formula operator^(const Formula& rhs);
	};
}

/* Complete the interface of Formula. */
#include <propcalc/truthtable.hpp>
#include <propcalc/tseitin.hpp>
#include <propcalc/cnf.hpp>

#endif /* PROPCALC_FORMULA_HPP */
