/*
 * domain.hpp - Variable and Domain
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

#ifndef PROPCALC_DOMAIN_HPP
#define PROPCALC_DOMAIN_HPP

#include <string>
#include <memory>
#include <vector>
#include <mutex>
#include <stdexcept>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>

namespace Propcalc {
	/**
	 * A propositional variable in libpropcalc is an object derived from the
	 * Variable class. It needs at least a name, which uniquely identifies
	 * it in its Domain.
	 */
	class Variable {
	public:
		std::string name;

		Variable(std::string name) : name(name) { }
		Variable(const char* s, size_t len) : name(s, len) { }
		virtual ~Variable(void) { }

		virtual std::string to_string(void) const { return "[" + name + "]"; }
	};

	/**
	 * Reference to a Variable.
	 */
	using VarRef = const Variable*;

	/**
	 * Given a Domain, a Variable can be equivalently be represented by
	 * its VarNr. This is an unsigned integer strictly greater than zero.
	 * The domain can Domain::pack a VarRef to a VarNr and Domain::unpack
	 * in reverse. The VarNr is used by algorithms which need a total
	 * ordering on the variables, and need it frequently. The DIMACS
	 * converters are such an example and this format is also the reason
	 * why a VarNr of a valid Variable should never be zero. SAT solvers
	 * are another barrier of conversion between VarRef and VarNr.
	 *
	 * Since normally the VarRef gives you access to an object less
	 * indirectly, you should prefer using that.
	 */
	using VarNr = unsigned int;

	namespace X::Domain {
		/**
		 * This exception is thrown when a VarNr of zero is encountered.
		 */
		struct InvalidVarNr : std::out_of_range {
			InvalidVarNr(void) : std::out_of_range("VarNr must be at least 1") { }
		};
	}

	/**
	 * libpropcalc was designed with most eyes towards applications
	 * of propositional calculus, in particular exploiting SAT solvers
	 * to search for combinatorial objects with properties that can be
	 * expressed with propositional calculus. In such applications,
	 * each variable represents a domain-specific switch. A Domain
	 * object represents this domain-specificness.
	 *
	 * The Domain manages the universe of variables relevant for an
	 * application. It allocates new variables and keeps track of them.
	 * It is the Domain object which manages the Variable's lifetime.
	 * It provides algorithms in the library with a total ordering
	 * and lookup procedures on the variable universe.
	 */
	class Domain {
	public:
		virtual std::string name(VarRef var) { return var->name; }

		/** Return a variable from its name. */
		virtual VarRef resolve(std::string name) = 0;
		/** Convert a variable to its 1-based ID number. */
		virtual VarNr  pack(VarRef var)          = 0;
		/** Convert a variable number to the object. */
		virtual VarRef unpack(VarNr nr)          = 0;

		/**
		 * Return the universe, i.e. a list of all variables in the
		 * domain, ordered by their VarNr (`pack` value). */
		virtual std::vector<VarRef> list(void) const = 0;

		/** Size of the domain's universe. */
		virtual size_t size(void) const {
			return list().size();
		}

		/** Take a set of variables and return them ordered by VarNr. */
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

	namespace X::Cache {
		/**
		 * This error is thrown when a frozen Cache object would need to
		 * modify itself to fulfill a `resolve` or `unpack` request.
		 */
		struct Frozen : std::logic_error {
			Frozen(void) : std::logic_error("Cache is frozen") { }
		};
	}

	/**
	 * Cache is a generic implementation of Domain.
	 *
	 * For each new string that is to be `resolve`d, a new generic Variable
	 * object for that name is allocated and cached. Subsequent `resolve`s
	 * of this name will return the same variable object.
	 *
	 * Variable numbers are then just (shifted, 1-based) indices of the
	 * objects in the cache. The `pack` and `unpack` methods have their
	 * own redundant caches to make each lookup amortized constant time.
	 *
	 * A request to `unpack` a high variable number will result in all the
	 * missing variables to be allocated. Their names are just the decimal
	 * representations of their respective VarNr.
	 *
	 * Cache tries to make all requests for variables succeed somehow and
	 * always answers consistently. For this reason it can be used as the
	 * default domain for any operation where the concept of domain is not
	 * required or the particular domain is unknown, such as when reading
	 * a generic DIMACS CNF file.
	 */
	class Cache : public Domain {
	private:
		std::vector<std::unique_ptr<Variable>> cache;
		std::unordered_map<std::string, VarRef> by_name;
		std::vector<VarRef> by_nr;
		std::unordered_map<VarRef, VarNr> by_ref;
		bool frozen = false;

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

		/** Mark the Cache as immutable. No more variables will be created. */
		void freeze(void);
		/** Undo `freeze`. */
		void thaw(void);
	};
}

#endif /* PROPCALC_DOMAIN_HPP */
