/*
 * variable.hpp - Variable and Domain
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

#ifndef PROPCALC_VARIABLE_HPP
#define PROPCALC_VARIABLE_HPP

#include <string>
#include <memory>
#include <vector>
#include <mutex>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>

namespace Propcalc {
	class Variable {
	public:
		std::string name;

		Variable(std::string name) : name(name) { }
		Variable(const char* s, size_t len) : name(s, len) { }
		virtual ~Variable(void) { }

		virtual std::string to_string(void) const { return "[" + name + "]"; }
	};

	using VarNr  = unsigned int;
	using VarRef = const Variable*;

	class Domain {
	public:
		virtual std::string name(VarRef var) { return var->name; }

		virtual VarRef resolve(std::string name) = 0;
		virtual VarNr  pack(VarRef var)          = 0;
		virtual VarRef unpack(VarNr nr)          = 0;

		virtual std::vector<VarRef> list(void) const = 0;

		virtual size_t size(void) const {
			return list().size();
		}

		virtual std::vector<VarRef> sort(std::unordered_set<VarRef>& pile) {
			std::vector<VarRef> sorted(pile.size());
			std::copy(pile.begin(), pile.end(), sorted.begin());

			std::sort(sorted.begin(), sorted.end(),
				[&] (VarRef a, VarRef b) -> bool {
					return pack(a) < pack(b);
			});

			return sorted;
		}
	};

	class Cache : public Domain {
	private:
		std::vector<std::unique_ptr<Variable>> cache;
		std::unordered_map<std::string, VarRef> by_name;
		std::vector<VarRef> by_nr;
		std::unordered_map<VarRef, VarNr> by_ref;

	protected:
		mutable std::mutex access;
		std::pair<VarNr, VarRef> new_variable(std::string name);
		std::pair<VarNr, VarRef> put_variable(std::unique_ptr<Variable> uvar);

	public:
		virtual ~Cache(void) { }

		virtual VarRef resolve(std::string name);
		virtual VarNr  pack(VarRef var);
		virtual VarRef unpack(VarNr nr);

		virtual std::vector<VarRef> list(void) const;
		virtual size_t size(void) const;
		virtual std::vector<VarRef> sort(std::unordered_set<VarRef>& pile) const;
	};
}

#endif /* PROPCALC_VARIABLE_HPP */
