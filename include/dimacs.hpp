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
			std::shared_ptr<Domain> domain;
			Clause last;

		public:
			In(std::istream& in, std::shared_ptr<Domain> domain = std::make_shared<Cache>()) :
				in(in), domain(domain) {
				++*this; /* fast-forward to first clause */
			}

			Clause operator*(void) const { return last;      }
			operator bool(void) const    { return !in.eof(); }
			In& operator++(void);
		};

		Formula read(std::istream& in, std::shared_ptr<Domain> domain = std::make_shared<Cache>());

		class Out : public Stream<std::string> {
			Stream<Clause>& st;
			std::shared_ptr<Domain> domain;

		public:
			Out(Stream<Clause>& clauses, std::shared_ptr<Domain> domain) :
				st(clauses), domain(domain)
			{ }

			operator bool(void) const   { return !!st;  }
			Out& operator++(void) { ++st; return *this; }
			std::string operator*(void) const;
		};

		struct Header {
			std::vector<std::string> comments;
			VarNr maxvar;
			size_t nclauses;
		};

		void write(std::ostream& out, Stream<Clause>& clauses, std::shared_ptr<Domain> domain, std::vector<std::string> comments = {});
		void write(std::ostream& out, Stream<Clause>& clauses, std::shared_ptr<Domain> domain, Header header);
	}
}

#endif /* PROPCALC_DIMACS_HPP */
