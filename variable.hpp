/*
 * variable.hpp - Variable and Domain
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

#ifndef PROPCALC_VARIABLE_HPP
#define PROPCALC_VARIABLE_HPP

#include <string>
#include <memory>
#include <vector>
#include <mutex>
#include <unordered_map>

#include <propcalc/ast.hpp>

namespace Propcalc {
	class Variable : public Ast {
	public:
		std::string name;

		Variable(std::string name) : name(name) { }
		Variable(const char *s, size_t len) : name(std::string(s, len)) { }

		virtual Ast::Type  type(void)  const { return Ast::Type::Variable; }
		virtual Ast::Assoc assoc(void) const { return Ast::Assoc::Non;     }
		virtual Ast::Prec  prec(void)  const { return Ast::Prec::Symbolic; }

		virtual std::string to_string(void)  const { return "[" + name + "]";  }
		virtual std::string to_infix(void)   const { return this->to_string(); }
		virtual std::string to_prefix(void)  const { return this->to_string(); }
		virtual std::string to_postfix(void) const { return this->to_string(); }
	};

	class Domain {
	public:
		virtual std::shared_ptr<Variable> get(std::string name)   = 0;
		virtual std::vector<std::shared_ptr<Variable>> list(void) = 0;
		virtual size_t get_nr(std::string name)                   = 0;
		virtual size_t get_nr(std::shared_ptr<Variable> var)      = 0;

		/*
		 * TODO: Tseitin transform wants to introduce new temporary
		 * variables. The domain itself has to support this,
		 * i.e. we don't want to wrap the domain into a "Tseitin
		 * domain wrapper" just to get access to temporary vars,
		 * because then we couldn't compose formulas anymore,
		 * because that needs identical domains. The outside should
		 * dictate what class the temporaries have...
		 */
		//virtual ??? temp(???) = 0;
	};

	class Cache : public Domain {
	private:
		std::mutex update;
		std::unordered_map<
			std::string,
			std::pair<
				std::shared_ptr<Variable>,
				size_t
			>
		> cache;

	public:
		virtual std::shared_ptr<Variable> get(std::string name);
		virtual std::vector<std::shared_ptr<Variable>> list(void);
		virtual size_t get_nr(std::string name);
		virtual size_t get_nr(std::shared_ptr<Variable> var);
	};
}

#endif /* PROPCALC_VARIABLE_HPP */
