/*
 * ast.hpp - AST classes
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

#ifndef PROPCALC_AST_HPP
#define PROPCALC_AST_HPP

#include <string>
#include <memory>
#include <vector>

#include <propcalc/variable.hpp>
#include <propcalc/assignment.hpp>

namespace Propcalc {
	/**
	 * Ast is the base class for all AST nodes. It contains virtual methods
	 * to identify the type of node, for stringification and evaluation,
	 * which have to be overloaded to provide the specific node's semantics.
	 */
	class Ast {
	public:
		/**
		 * Types of Ast nodes. These enum values are in 1-to-1 correspondence
		 * with the subclasses of Ast.
		 *
		 * TODO: In the future, this may be removed in favor of `typeid`.
		 */
		enum class Type {
			Const, Var,
			Not, And, Or,
			Impl, Eqv, Xor
		};

		/**
		 * Bitmasks for associativity.
		 */
		enum class Assoc {
			Non   = 0x0,
			Left  = 0x1,
			Right = 0x2,
			Both  = Left | Right
		};

		/**
		 * Precedence numbers, the higher the tighter.
		 */
		enum class Prec {
			Tight    = 20,
			Symbolic = Tight,
			Notish   = 14,
			Andish   = 12,
			Orish    = 10,
			Implish  = 8,
			Eqvish   = 6,
			Xorish   = Eqvish,
			Loose    = 0
		};

		/** Return this node's Ast::Type. */
		virtual Ast::Type  type(void)  const = 0;
		/** Return this node's Ast::Assoc. */
		virtual Ast::Assoc assoc(void) const = 0;
		/** Return this node's Ast::Prec. */
		virtual Ast::Prec  prec(void)  const = 0;

		/**
		 * Evaluate the subtree rooted at this node on the given assignment.
		 * If the assignment is undefined on a variable that is encountered
		 * while evaluating, an std::out_of_range exception is thrown.
		 *
		 * Beware that evaluation of even a partially defined assignment
		 * can succeed (is not guaranteed to throw an exception), because
		 * conjunction, disjunction and implication short-circuit.
		 */
		virtual bool eval(const Assignment& assign) const = 0;

		/**
		 * Evaluate the subtree rooted at this node on the given (partial)
		 * assignment. This has the effect of replacing all variables
		 * defined in the assignment with their values and inductively
		 * simplifying the AST nodes involving constants. The resulting
		 * formula is either a sole constant or does not have any constant
		 * nodes or variable nodes referred to in the assignment anymore.
		 */
		virtual std::shared_ptr<Ast> simplify(const Assignment& assign) const = 0;

		/** Convert subtree to infix. */
		virtual std::string to_infix(void)   const = 0;
		/** Convert subtree to prefix (polish notation). */
		virtual std::string to_prefix(void)  const = 0;
		/** Convert subtree to postfix (reverse polish notation). */
		virtual std::string to_postfix(void) const = 0;

		class Const;
		class Var;
		class Not;
		class And;
		class Or;
		class Impl;
		class Eqv;
		class Xor;
	};

	class Ast::Const : public Ast {
	public:
		bool value;

		Const(bool value) : value(value) { }

		virtual Ast::Type  type(void)  const { return Ast::Type::Const;    }
		virtual Ast::Assoc assoc(void) const { return Ast::Assoc::Non;     }
		virtual Ast::Prec  prec(void)  const { return Ast::Prec::Symbolic; }

		virtual bool eval(const Assignment&) const { return value; }
		virtual std::shared_ptr<Ast> simplify(const Assignment&) const {
			return std::make_shared<Ast::Const>(value);
		}

		virtual std::string to_string(void)  const { return value ? "\\T" : "\\F"; }
		virtual std::string to_infix(void)   const { return this->to_string(); }
		virtual std::string to_prefix(void)  const { return this->to_string(); }
		virtual std::string to_postfix(void) const { return this->to_string(); }
	};

	class Ast::Var : public Ast {
	public:
		VarRef var;

		Var(VarRef var) : var(var) { }

		virtual Ast::Type  type(void)  const { return Ast::Type::Var;      }
		virtual Ast::Assoc assoc(void) const { return Ast::Assoc::Non;     }
		virtual Ast::Prec  prec(void)  const { return Ast::Prec::Symbolic; }

		virtual bool eval(const Assignment& assign) const { return assign[var]; }

		/* TODO: the way this is designed makes sharing nodes hard.
		 * We don't do this at the moment and always allocate. */
		virtual std::shared_ptr<Ast> simplify(const Assignment& assign) const {
			if (assign.exists(var))
				return std::make_shared<Ast::Const>(assign[var]);
			return std::make_shared<Ast::Var>(var);
		}

		virtual std::string to_string(void)  const { return var->to_string(); }
		virtual std::string to_infix(void)   const { return var->to_string(); }
		virtual std::string to_prefix(void)  const { return var->to_string(); }
		virtual std::string to_postfix(void) const { return var->to_string(); }
	};

	class Ast::Not : public Ast {
	public:
		std::shared_ptr<Ast> rhs;

		Not(std::shared_ptr<Ast> rhs) : rhs(rhs) { }

		virtual Ast::Type  type(void)  const { return Ast::Type::Not;    }
		virtual Ast::Assoc assoc(void) const { return Ast::Assoc::Non;   }
		virtual Ast::Prec  prec(void)  const { return Ast::Prec::Notish; }

		virtual bool eval(const Assignment& assign) const { return ! rhs->eval(assign); }

		virtual std::shared_ptr<Ast> simplify(const Assignment& assign) const;

		virtual std::string to_infix(void)   const;
		virtual std::string to_prefix(void)  const;
		virtual std::string to_postfix(void) const;
	};

	class Ast::And : public Ast {
	public:
		std::shared_ptr<Ast> lhs;
		std::shared_ptr<Ast> rhs;

		And(std::shared_ptr<Ast> lhs, std::shared_ptr<Ast> rhs) : lhs(lhs), rhs(rhs) { }

		virtual Ast::Type  type(void)  const { return Ast::Type::And;    }
		virtual Ast::Assoc assoc(void) const { return Ast::Assoc::Both;  }
		virtual Ast::Prec  prec(void)  const { return Ast::Prec::Andish; }

		virtual bool eval(const Assignment& assign) const { return lhs->eval(assign) && rhs->eval(assign); }

		virtual std::shared_ptr<Ast> simplify(const Assignment& assign) const;

		virtual std::string to_infix(void)   const;
		virtual std::string to_prefix(void)  const;
		virtual std::string to_postfix(void) const;
	};

	class Ast::Or : public Ast {
	public:
		std::shared_ptr<Ast> lhs;
		std::shared_ptr<Ast> rhs;

		Or(std::shared_ptr<Ast> lhs, std::shared_ptr<Ast> rhs) : lhs(lhs), rhs(rhs) { }

		virtual Ast::Type  type(void)  const { return Ast::Type::Or;    }
		virtual Ast::Assoc assoc(void) const { return Ast::Assoc::Both; }
		virtual Ast::Prec  prec(void)  const { return Ast::Prec::Orish; }

		virtual bool eval(const Assignment& assign) const { return lhs->eval(assign) || rhs->eval(assign); }

		virtual std::shared_ptr<Ast> simplify(const Assignment& assign) const;

		virtual std::string to_infix(void)   const;
		virtual std::string to_prefix(void)  const;
		virtual std::string to_postfix(void) const;
	};

	class Ast::Impl : public Ast {
	public:
		std::shared_ptr<Ast> lhs;
		std::shared_ptr<Ast> rhs;

		Impl(std::shared_ptr<Ast> lhs, std::shared_ptr<Ast> rhs) : lhs(lhs), rhs(rhs) { }

		virtual Ast::Type  type(void)  const { return Ast::Type::Impl;    }
		virtual Ast::Assoc assoc(void) const { return Ast::Assoc::Right;  }
		virtual Ast::Prec  prec(void)  const { return Ast::Prec::Implish; }

		virtual bool eval(const Assignment& assign) const { return !lhs->eval(assign) || rhs->eval(assign); }

		virtual std::shared_ptr<Ast> simplify(const Assignment& assign) const;

		virtual std::string to_infix(void)   const;
		virtual std::string to_prefix(void)  const;
		virtual std::string to_postfix(void) const;
	};

	class Ast::Eqv : public Ast {
	public:
		std::shared_ptr<Ast> lhs;
		std::shared_ptr<Ast> rhs;

		Eqv(std::shared_ptr<Ast> lhs, std::shared_ptr<Ast> rhs) : lhs(lhs), rhs(rhs) { }

		virtual Ast::Type  type(void)  const { return Ast::Type::Eqv;    }
		virtual Ast::Assoc assoc(void) const { return Ast::Assoc::Both;  }
		virtual Ast::Prec  prec(void)  const { return Ast::Prec::Eqvish; }

		virtual bool eval(const Assignment& assign) const { return lhs->eval(assign) == rhs->eval(assign); }

		virtual std::shared_ptr<Ast> simplify(const Assignment& assign) const;

		virtual std::string to_infix(void)   const;
		virtual std::string to_prefix(void)  const;
		virtual std::string to_postfix(void) const;
	};

	class Ast::Xor : public Ast {
	public:
		std::shared_ptr<Ast> lhs;
		std::shared_ptr<Ast> rhs;

		Xor(std::shared_ptr<Ast> lhs, std::shared_ptr<Ast> rhs) : lhs(lhs), rhs(rhs) { }

		virtual Ast::Type  type(void)  const { return Ast::Type::Xor;    }
		virtual Ast::Assoc assoc(void) const { return Ast::Assoc::Both;  }
		virtual Ast::Prec  prec(void)  const { return Ast::Prec::Xorish; }

		virtual bool eval(const Assignment& assign) const { return lhs->eval(assign) != rhs->eval(assign); }

		virtual std::shared_ptr<Ast> simplify(const Assignment& assign) const;

		virtual std::string to_infix(void)   const;
		virtual std::string to_prefix(void)  const;
		virtual std::string to_postfix(void) const;
	};
}

#endif /* PROPCALC_AST_HPP */
