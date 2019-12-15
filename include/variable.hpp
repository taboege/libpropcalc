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
#include <map>
#include <unordered_set>

namespace Propcalc {
	class Variable {
	public:
		std::string name;

		Variable(std::string name) : name(name) { }
		Variable(const char* s, size_t len) : name(s, len) { }

		virtual std::string to_string(void) const { return "[" + name + "]"; }
	};

	using VarRef = const Variable*;

	class Domain {
	public:
		virtual VarRef resolve(std::string name)     = 0;
		virtual std::vector<VarRef> list(void) const = 0;

		virtual std::vector<VarRef> sort(
			std::unordered_set<VarRef>& pile
		) const = 0;
	};

	class Cache : public Domain {
	private:
		mutable std::mutex access;
		std::map<std::string, std::unique_ptr<Variable>> cache;
		std::vector<VarRef> order;

	public:
		virtual VarRef resolve(std::string name);
		virtual std::vector<VarRef> list(void) const;

		virtual std::vector<VarRef> sort(
			std::unordered_set<VarRef>& pile
		) const;
	};
}

#endif /* PROPCALC_VARIABLE_HPP */
