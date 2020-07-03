/*
 * formula.hpp - Formula, subformulas, parser
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
#include <vector>
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
	 * A Formula object represents a (memory-managed) formula. It consists
	 * of a vector of Ast objects for the formula in polish notation and a
	 * pointer to a Domain object which the internals consult when they need
	 * information about variables appearing in the formula. The Domain must
	 * be owned externally and outlive the formula.
	 */
	class Formula {
		/* Prefer to stringify Type::Var nodes as their names instead of
		 * their numbers. */
		std::string to_string(Ast ast) const {
			if (ast.type == Ast::Type::Var)
				return domain->unpack(ast.var)->to_string();
			return ast.to_string();
		}

		/* Recursive versions of eval and simplify. */
		bool EVAL(const Assignment& assign, unsigned int root) const;
		std::vector<Ast> SIMPLIFY(const Assignment& assign, unsigned int root = 0) const;

		std::vector<Ast> SIMPLIFY(const Assignment& assign, const std::vector<Ast>& pn2) const {
			return Formula(pn2, domain).SIMPLIFY(assign);
		}

	public:
		Domain* domain;      /**< The formula's domain (externally owned).  */
		std::vector<Ast> pn; /**< Preorder ("polish") traversal of the AST. */

		/**
		 * DefaultDomain is the default global domain for variables used
		 * by the Formula parser.
		 */
		static Cache DefaultDomain;

		/**
		 * Parse a formula in infix form.
		 *
		 * The parser accepts any well-formed formula in propositional
		 * calculus and turns it into a preorder traversal of its AST.
		 * The notion of well-formedness is standard (constants and variables
		 * are atomic terms, atomic terms are terms, terms can be combined
		 * with operators, parentheses may be used). A formula may *not*
		 * produce an empty AST.
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
		 * implication is by convention *right*-associative. When parsing
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
		 * ## On incompatibility with sagemath's propcalc
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

		/** Wrap existing AST (polish notation) in a formula. */
		Formula(std::vector<Ast> pn, Domain* domain = &DefaultDomain) :
			domain(domain),
			pn(pn)
		{ }

		/**
		 * Convert a Clause into a Formula. Returns a Formula which is a
		 * disjunction of positive or negative variables. If the clause is
		 * empty, the formula consists of a single Ast::Type::Const node,
		 * which is false (false being the identity element with respect
		 * to disjunction).
		 */
		Formula(Clause& cl, Domain* domain);

		/**
		 * Convert a stream of Clause objects into a Formula. Returns a Formula
		 * which is a conjunction of the formulas returned by the Formula
		 * constructor applied to an individual clause. If the Stream has no
		 * element, the constructed formula consists of a single Ast::Type::Const
		 * node, which is true (true being the identity element with respect
		 * to conjunction).
		 */
		Formula(Stream<Clause>& clauses, Domain* domain);

		/**
		 * Iterator for the AST nodes in a formula or a subformula. It is
		 * initialized with a pointer to the vector storing the AST nodes
		 * of the backing formula, an index pointing at the root of a sub-
		 * formula and a balance of -1. At any point during the iteration,
		 * balance is the "need" for operands in order to complete the
		 * subformula.
		 * 
		 * The subformula is completed once balance is zero for the first
		 * time. The balance can be updated (bidirectionally) according to
		 * the AST node's arity.
		 *
		 * The end iterator is a sentinel that checks whether the balance
		 * is zero.
		 */
		class iterator {
			const std::vector<Ast>* pn;
			Domain* domain = nullptr;

		public:
			using iterator_category = std::bidirectional_iterator_tag;
			using value_type = Ast;
			using difference_type = std::ptrdiff_t;
			using pointer = Ast*;
			using reference = Ast&;

			unsigned int current;
			int balance = -1;

			iterator(const Formula& fm, unsigned int root = 0) : pn(&fm.pn), domain(fm.domain), current(root) { }
			iterator(const std::vector<Ast>* pn, Domain* domain, unsigned int root) : pn(pn), domain(domain), current(root) { }
			iterator(void) : pn(nullptr), current(0), balance(0) { }

			template<typename I>
			bool operator!=(I b) const {
				if (!b.pn)
					return balance != 0;
				return pn != b.pn || current != b.current;
			}

			template<typename I>
			bool operator==(I b) const {
				if (!b.pn)
					return balance == 0;
				return pn == b.pn && current == b.current;
			}

			Ast operator*(void) const {
				return (*pn)[current];
			}

			const Ast* operator->(void) const {
				return &(*pn)[current];
			}

			iterator& operator++(void) {
				/* Update: subtract (arity - 1) from the balance. This means
				 * that the node we just processed gives us one term, but
				 * requires arity terms to do so. */
				int need = -1 + (int) (*pn)[current].arity;
				balance -= need;
				++current;
				return *this;
			}

			iterator& operator--(void) {
				/* Undo the update above. */
				--current;
				int need = -1 + (int) (*pn)[current].arity;
				balance += need;
				return *this;
			}

			/**
			 * Create a new iterator for the subformula rooted at the
			 * current node. It will use the same formula, have the same
			 * root as the current element but the default balance of -1
			 * making it only iterate the current subtree.
			 */
			iterator sub(void) {
				return iterator(pn, domain, current);
			}

			/**
			 * Return a list of operands. Afterwards, the iterator points
			 * to the node following the current one, or at the end.
			 */
			std::vector<iterator> operands(void) {
				std::vector<iterator> res;
				int need = -(int) (**this).arity;
				for (++*this; balance; ++*this) {
					/* Operands are those nodes before which balance is
					 * the current number of nodes still needed by the
					 * root. At the beginning that is -arity and that
					 * number gets incremented every time an operand is
					 * found. */
					if (balance == need) {
						res.push_back(sub());
						++need;
					}
				}
				return res;
			}

			/**
			 * Create an independent Formula object from the subformula
			 * rooted at the iterator's current node. Afterwards, the
			 * iterator points to the following node (in-order), or to
			 * the end.
			 */
			Formula materialize(void) {
				std::vector<Ast> pn;
				/* End of the subformula is characterized by the first
				 * node where balance becomes current balance + 1. */
				for (auto end = balance + 1; balance != end; ++*this)
					pn.push_back(**this);
				return Formula(pn, domain);
			}
		};

		/** Create a new iterator for the AST nodes of this subformula. */
		iterator begin(unsigned int root = 0) const {
			return iterator(*this, root);
		}

		/** Return a sentinel end iterator. */
		iterator end(void) const {
			return iterator();
		}


		/**
		 * Return all variable numbers appearing in the formula in the order
		 * in which they appear.
		 */
		std::vector<VarNr> varnrs(void) const;

		/**
		 * Return all variables appearing in the formula sorted in ascending
		 * order by their `Domain.pack` value.
		 */
		std::vector<VarRef> vars(void) const;

		/** Return an empty assignment */
		Assignment assignment(void) const { return Assignment(vars()); }

		/**
		 * Evaluate the formula on the given assignment. If the assignment
		 * is undefined on a variable that is encountered while evaluating,
		 * an std::out_of_range exception is thrown.
		 */
		bool eval(const Assignment& assign) const;

		/**
		 * Evaluate the formula on the given assignment. If the assignment
		 * is undefined on a variable that is encountered while evaluating,
		 * an std::out_of_range exception is thrown.
		 *
		 * Unlike eval, the evaluation of even a partially defined assignment
		 * can succeed (is not guaranteed to throw an exception), because
		 * conjunction, disjunction and implication short-circuit in the
		 * implementation of this method.
		 */
		bool EVAL(const Assignment& assign) const {
			return EVAL(assign, 0);
		}

		/**
		 * Evaluate the formula on the given (partial) assignment. This has
		 * the effect of replacing all variables defined in the assignment
		 * with their values and inductively simplifying the AST nodes
		 * involving constants. The resulting formula is either a sole
		 * constant or does not have any constant nodes or variable nodes
		 * referred to in the assignment anymore.
		 *
		 * Without an assignment, the empty assignment is used.
		 */
		Formula simplify(void) const { return simplify(Assignment()); }
		Formula simplify(const Assignment& assign) const {
			return Formula(SIMPLIFY(assign, 0), domain);
		}

		/** Return a Truthtable stream for the formula. */
		Truthtable truthtable(void) const;
		/** Return a Tseitin transform stream for the formula. */
		Tseitin    tseitin(void)    const;
		/** Return a CNF stream for the formula. */
		CNF        cnf(void)        const;

		/** Return the prefix (polish notation) stringification of the formula. */
		std::string to_prefix(void)  const;
		/** Return the postfix (reverse polish notation) stringification of the formula. */
		std::string to_postfix(void) const;
		/** Return an infix stringification of the formula using a minimal amount of parenthesis. */
		std::string to_infix(void)   const;

		/** Access the i-th AST node. */
		Ast operator[](unsigned int i) const { return pn[i]; }

		/** Construct a new formula negating this one. */
		Formula operator~(void);
		/** Construct a new formula as the conjunction of this one and rhs. */
		Formula operator&(const Formula& rhs);
		/** Construct a new formula as the disjunction of this one and rhs. */
		Formula operator|(const Formula& rhs);
		/** Construct a new formula as the exclusive-or of this one and rhs. */
		Formula operator^(const Formula& rhs);

		bool operator==(const Formula& rhs) const {
			return domain == rhs.domain && pn == rhs.pn;
		}
	};
}

namespace std {

template<class T>
static inline void hash_combine(size_t& seed, T const& v);

template<>
struct hash<Propcalc::Formula> {
	size_t operator()(const Propcalc::Formula& fm) const {
		size_t seed = std::hash<Propcalc::Domain*>()(fm.domain);
		for (auto ast : fm)
			hash_combine(seed, std::hash<Propcalc::Ast>()(ast));
		return seed;
	}
};

template<class T>
static inline void hash_combine(size_t& seed, T const& v) {
	seed ^= std::hash<T>()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

}

/* Complete the interface of Formula. */
#include <propcalc/truthtable.hpp>
#include <propcalc/tseitin.hpp>
#include <propcalc/cnf.hpp>

#endif /* PROPCALC_FORMULA_HPP */
