/*
 * dimacs.hpp - DIMACS CNF files
 *
 * Copyright (C) 2020 Tobias Boege
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the Artistic License 2.0
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * Artistic License 2.0 for more details.
 */

#ifndef PROPCALC_DIMACS_HPP
#define PROPCALC_DIMACS_HPP

#include <iostream>
#include <string>
#include <memory>

#include <propcalc/stream.hpp>
#include <propcalc/conjunctive.hpp>
#include <propcalc/variable.hpp>
#include <propcalc/formula.hpp>

namespace Propcalc {
	namespace DIMACS {
		class In : public Conjunctive {
			std::istream& in;
			Domain* domain;

		public:
			/* FIXME: default domain = new Cache doesn't seem exception-safe. */
			In(std::istream& in, Domain* domain = new Cache) :
					in(in), domain(domain)
			{
				++*this; /* fast-forward to first clause */
			}

			operator bool(void) const { return !in.eof(); }
			In& operator++(void);
		};

		/* FIXME: default domain = new Cache doesn't seem exception-safe. */
		Formula read(std::istream& in, Domain* domain = new Cache);

		class Out : public Stream<std::string> {
			Conjunctive& clauses;
			std::shared_ptr<Domain> domain;

		public:
			Out(Conjunctive& clauses, Domain* domain) :
					clauses(clauses), domain(domain)
			{ }

			operator bool(void)   const { return !!clauses;  }
			Out& operator++(void) { ++clauses; return *this; }
			std::string operator*(void);
		};

		struct Header {
			std::vector<std::string> comments;
			VarNr maxvar;
			size_t nclauses;
		};

		void write(std::ostream& out, Conjunctive& clauses, Domain* domain, std::vector<std::string> comments = {});
		void write(std::ostream& out, Conjunctive& clauses, Domain* domain, Header header);
	}
}

#endif /* PROPCALC_DIMACS_HPP */
