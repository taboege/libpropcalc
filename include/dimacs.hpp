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
#include <propcalc/clause.hpp>
#include <propcalc/variable.hpp>
#include <propcalc/formula.hpp>

namespace Propcalc {
	namespace DIMACS {
		class In : public Stream<Clause> {
			std::istream& in;
			Domain* domain;

		public:
			In(std::istream& in, Domain* domain = new Cache) :
				in(in), domain(domain) {
				++*this; /* fast-forward to first clause */
			}

			operator bool(void) const { return !in.eof(); }
			In& operator++(void);
		};

		Formula read(std::istream& in, Domain* domain = new Cache);

		class Out : public Stream<std::string> {
			Stream<Clause>& st;
			std::shared_ptr<Domain> domain;

		public:
			Out(Stream<Clause>& clauses, Domain* domain) :
				st(clauses), domain(domain)
			{ }

			operator bool(void)   const { return !!st;  }
			Out& operator++(void) { ++st; return *this; }
			std::string operator*(void);
		};

		struct Header {
			std::vector<std::string> comments;
			VarNr maxvar;
			size_t nclauses;
		};

		void write(std::ostream& out, Stream<Clause>& clauses, Domain* domain, std::vector<std::string> comments = {});
		void write(std::ostream& out, Stream<Clause>& clauses, Domain* domain, Header header);
	}
}

#endif /* PROPCALC_DIMACS_HPP */
