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
#include <propcalc/conjunctive.hpp>

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
			Propcalc::Domain* lhs;
			Propcalc::Domain* rhs;

			Connective(Ast::Type op, Propcalc::Domain* lhs, Propcalc::Domain* rhs)
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
	public:
		Domain* domain;
		std::shared_ptr<Ast> root;

		/**
		 * DefaultDomain is the default global domain for variables used
		 * by the Formula parser.
		 */
		static Cache DefaultDomain;

		/**
		 * Parse a formula in infix form.
		 *
		 * The parser accepts any well-formed formula in propositional
		 * calculus and turns it into an AST. The notion of well-formedness
		 * is standard (constants and variables are atomic terms,
		 * atomic terms are terms, terms can be combined with operators,
		 * parentheses may be used). A formula may *not* produce an
		 * empty AST.
		 *
		 * Except inside of atomic tokens like variable names or operators,
		 * whitespace is insignificant.
		 *
		 * ## Operators
		 *
		 * The following well-known logical connectives are supported:
		 *
		 * - `~`: unary negation
		 * - `&`: conjunction
		 * - `|`: disjunction
		 * - `->` or `>`: implication
		 * - `<->` or `=`: equivalence
		 * - `^`: exclusive or
		 *
		 * These operators are listed in order of looser precedence:
		 * `~` binds the tighest, followed by `&` and so on. All of them
		 * *except* for `=` and `^` have distinct precedence levels.
		 * At the lower end, `=` and `^` together have the loosest
		 * precedence.
		 *
		 * All binary operators except for implication are associative,
		 * implication is by convention *right*-associative.  When parsing
		 * formulas (in infix form), the parser assembles an AST where all
		 * chained operators associate to the right, i.e. `a & b & c`
		 * would parse as `a & (b & c)` and produce this AST:
		 *
		 *          &
		 *         / \
		 *        /   &
		 *       /   / \
		 *      a   b   c
		 *
		 * ## Variables and constants
		 *
		 * Variables are strings that begin with an alphanumeric character
		 * and consist entirely of alphanumerics or underscores. It is legal
		 * to make a variable called `3` or `3_4` or `xyz` or `a25`.
		 * If more complicated/free-form variable names are desired, place
		 * them inside square brackets. After an opening square bracket,
		 * anything ASCII goes, until the closing square bracket. So `[_]`,
		 * `[12|47]`, `[Once upon a Time...]` are also legal variables.
		 *
		 * The constants *true* and *false* can be encoded as `\T` and `\F`.
		 * Mind the leading backslash to distinguish them from variables.
		 *
		 * ## Compatibility with sagemath's propcalc
		 *
		 * This library is developed as a replacement for the propcalc
		 * package of sagemath, but even the formula parser is not 100%
		 * compatible. All the operator symbols of sagemath-propcalc are
		 * supported, their variable notation is supported as well.
		 * However, the implementations of `to_{pre,post,in}fix` prefer
		 * the non-sagemath variants of operators and variables.
		 *
		 * A deeper issue concerns precedence and associativity. In sage's
		 * propcalc, all binary operators are *left*-associative. This means
		 * that AST will almost always be different between them and us.
		 * Since there is one non-associative connective, implication,
		 * this also means that the semantics of formulas are incompatible.
		 *
		 * The precedence list of libpropcalc is
		 *
		 *     `~`  >  `&`  >  `|`  >  `->`  >  `<->` = `^`
		 *
		 * whereas sagemath's is
		 *
		 *     `~`  >  `&`  >  `|`  >  `^`  > `->`  >  `<->`
		 *
		 * As far as I am aware, there is no authoritative source on the
		 * precedence and associativity of these connectives (although
		 * implication has always been right-associative when I encountered
		 * it). The low precedence of `^` probably makes sense if you think
		 * of it as "addition mod 2". In libpropcalc, its precedence is
		 * justified by being "not-equal".
		 *
		 * Please pay attention to these differences when porting code.
		 */
		Formula(const std::string& fm, Domain* domain = &DefaultDomain);

		/** Wrap existing AST in a formula. */
		Formula(std::shared_ptr<Ast> root, Domain* domain = &DefaultDomain) :
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
		Formula(Clause& cl, Domain* domain);

		/**
		 * Convert a conjunctive stream of clauses into a Formula. Returns
		 * a Formula which is a conjunction of the formulas returned by the
		 * Formula constructor applied to an individual clause. If the
		 * conjunctive has no element, the constructed formula consists of
		 * a single Ast::Const node, which is true (true being the identity
		 * element with respect to conjunction).
		 */
		Formula(Conjunctive& clauses, Domain* domain);

		/**
		 * Return all variables appearing in the formula sorted in ascending
		 * order by their `Domain.pack` value.
		 */
		std::vector<VarRef> vars(void) const;

		/** Return an empty assignment */
		Assignment assignment(void) const {
			return Assignment(vars());
		}

		/**
		 * Evaluate the root Ast node on the assignment. The same comments
		 * apply.
		 */
		bool eval(const Assignment& assign) const { return root->eval(assign); }

		/**
		 * Partially evaluate the root Ast node on the (partial) assignment.
		 * Without an assignment, the empty assignment is used. The same
		 * comments as for Ast::simplify apply.
		 */
		Formula simplify(void) const { return simplify(Assignment()); }
		Formula simplify(const Assignment& assign) const {
			return Formula(root->simplify(assign), domain);
		}

		/** Return a Truthtable stream for the formula. */
		Truthtable truthtable(bool caching = false) const;
		/** Return a Tseitin transform stream for the formula. */
		Tseitin    tseitin(bool caching = false) const;
		/** Return a CNF stream for the formula. */
		CNF        cnf(bool caching = false) const;

		/** Return an infix stringification of the formula using a minimal amount of parenthesis. */
		std::string to_infix(void)   const { return root->to_infix();   }
		/** Return the postfix (reverse polish notation) stringification of the formula. */
		std::string to_prefix(void)  const { return root->to_prefix();  }
		/** Return the prefix (polish notation) stringification of the formula. */
		std::string to_postfix(void) const { return root->to_postfix(); }

		/** Construct a new formula negating this one. */
		Formula notf(void) const;
		/** Construct a new formula as the conjunction of this one and rhs. */
		Formula andf(const Formula& rhs) const;
		/** Construct a new formula as the disjunction of this one and rhs. */
		Formula orf(const Formula& rhs) const;
		/** Construct a new formula as the implication of this one and rhs. */
		Formula thenf(const Formula& rhs) const;
		/** Construct a new formula as the equivalence of this one and rhs. */
		Formula eqvf(const Formula& rhs) const;
		/** Construct a new formula as the exclusive-or of this one and rhs. */
		Formula xorf(const Formula& rhs) const;

		Formula operator~(void) const { return notf(); }
		Formula operator&(const Formula& rhs) const { return andf(rhs); }
		Formula operator|(const Formula& rhs) const { return orf(rhs);  }
		Formula operator^(const Formula& rhs) const { return xorf(rhs); }

		bool operator==(const Formula& rhs) {
			return domain == rhs.domain && root->equals(*rhs.root);
		}
	};
}

/* Complete the interface of Formula. */
#include <propcalc/truthtable.hpp>
#include <propcalc/tseitin.hpp>
#include <propcalc/cnf.hpp>

#endif /* PROPCALC_FORMULA_HPP */
