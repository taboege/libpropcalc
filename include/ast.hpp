/*
 * ast.hpp - Ast class
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

#ifndef PROPCALC_AST_HPP
#define PROPCALC_AST_HPP

#include <string>

#include <propcalc/variable.hpp>

namespace Propcalc {
	/**
	 * Ast is a data structure for storing a node in the preorder traversal
	 * of an abstract syntax tree of a formula. It contains plain old data
	 * without pointers and ownership. On common platforms it takes up only
	 * 4 + 4*1 = 8 bytes.
	 *
	 * Even though all attributes are public and could be changed, you
	 * shouldn't do that. The library may use certain internal assumptions
	 * about the properties of operators it knows, and those it doesn't
	 * know it can't process. Instead, the purpose of keeping the attributes
	 * around publicly in every object is simplicity and having everything
	 * available locally in the AST inside a Formula object.
	 */
	struct Ast {
		/**
		 * Types of Ast nodes.
		 */
		enum class Type : unsigned char {
			Const = 0, Var,
			Not, And, Or,
			Impl, Eqv, Xor
		};

		Type type; /**< The node type. */

		/**
		 * Symolic names for arities.
		 */
		enum class Arity : unsigned char {
			Leaf   = 0,
			Unary  = 1,
			Binary = 2
		};

		Arity arity; /**< The arity of this node. */

		/**
		 * Precedence numbers, the higher the tighter.
		 */
		enum class Prec : unsigned char {
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

		Prec prec; /**< The precedence of this node. */

		/**
		 * Bitmasks for associativity.
		 */
		enum class Assoc : unsigned char {
			Non   = 0x0,
			Left  = 0x1,
			Right = 0x2,
			Both  = Left | Right
		};

		Assoc assoc; /**< The associativity of this node. */

		/**
		 * Payload, if any.
		 */
		union {
			bool value; /**< The value of a Const node. */
			VarNr var;  /**< The VarNr of a Var node.   */
		};

		/**
		 * Return a string representation of this node. Type::Const nodes
		 * become "\T" or "\F", Type::Var stringify to their VarNr, and
		 * the operator nodes have fixed string representations.
		 *
		 * The caller may want to use its Domain object to get the name
		 * of a variable node instead of a stringification of its number.
		 */
		std::string to_string(void) const {
			static std::string table[] = {
				"C?", "V?", /* Const and Var are not tabulated */
				"~", "&", "|", ">", "=", "^"
			};
			switch (type) {
				case Type::Const:
					return value ? "\\T" : "\\F";
				case Type::Var:
					return std::to_string(var);
				case Type::Not:
				case Type::And:
				case Type::Or:
				case Type::Impl:
				case Type::Eqv:
				case Type::Xor:
					return table[(unsigned int) type];
				default:
					return "???"; /* silence compiler */
			}
		}

		bool operator==(const Ast& rhs) const {
			bool res =
				type  == rhs.type  &&
				arity == rhs.arity &&
				prec  == rhs.prec  &&
				assoc == rhs.assoc;
			if (type == Ast::Type::Const)
				res &= value == rhs.value;
			else if (type == Ast::Type::Var)
				res &= var == rhs.var;
			return res;
		}
	};
}

namespace std {

template<>
struct hash<Propcalc::Ast> {
	size_t operator()(const Propcalc::Ast& ast) const {
		return ((size_t) ast.var  << 16)
			 | ((size_t) ast.prec << 8)
			 | ((size_t) ast.type << 4)
			 | ((size_t) ast.assoc);
	}
};

}

#endif /* PROPCALC_AST_HPP */
