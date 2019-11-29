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

	class Ast {
	public:
		enum class Type {
			Const, Var,
			Not, And, Or,
			Impl, Eqv, Xor
		};

		enum class Assoc {
			Non   = 0x0,
			Left  = 0x1,
			Right = 0x2,
			Both  = Left | Right
		};

		/*
		 * Careful! These values must order the same as in the parser's
		 * operator table in formula.cpp.
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

		virtual Ast::Type  type(void)  const = 0;
		virtual Ast::Assoc assoc(void) const = 0;
		virtual Ast::Prec  prec(void)  const = 0;

		virtual void fill_vars(std::unordered_set<const Variable*>& pile) const = 0;
		virtual bool eval(const Assignment& assign) const = 0;

		virtual std::string to_infix(void)   const = 0;
		virtual std::string to_prefix(void)  const = 0;
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

		virtual void fill_vars(std::unordered_set<const Variable*>& pile) const { }
		virtual bool eval(const Assignment& assign) const { return value; }

		virtual std::string to_string(void)  const { return value ? "\\T" : "\\F"; }
		virtual std::string to_infix(void)   const { return this->to_string(); }
		virtual std::string to_prefix(void)  const { return this->to_string(); }
		virtual std::string to_postfix(void) const { return this->to_string(); }
	};

	class Ast::Var : public Ast {
	public:
		const Variable* var;

		Var(const Variable* var) : var(var) { }

		virtual Ast::Type  type(void)  const { return Ast::Type::Var;      }
		virtual Ast::Assoc assoc(void) const { return Ast::Assoc::Non;     }
		virtual Ast::Prec  prec(void)  const { return Ast::Prec::Symbolic; }

		virtual void fill_vars(std::unordered_set<const Variable*>& pile) const {
			pile.insert(var);
		}

		virtual bool eval(const Assignment& assign) const { return assign[var]; }

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

		virtual void fill_vars(std::unordered_set<const Variable*>& pile) const {
			rhs->fill_vars(pile);
		}

		virtual bool eval(const Assignment& assign) const { return ! rhs->eval(assign); }

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

		virtual void fill_vars(std::unordered_set<const Variable*>& pile) const {
			lhs->fill_vars(pile);
			rhs->fill_vars(pile);
		}

		virtual bool eval(const Assignment& assign) const { return lhs->eval(assign) && rhs->eval(assign); }

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

		virtual void fill_vars(std::unordered_set<const Variable*>& pile) const {
			lhs->fill_vars(pile);
			rhs->fill_vars(pile);
		}

		virtual bool eval(const Assignment& assign) const { return lhs->eval(assign) || rhs->eval(assign); }

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

		virtual void fill_vars(std::unordered_set<const Variable*>& pile) const {
			lhs->fill_vars(pile);
			rhs->fill_vars(pile);
		}

		virtual bool eval(const Assignment& assign) const { return !lhs->eval(assign) || rhs->eval(assign); }

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

		virtual void fill_vars(std::unordered_set<const Variable*>& pile) const {
			lhs->fill_vars(pile);
			rhs->fill_vars(pile);
		}

		virtual bool eval(const Assignment& assign) const { return lhs->eval(assign) == rhs->eval(assign); }

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

		virtual void fill_vars(std::unordered_set<const Variable*>& pile) const {
			lhs->fill_vars(pile);
			rhs->fill_vars(pile);
		}

		virtual bool eval(const Assignment& assign) const { return lhs->eval(assign) != rhs->eval(assign); }

		virtual std::string to_infix(void)   const;
		virtual std::string to_prefix(void)  const;
		virtual std::string to_postfix(void) const;
	};
}

#endif /* PROPCALC_AST_HPP */
