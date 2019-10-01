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

#include <iostream>

namespace Propcalc {

	class Ast {
	public:
		virtual std::string to_pn(void)  const = 0;
		virtual std::string to_rpn(void) const = 0;

		class Const;
		class Var;
		class Sym;
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

		Const(bool value);

		virtual std::string to_pn(void)  const;
		virtual std::string to_rpn(void) const;
	};

	class Ast::Var : public Ast {
	public:
		unsigned int nr;

		Var(unsigned int nr);

		virtual std::string to_pn(void)  const;
		virtual std::string to_rpn(void) const;
	};

	class Ast::Sym : public Ast {
	public:
		std::string str;
		unsigned int nr;
		void *obj;

		Sym(std::string str);
		Sym(const char *s, size_t len);

		virtual std::string to_pn(void)  const;
		virtual std::string to_rpn(void) const;
	};

	class Ast::Not : public Ast {
	public:
		std::shared_ptr<Ast> rhs;

		Not(std::shared_ptr<Ast> rhs);

		virtual std::string to_pn(void)  const;
		virtual std::string to_rpn(void) const;
	};

	class Ast::And : public Ast {
	public:
		std::shared_ptr<Ast> lhs;
		std::shared_ptr<Ast> rhs;

		And(std::shared_ptr<Ast> lhs, std::shared_ptr<Ast> rhs);

		virtual std::string to_pn(void)  const;
		virtual std::string to_rpn(void) const;
	};

	class Ast::Or : public Ast {
	public:
		std::shared_ptr<Ast> lhs;
		std::shared_ptr<Ast> rhs;

		Or(std::shared_ptr<Ast> lhs, std::shared_ptr<Ast> rhs);

		virtual std::string to_pn(void)  const;
		virtual std::string to_rpn(void) const;
	};

	class Ast::Impl : public Ast {
	public:
		std::shared_ptr<Ast> lhs;
		std::shared_ptr<Ast> rhs;

		Impl(std::shared_ptr<Ast> lhs, std::shared_ptr<Ast> rhs);

		virtual std::string to_pn(void)  const;
		virtual std::string to_rpn(void) const;
	};

	class Ast::Eqv : public Ast {
	public:
		std::shared_ptr<Ast> lhs;
		std::shared_ptr<Ast> rhs;

		Eqv(std::shared_ptr<Ast> lhs, std::shared_ptr<Ast> rhs);

		virtual std::string to_pn(void)  const;
		virtual std::string to_rpn(void) const;
	};

	class Ast::Xor : public Ast {
	public:
		std::shared_ptr<Ast> lhs;
		std::shared_ptr<Ast> rhs;

		Xor(std::shared_ptr<Ast> lhs, std::shared_ptr<Ast> rhs);

		virtual std::string to_pn(void)  const;
		virtual std::string to_rpn(void) const;
	};
}

#endif /* PROPCALC_AST_HPP */
